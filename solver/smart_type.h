#pragma once

#include "type.h"
#include "sat_value_listener.h"
#include "lra_value_listener.h"
#include "ov_value_listener.h"

namespace ratio
{

class solver;
class atom;
class flaw;
class atom_flaw;
class resolver;

class smart_type : public type
{
  friend class solver;

public:
  smart_type(solver &slv, scope &scp, const std::string &name);
  smart_type(const smart_type &that) = delete;
  virtual ~smart_type();

  atom_flaw &get_reason(const atom &atm) const { return *reason.at(&atm); } // returns the flaw which has given rise to the atom..

private:
  virtual std::vector<flaw *> get_flaws() = 0;
  virtual void new_fact(atom_flaw &);
  virtual void new_goal(atom_flaw &);

protected:
  std::vector<resolver *> get_resolvers(const std::set<atom *> &atms); // returns the vector of resolvers which has given rise to the given atoms..

protected:
  solver &slv;

private:
  std::unordered_map<const atom *, atom_flaw *> reason; // the reason for having introduced an atom..
};

class atom_listener : public smt::sat_value_listener, public smt::lra_value_listener, public smt::ov_value_listener
{
public:
  atom_listener(atom &atm);
  atom_listener(const atom_listener &that) = delete;
  virtual ~atom_listener();

protected:
  atom &atm;
};
} // namespace ratio