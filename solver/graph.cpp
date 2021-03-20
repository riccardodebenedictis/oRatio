#include "graph.h"
#include "solver.h"
#include "flaw.h"
#include "resolver.h"
#include "combinations.h"
#include "atom_flaw.h"
#include <cassert>

using namespace smt;

namespace ratio
{
    graph::graph(solver &slv) : slv(slv), gamma(slv.get_sat_core().new_var()) { LOG("γ is " << to_string(gamma)); }
    graph::~graph() {}

    void graph::enqueue(flaw &f) { flaw_q.push_back(&f); }

    void graph::propagate_costs(flaw &f)
    {
        rational c_cost; // the current cost..
        if (slv.get_sat_core().value(f.get_phi()) == False)
            c_cost = rational::POSITIVE_INFINITY;
        else
        {
            resolver *chpst_res = f.get_cheapest_resolver(); // the flaw's cheapest resolver..
            c_cost = chpst_res ? chpst_res->get_estimated_cost() : rational::POSITIVE_INFINITY;
        }

        if (f.est_cost == c_cost)
            return; // nothing to propagate..
        else if (visited.count(&f))
        { // we are propagating costs within a causal cycle..
            c_cost = rational::POSITIVE_INFINITY;
            if (f.est_cost == c_cost)
                return; // nothing to propagate..
        }

        if (!slv.trail.empty()) // we store the current flaw's estimated cost, if not already stored, for allowing backtracking..
            slv.trail.back().old_f_costs.try_emplace(&f, f.est_cost);

        // we update the flaw's estimated cost..
        f.est_cost = c_cost;
        G_FIRE_FLAW_COST_CHANGED(f);

        // we (try to) update the estimated costs of the supports' effects and enqueue them for cost propagation..
        visited.insert(&f);
        for (const auto &supp : f.supports)
            if (slv.get_sat_core().value(supp->rho) != False)
                propagate_costs(supp->effect);
        visited.erase(&f);
    }

    void graph::build()
    {
        assert(slv.get_sat_core().root_level());
        LOG("building the causal graph..");

        while (std::any_of(slv.flaws.begin(), slv.flaws.end(), [](flaw *f) { return is_positive_infinite(f->get_estimated_cost()); }))
        {
            if (flaw_q.empty())
                throw unsolvable_exception();
#ifdef DEFERRABLE_FLAWS
            flaw *c_f = flaw_q.front();
            flaw_q.pop_front();
            assert(!c_f->expanded);
            if (slv.get_sat_core().value(c_f->phi) != False)
            {
                std::unordered_set<flaw *> visited;
                if (is_deferrable(*c_f)) // we have a deferrable flaw: we can postpone its expansion..
                    flaw_q.push_back(c_f);
                else
                    slv.expand_flaw(*c_f); // we expand the flaw..
                assert(visited.empty());
            }
#else
            size_t q_size = flaw_q.size();
            for (size_t i = 0; i < q_size; ++i)
            {
                flaw *c_f = flaw_q.front();
                flaw_q.pop_front();
                assert(!c_f->expanded);
                if (slv.get_sat_core().value(c_f->phi) != False)
                    slv.expand_flaw(*c_f); // we expand the flaw..
            }
#endif
        }

        // we extract the inconsistencies (and translate them into flaws)..
        std::vector<std::vector<std::pair<lit, double>>> incs = slv.get_incs();
        for (const auto &f : slv.pending_flaws)
            slv.new_flaw(*f, false); // we add the flaws, without enqueuing, to the planning graph..
        slv.pending_flaws.clear();

        // we perform some cleanings..
        slv.get_sat_core().simplify_db();
    }

    void graph::add_layer()
    {
        assert(slv.get_sat_core().root_level());
        assert(std::none_of(slv.flaws.begin(), slv.flaws.end(), [](flaw *f) { return is_positive_infinite(f->get_estimated_cost()); }));
        LOG("adding a layer to the causal graph..");

        std::deque<flaw *> f_q(flaw_q);
        while (std::all_of(f_q.begin(), f_q.end(), [](flaw *f) { return is_infinite(f->get_estimated_cost()); }))
        {
            if (flaw_q.empty())
                throw unsolvable_exception();
            size_t q_size = flaw_q.size();
            for (size_t i = 0; i < q_size; ++i)
            {
                flaw *c_f = flaw_q.front();
                flaw_q.pop_front();
                if (slv.get_sat_core().value(c_f->phi) != False)
                    slv.expand_flaw(*c_f); // we expand the flaw..
            }
        }

        // we extract the inconsistencies (and translate them into flaws)..
        std::vector<std::vector<std::pair<lit, double>>> incs = slv.get_incs();
        for (const auto &f : slv.pending_flaws)
            slv.new_flaw(*f, false); // we add the flaws, without enqueuing, to the planning graph..
        slv.pending_flaws.clear();

        // we perform some cleanings..
        slv.get_sat_core().simplify_db();
    }

#ifdef DEFERRABLE_FLAWS
    bool graph::is_deferrable(flaw &f)
    {
        if (f.get_estimated_cost() < rational::POSITIVE_INFINITY || std::any_of(f.resolvers.begin(), f.resolvers.end(), [this](resolver *r) { return slv.sat.value(r->rho) == True; }))
            return true; // we already have a possible solution for this flaw, thus we defer..
        if (slv.get_sat_core().value(f.get_phi()) == True || visited.count(&f))
            return false; // we necessarily have to solve this flaw: it cannot be deferred..
        visited.insert(&f);
        bool def = std::all_of(f.supports.begin(), f.supports.end(), [this](resolver *r) { return is_deferrable(r->get_effect()); });
        visited.erase(&f);
        return def;
    }
#endif

    void graph::check_gamma()
    {
        assert(slv.root_level());
        if (slv.get_sat_core().value(gamma) != False) // a unit clause, not gamma, has been deduced or, simply, gamma has never been set..
            build();                                  // the search procedure may have excluded those parts of the graph that could lead to a solution..
        else
        { // the graph has been invalidated..
            LOG("search has exhausted the graph..");
            // we create a new graph var..
            gamma = lit(slv.get_sat_core().new_var());
            LOG("γ is" << to_string(gamma));
            already_closed.clear();
            add_layer(); // we add a layer to the current graph..
        }

        LOG("pruning the graph..");
        // these flaws have not been expanded, hence, cannot have a solution..
        for (const auto &f : flaw_q)
            if (already_closed.insert(variable(f->phi)).second)
                if (!slv.get_sat_core().new_clause({!gamma, !f->phi}))
                    throw unsolvable_exception();

        slv.take_decision(gamma);
    }
} // namespace ratio