#pragma once

#include "constr.h"

namespace smt
{

  class sat_core;

  /**
   * This class is used for representing propositional clauses.
   */
  class clause : public constr
  {
    friend class sat_core;

  private:
    clause(sat_core &s, const std::vector<lit> &lits);
    clause(const clause &orig) = delete;
    ~clause() override;

  public:
    const std::vector<lit> get_lits() const { return lits; }

  private:
    const bool propagate(const lit &p) override;
    const bool simplify() override;
    void remove() override;
    void get_reason(const lit &p, std::vector<lit> &out_reason) const override;

    friend std::string to_string(const clause &c);

  private:
    std::vector<lit> lits;
  };
} // namespace smt