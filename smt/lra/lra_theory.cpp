#include "lra_theory.h"
#include "lra_constraint.h"
#include "lra_value_listener.h"
#include <algorithm>
#include <cassert>

namespace smt
{

    lra_theory::lra_theory(sat_core &sat) : theory(sat) {}
    lra_theory::~lra_theory() {}

    var lra_theory::new_var() noexcept
    {
        // we create a new arithmetic variable..
        const var id = vals.size();
        c_bounds.push_back({rational::NEGATIVE_INFINITY, TRUE_var}); // we set the lower bound at -inf..
        c_bounds.push_back({rational::POSITIVE_INFINITY, TRUE_var}); // we set the upper bound at +inf..
        vals.push_back(rational::ZERO);                              // we set the current value at 0..
        exprs.emplace("x" + std::to_string(id), id);
        a_watches.resize(vals.size());
        t_watches.resize(vals.size());
        return id;
    }

    var lra_theory::new_var(const lin &l) noexcept
    { // we create, if needed, a new arithmetic variable which is equal to the given linear expression..
        const std::string s_expr = to_string(l);
        if (const auto at_expr = exprs.find(s_expr); at_expr != exprs.end()) // the expression already exists..
            return at_expr->second;
        else
        { // we need to create a new slack variable..
            assert(sat.root_level());
            const var slack = new_var();
            exprs.emplace(s_expr, slack);
            c_bounds[lb_index(slack)] = {lb(l), TRUE_var}; // we set the lower bound at the lower bound of the given linear expression..
            c_bounds[ub_index(slack)] = {ub(l), TRUE_var}; // we set the upper bound at the upper bound of the given linear expression..
            vals[slack] = value(l);                        // we set the initial value of the new slack variable at the value of the given linear expression..
            new_row(slack, l);                             // we add a new row into the tableau..
            return slack;
        }
    }

    lit lra_theory::new_lt(const lin &left, const lin &right) noexcept
    {
        lin expr = left - right;
        std::vector<var> vars;
        vars.reserve(expr.vars.size());
        for (const auto &term : expr.vars)
            vars.push_back(term.first);
        for (const auto &v : vars)
            if (const auto at_v = tableau.find(v); at_v != tableau.end())
            {
                rational c = expr.vars[v];
                expr.vars.erase(v);
                expr += at_v->second->l * c;
            }

        const inf_rational c_right = inf_rational(-expr.known_term, -1);
        expr.known_term = 0;

        if (ub(expr) <= c_right) // the constraint is already satisfied..
            return TRUE_var;
        else if (lb(expr) > c_right) // the constraint is unsatisfable..
            return FALSE_var;

        const var slack = new_var(expr);
        const std::string s_assertion = "x" + std::to_string(slack) + " <= " + to_string(c_right);
        if (const auto at_asrt = s_asrts.find(s_assertion); at_asrt != s_asrts.end()) // this assertion already exists..
            return at_asrt->second;
        else
        { // we need to create a new control variable..
            const var ctr = sat.new_var();
            bind(ctr);
            s_asrts.emplace(s_assertion, ctr);
            v_asrts[ctr].push_back(new assertion(*this, op::leq, ctr, slack, c_right));
            return ctr;
        }
    }

    lit lra_theory::new_leq(const lin &left, const lin &right) noexcept
    {
        lin expr = left - right;
        std::vector<var> vars;
        vars.reserve(expr.vars.size());
        for (const auto &term : expr.vars)
            vars.push_back(term.first);
        for (const auto &v : vars)
            if (const auto at_v = tableau.find(v); at_v != tableau.end())
            {
                rational c = expr.vars[v];
                expr.vars.erase(v);
                expr += at_v->second->l * c;
            }

        const inf_rational c_right = -expr.known_term;
        expr.known_term = 0;

        if (ub(expr) <= c_right) // the constraint is already satisfied..
            return TRUE_var;
        else if (lb(expr) > c_right) // the constraint is unsatisfable..
            return FALSE_var;

        const var slack = new_var(expr);
        const std::string s_assertion = "x" + std::to_string(slack) + " <= " + to_string(c_right);
        if (const auto at_asrt = s_asrts.find(s_assertion); at_asrt != s_asrts.end()) // this assertion already exists..
            return at_asrt->second;
        else
        { // we need to create a new control variable..
            const var ctr = sat.new_var();
            bind(ctr);
            s_asrts.emplace(s_assertion, ctr);
            v_asrts[ctr].push_back(new assertion(*this, op::leq, ctr, slack, c_right));
            return ctr;
        }
    }

    lit lra_theory::new_geq(const lin &left, const lin &right) noexcept
    {
        lin expr = left - right;
        std::vector<var> vars;
        vars.reserve(expr.vars.size());
        for (const auto &term : expr.vars)
            vars.push_back(term.first);
        for (const auto &v : vars)
            if (const auto at_v = tableau.find(v); at_v != tableau.end())
            {
                rational c = expr.vars[v];
                expr.vars.erase(v);
                expr += at_v->second->l * c;
            }

        const inf_rational c_right = -expr.known_term;
        expr.known_term = 0;

        if (lb(expr) >= c_right) // the constraint is already satisfied..
            return TRUE_var;
        else if (ub(expr) < c_right) // the constraint is unsatisfable..
            return FALSE_var;

        const var slack = new_var(expr);
        const std::string s_assertion = "x" + std::to_string(slack) + " >= " + to_string(c_right);
        if (const auto at_asrt = s_asrts.find(s_assertion); at_asrt != s_asrts.end()) // this assertion already exists..
            return at_asrt->second;
        else
        { // we need to create a new control variable..
            const var ctr = sat.new_var();
            bind(ctr);
            s_asrts.emplace(s_assertion, ctr);
            v_asrts[ctr].push_back(new assertion(*this, op::geq, ctr, slack, c_right));
            return ctr;
        }
    }

    lit lra_theory::new_gt(const lin &left, const lin &right) noexcept
    {
        lin expr = left - right;
        std::vector<var> vars;
        vars.reserve(expr.vars.size());
        for (const auto &term : expr.vars)
            vars.push_back(term.first);
        for (const auto &v : vars)
            if (const auto at_v = tableau.find(v); at_v != tableau.end())
            {
                rational c = expr.vars[v];
                expr.vars.erase(v);
                expr += at_v->second->l * c;
            }

        const inf_rational c_right = inf_rational(-expr.known_term, 1);
        expr.known_term = 0;

        if (lb(expr) >= c_right) // the constraint is already satisfied..
            return TRUE_var;
        else if (ub(expr) < c_right) // the constraint is unsatisfable..
            return FALSE_var;

        const var slack = new_var(expr);
        const std::string s_assertion = "x" + std::to_string(slack) + " >= " + to_string(c_right);
        if (const auto at_asrt = s_asrts.find(s_assertion); at_asrt != s_asrts.end()) // this assertion already exists..
            return at_asrt->second;
        else
        { // we need to create a new control variable..
            const var ctr = sat.new_var();
            bind(ctr);
            s_asrts.emplace(s_assertion, ctr);
            v_asrts[ctr].push_back(new assertion(*this, op::geq, ctr, slack, c_right));
            return ctr;
        }
    }

    bool lra_theory::equates(const lin &l0, const lin &l1) const noexcept
    {
        const auto l0_bounds = bounds(l0);
        const auto l1_bounds = bounds(l1);
        return l0_bounds.second >= l1_bounds.first && l0_bounds.first <= l1_bounds.second; // the two intervals intersect..
    }

    bool lra_theory::propagate(const lit &p) noexcept
    {
        assert(cnfl.empty());
        for (const auto &a : v_asrts[variable(p)])
            switch (sat.value(a->b))
            {
            case True: // the assertion is direct..
                if (!((a->o == op::leq) ? assert_upper(a->x, a->v, p) : assert_lower(a->x, a->v, p)))
                    return false;
                break;
            case False: // the assertion is negated..
                if (!((a->o == op::leq) ? assert_lower(a->x, a->v + inf_rational(rational::ZERO, rational::ONE), p) : assert_upper(a->x, a->v - inf_rational(rational::ZERO, rational::ONE), p)))
                    return false;
                break;
            }

        return true;
    }

    bool lra_theory::check() noexcept
    {
        assert(cnfl.empty());
        while (true)
        {
            const auto &x_i_it = std::find_if(tableau.begin(), tableau.end(), [this](const std::pair<var, row *> &v) { return value(v.first) < lb(v.first) || value(v.first) > ub(v.first); });
            if (x_i_it == tableau.end())
                return true;
            // the current value of the x_i variable is out of its c_bounds..
            const var x_i = (*x_i_it).first;
            // the flawed row..
            const row *f_row = (*x_i_it).second;
            if (value(x_i) < lb(x_i))
            {
                const auto &x_j_it = std::find_if(f_row->l.vars.begin(), f_row->l.vars.end(), [f_row, this](const std::pair<var, inf_rational> &v) { return (is_positive(f_row->l.vars.at(v.first)) && value(v.first) < ub(v.first)) || (is_negative(f_row->l.vars.at(v.first)) && value(v.first) > lb(v.first)); });
                if (x_j_it != f_row->l.vars.end()) // var x_j can be used to increase the value of x_i..
                    pivot_and_update(x_i, (*x_j_it).first, lb(x_i));
                else
                { // we generate an explanation for the conflict..
                    for (const auto &term : f_row->l.vars)
                        if (is_positive(term.second))
                            cnfl.push_back(!c_bounds[lra_theory::ub_index(term.first)].reason);
                        else if (is_negative(term.second))
                            cnfl.push_back(!c_bounds[lra_theory::lb_index(term.first)].reason);
                    cnfl.push_back(!c_bounds[lra_theory::lb_index(x_i)].reason);
                    return false;
                }
            }
            else if (value(x_i) > ub(x_i))
            {
                const auto &x_j_it = std::find_if(f_row->l.vars.begin(), f_row->l.vars.end(), [f_row, this](const std::pair<var, inf_rational> &v) { return (is_negative(f_row->l.vars.at(v.first)) && value(v.first) < ub(v.first)) || (is_positive(f_row->l.vars.at(v.first)) && value(v.first) > lb(v.first)); });
                if (x_j_it != f_row->l.vars.end()) // var x_j can be used to decrease the value of x_i..
                    pivot_and_update(x_i, (*x_j_it).first, ub(x_i));
                else
                { // we generate an explanation for the conflict..
                    for (const auto &term : f_row->l.vars)
                        if (is_positive(term.second))
                            cnfl.push_back(!c_bounds[lra_theory::lb_index(term.first)].reason);
                        else if (is_negative(term.second))
                            cnfl.push_back(!c_bounds[lra_theory::ub_index(term.first)].reason);
                    cnfl.push_back(!c_bounds[lra_theory::ub_index(x_i)].reason);
                    return false;
                }
            }
        }
    }

    void lra_theory::push() noexcept { layers.push_back(std::unordered_map<size_t, bound>()); }

    void lra_theory::pop() noexcept
    {
        // we restore the variables' c_bounds and their reason..
        for (const auto &b : layers.back())
            c_bounds[b.first] = b.second;
        layers.pop_back();
    }

    bool lra_theory::assert_lower(const var &x_i, const inf_rational &val, const lit &p) noexcept
    {
        assert(cnfl.empty());
        if (val <= lb(x_i))
            return true;
        else if (val > ub(x_i))
        {
            cnfl.push_back(!p);                              // either the literal 'p' is false ..
            cnfl.push_back(!c_bounds[ub_index(x_i)].reason); // or what asserted the upper bound is false..
            return false;
        }
        else
        {
            if (!layers.empty() && !layers.back().count(lb_index(x_i)))
                layers.back().insert({lb_index(x_i), {lb(x_i), c_bounds[lb_index(x_i)].reason}});
            c_bounds[lb_index(x_i)] = {val, p};

            if (vals[x_i] < val && !is_basic(x_i))
                update(x_i, val);

            // unate propagation..
            for (const auto &c : a_watches[x_i])
                if (!c->propagate_lb(x_i))
                    return false;
            // bound propagation..
            for (const auto &c : t_watches[x_i])
                if (!c->propagate_lb(x_i))
                    return false;

            return true;
        }
    }

    bool lra_theory::assert_upper(const var &x_i, const inf_rational &val, const lit &p) noexcept
    {
        assert(cnfl.empty());
        if (val >= ub(x_i))
            return true;
        else if (val < lb(x_i))
        {
            cnfl.push_back(!p);                              // either the literal 'p' is false ..
            cnfl.push_back(!c_bounds[lb_index(x_i)].reason); // or what asserted the lower bound is false..
            return false;
        }
        else
        {
            if (!layers.empty() && !layers.back().count(ub_index(x_i)))
                layers.back().insert({ub_index(x_i), {ub(x_i), c_bounds[ub_index(x_i)].reason}});
            c_bounds[ub_index(x_i)] = {val, p};

            if (vals[x_i] > val && !is_basic(x_i))
                update(x_i, val);

            // unate propagation..
            for (const auto &c : a_watches[x_i])
                if (!c->propagate_ub(x_i))
                    return false;
            // bound propagation..
            for (const auto &c : t_watches[x_i])
                if (!c->propagate_ub(x_i))
                    return false;

            return true;
        }
    }

    void lra_theory::update(const var &x_i, const inf_rational &v) noexcept
    {
        assert(!is_basic(x_i) && "x_i should be a non-basic variable..");
        for (const auto &c : t_watches[x_i])
        { // x_j = x_j + a_ji(v - x_i)..
            vals[c->x] += c->l.vars.at(x_i) * (v - vals[x_i]);
            if (const auto at_c_x = listening.find(c->x); at_c_x != listening.end())
                for (const auto &l : at_c_x->second)
                    l->lra_value_change(c->x);
        }
        // x_i = v..
        vals[x_i] = v;
        if (const auto at_x_i = listening.find(x_i); at_x_i != listening.end())
            for (const auto &l : at_x_i->second)
                l->lra_value_change(x_i);
    }

    void lra_theory::pivot_and_update(const var &x_i, const var &x_j, const inf_rational &v) noexcept
    {
        assert(is_basic(x_i) && "x_i should be a basic variable..");
        assert(!is_basic(x_j) && "x_j should be a non-basic variable..");
        assert(tableau.at(x_i)->l.vars.count(x_j));

        const inf_rational theta = (v - vals[x_i]) / tableau.at(x_i)->l.vars.at(x_j);
        assert(!is_infinite(theta));

        // x_i = v
        vals[x_i] = v;
        if (const auto at_x_i = listening.find(x_i); at_x_i != listening.end())
            for (const auto &l : at_x_i->second)
                l->lra_value_change(x_i);

        // x_j += theta
        vals[x_j] += theta;
        if (const auto at_x_j = listening.find(x_j); at_x_j != listening.end())
            for (const auto &l : at_x_j->second)
                l->lra_value_change(x_j);

        for (const auto &c : t_watches[x_j])
            if (c->x != x_i)
            { // x_k += a_kj * theta..
                vals[c->x] += c->l.vars.at(x_j) * theta;
                if (const auto at_x_c = listening.find(c->x); at_x_c != listening.end())
                    for (const auto &l : at_x_c->second)
                        l->lra_value_change(c->x);
            }

        pivot(x_i, x_j);
    }

    void lra_theory::pivot(const var x_i, const var x_j) noexcept
    {
        // the exiting row..
        row *ex_row = tableau[x_i];
        lin expr = std::move(ex_row->l);
        tableau.erase(x_i);
        // we remove the row from the watches..
        for (const auto &c : expr.vars)
            t_watches[c.first].erase(ex_row);
        delete ex_row;

        const rational c = expr.vars[x_j];
        expr.vars.erase(x_j);
        expr /= -c;
        expr.vars.emplace(x_i, rational::ONE / c);

        // these are the rows in which x_j appears..
        std::unordered_set<row *> x_j_watches;
        std::swap(x_j_watches, t_watches[x_j]);
        for (const auto &r : x_j_watches)
        { // 'r' is a row in which 'x_j' appears..
            rational cc = r->l.vars[x_j];
            r->l.vars.erase(x_j);
            for (const auto &term : std::map<const var, rational>(expr.vars))
                if (const auto trm_it = r->l.vars.find(term.first); trm_it == r->l.vars.end())
                { // we are adding a new term to 'r'..
                    r->l.vars.emplace(term.first, term.second * cc);
                    t_watches[term.first].emplace(r);
                }
                else
                { // we are updating an existing term of 'r'..
                    assert(trm_it->first == term.first);
                    trm_it->second += term.second * cc;
                    if (trm_it->second == rational::ZERO)
                    { // the updated term's coefficient has become equal to zero, hence we can remove the term..
                        r->l.vars.erase(trm_it);
                        t_watches[term.first].erase(r);
                    }
                }
            r->l.known_term += expr.known_term * cc;
        }

        // we add a new row into the tableau..
        new_row(x_j, expr);
    }

    void lra_theory::new_row(const var &x, const lin &l) noexcept
    {
        row *r = new row(*this, x, l);
        tableau.emplace(x, r);
        for (const auto &term : l.vars)
            t_watches[term.first].emplace(r);
    }

    std::string to_string(const lra_theory &rhs) noexcept
    {
        std::string la;
        la += "{ \"vars\" : [";
        for (size_t i = 0; i < rhs.vals.size(); ++i)
        {
            if (i)
                la += ", ";
            la += "{ \"name\" : \"x" + std::to_string(i) + "\", \"value\" : " + to_string(rhs.value(i));
            if (!is_negative_infinite(rhs.lb(i)))
                la += ", \"lb\" : " + to_string(rhs.lb(i));
            if (!is_positive_infinite(rhs.ub(i)))
                la += ", \"ub\" : " + to_string(rhs.ub(i));
            la += "}";
        }
        la += "], \"asrts\" : [";
        for (auto it = rhs.v_asrts.cbegin(); it != rhs.v_asrts.cend(); ++it)
        {
            if (it != rhs.v_asrts.cbegin())
                la += ", ";
            la += "[";
            for (auto a_it = it->second.cbegin(); a_it != it->second.cend(); ++a_it)
            {
                if (a_it != it->second.cbegin())
                    la += ", ";
                la += to_string(**a_it);
            }
            la += "]";
        }
        la += "], \"tableau\" : [";
        for (auto it = rhs.tableau.cbegin(); it != rhs.tableau.cend(); ++it)
        {
            if (it != rhs.tableau.cbegin())
                la += ", ";
            la += to_string(*it->second);
        }
        la += "]";
        la += "}";
        return la;
    }
} // namespace smt