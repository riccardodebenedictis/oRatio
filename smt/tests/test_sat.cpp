#include "sat_core.h"
#include <cassert>

using namespace smt;

void test_basic_core_0()
{
    sat_core core;
    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();

    bool nc = core.new_clause({lit(b0, false), !lit(b1), b2});
    assert(nc);
    bool ch = core.check();
    assert(ch);
    assert(core.value(b0) == Undefined);
    assert(core.value(b1) == Undefined);
    assert(core.value(b2) == Undefined);

    bool assm = core.assume(b0) && core.check();
    assert(assm);
    assert(core.value(b0) == True);
    assert(core.value(b1) == Undefined);
    assert(core.value(b2) == Undefined);

    assm = core.assume(b1) && core.check();
    assert(assm);
    assert(core.value(b0) == True);
    assert(core.value(b1) == True);
    assert(core.value(b2) == True);
}

void test_basic_core_1()
{
    sat_core core;
    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();

    bool ch = core.eq(b0, !lit(b1)) && core.check();
    assert(ch);
    assert(core.value(b0) == Undefined);
    assert(core.value(b1) == Undefined);
    assert(core.value(b2) == Undefined);

    ch = core.disj({b1, b2}, b0) && core.check();
    assert(ch);

    ch = core.assume(b0) && core.check();
    assert(ch);
    assert(core.value(b0) == True);
    assert(core.value(b1) == False);
    assert(core.value(b2) == True);
}

void test_basic_core_2()
{
    sat_core core;
    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();

    // b0 -> (b1 AND b2)
    bool ch = core.conj({b1, b2}, b0) && core.check();
    assert(ch);

    // (!b1, !b2)
    bool nc = core.new_clause({!lit(b1), !lit(b2)});
    assert(nc);
    ch = core.check();
    assert(ch);

    // propagation, in this case, does not work!
    assert(core.value(b0) == Undefined);

    // yet it works through learning..
    bool assm = core.assume(b0) && core.check();
    assert(assm);
    assert(core.value(b0) == False);
}

void test_no_good()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
    var b3 = core.new_var();
    var b4 = core.new_var();
    var b5 = core.new_var();
    var b6 = core.new_var();
    var b7 = core.new_var();
    var b8 = core.new_var();

    bool nc = core.new_clause({b0, b1});
    assert(nc);
    nc = core.new_clause({b0, b2, b6});
    assert(nc);
    nc = core.new_clause({lit(b1, false), lit(b2, false), b3});
    assert(nc);
    nc = core.new_clause({lit(b3, false), b4, b7});
    assert(nc);
    nc = core.new_clause({lit(b3, false), b5, b8});
    assert(nc);
    nc = core.new_clause({lit(b4, false), lit(b5, false)});
    assert(nc);

    bool assm = core.assume(lit(b6, false)) && core.check();
    assert(assm);
    assm = core.assume(lit(b7, false)) && core.check();
    assert(assm);
    assm = core.assume(lit(b8, false)) && core.check();
    assert(assm);
    assm = core.assume(lit(b0, false)) && core.check();
    assert(assm);
}

void test_assumptions()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
    var b3 = core.new_var();
    var b4 = core.new_var();
    var b5 = core.new_var();
    var b6 = core.new_var();
    var b7 = core.new_var();
    var b8 = core.new_var();

    bool nc = core.new_clause({b0, b1});
    assert(nc);
    nc = core.new_clause({b0, b2, b6});
    assert(nc);
    nc = core.new_clause({lit(b1, false), lit(b2, false), b3});
    assert(nc);
    nc = core.new_clause({lit(b3, false), b4, b7});
    assert(nc);
    nc = core.new_clause({lit(b3, false), b5, b8});
    assert(nc);
    nc = core.new_clause({lit(b4, false), lit(b5, false)});
    assert(nc);

    bool assm = core.check({lit(b6, false), lit(b7, false), lit(b8, false), lit(b0, false)});
    assert(!assm);
}

void test_amo()
{
    sat_core core;

    var b0 = core.new_var();
    var b1 = core.new_var();
    var b2 = core.new_var();
    var b3 = core.new_var();
    var b4 = core.new_var();
    var b5 = core.new_var();

    bool nc = core.at_most_one({b0, b1, b2, b3, b4, b5});
    assert(nc);

    bool assm = core.assume(lit(b0, false)) && core.check();
    assert(assm);
    assm = core.assume(b1) && core.check();
    assert(assm);
    assm = core.check({lit(b2, false), lit(b3, false), lit(b4, false), lit(b5, false)});
    assert(assm);
}

int main(int, char **)
{
    test_basic_core_0();
    test_basic_core_1();
    test_basic_core_2();

    test_no_good();

    test_assumptions();

    test_amo();
}