#pragma once

#include "inf_rational.h"
#include "lit.h"
#include <vector>
#include <mutex>
#include <map>
#include <set>

namespace ratio
{

  class solver;
  class predicate;
  class atom;

  class executor
  {

  public:
    executor(solver &slv, const size_t &tick_duration, const smt::rational &units_for_milliseconds = smt::rational(1, 1000));
    executor(const executor &orig) = delete;
    ~executor();

    size_t get_current_time() const { return current_time; };

    void start();
    void stop();
    void set_timeout(const size_t &delay);

    void reset_timelines();

  private:
    virtual void tick() {}
    virtual void starting(const std::set<atom *> &atoms) {}
    virtual void ending(const std::set<atom *> &atoms) {}

    void dont_start(const std::set<atom *> &atoms) { not_starting.insert(atoms.begin(), atoms.end()); }
    void dont_end(const std::set<atom *> &atoms) { not_starting.insert(atoms.begin(), atoms.end()); }

  private:
    solver &slv;
    predicate &int_pred;
    predicate &imp_pred;
    const size_t tick_duration;                 // the duration of the ticks in milliseconds..
    const smt::rational units_for_milliseconds; // the number of plan units for a millisecond (e.g., 1/1000 means one unit for second)..
    std::mutex mtx;                             // a mutex for the critical sections..
    bool executing = false;
    size_t current_time;
    std::vector<smt::lit> pending_facts;
    std::set<atom *> not_starting, not_ending;
    std::map<smt::inf_rational, std::set<atom *>> s_atms, e_atms; // for each pulse (in milliseconds), the atoms starting/ending at that pulse..
    std::set<smt::inf_rational> pulses;                           // all the pulses (in milliseconds) of the plan..
  };
} // namespace ratio