#include "catch.hpp"

#include <iostream>
#include "../lib/UnconstrainedVariableSimplifier.h"
#include <z3++.h>
#include <tuple>

using namespace z3;

bool CheckUnsatOrPrintModel(solver& s)
{
    if (s.check() == sat)
    {
        std::cout << s << std::endl;
        std::cout << s.get_model() << std::endl;
        return false;
    }

    return true;
}

TEST_CASE( "Unconstrained: unconstrained binary functions", "[verify-unconstrained-binary-functions]" )
{
    context c;

    solver s(c);
    expr t = c.bv_const("t", 4);
    expr u = c.bv_const("u", 4);
    expr res = c.bv_const("res", 4);
    expr v = c.bv_const("v", 4);

    auto functions = { Z3_mk_bvadd, Z3_mk_bvsub, Z3_mk_bvmul, Z3_mk_bvudiv, Z3_mk_bvurem,
                       Z3_mk_bvshl, Z3_mk_bvashr, Z3_mk_bvlshr,
                       Z3_mk_bvand, Z3_mk_bvor, Z3_mk_bvxor
    };

    SECTION( "f(t, u)")
    {
        for (auto& f : functions)
        {
            auto expr_f = [&] (z3::expr& x, z3::expr& y) { return to_expr(c, f(c, x, y)); };

            expr original = expr_f(t, u);

            expr original_v = expr_f(t, v);
            UnconstrainedVariableSimplifier simplifier(c, original_v);
            simplifier.SetMulReplacementMode(MASK);
            simplifier.MarkConstrained({"t"});
            simplifier.SimplifyIte();
            expr simplified = simplifier.GetExpr();

            INFO( " Checking " + original.to_string() + " subset");

            s.push();
            s.add(original == res && forall(v, simplified != res));
            REQUIRE(s.check() == unsat);
            s.pop();

            INFO( " Checking " + original.to_string() + " superset");

            s.push();
            s.add(simplified == res && forall(u, original != res));
            REQUIRE(s.check() == unsat);
            s.pop();
        }
    }

    SECTION( "f(u, t)")
    {
        for (auto& f : functions)
        {
            auto expr_f = [&] (z3::expr& x, z3::expr& y) { return to_expr(c, f(c, x, y)); };

            expr original = expr_f(u, t);

            expr original_v = expr_f(v, t);
            UnconstrainedVariableSimplifier simplifier(c, original_v);
            simplifier.SetMulReplacementMode(MASK);
            simplifier.MarkConstrained({"t"});
            simplifier.SimplifyIte();
            expr simplified = simplifier.GetExpr();

            INFO( " Checking " + original.to_string() + " subset");

            s.push();
            s.add(original == res && forall(v, simplified != res));
            REQUIRE(s.check() == unsat);
            s.pop();

            INFO( " Checking " + original.to_string() + " superset");

            s.push();
            s.add(simplified == res && forall(u, original != res));
            REQUIRE(s.check() == unsat);
            s.pop();
        }
    }
}

TEST_CASE( "Unconstrained: goal unconstrained", "[verify-goal-unconstrained]" )
{
    context c;

    solver s(c);
    expr t = c.bv_const("t", 4);
    expr u = c.bv_const("u", 4);
    expr res = c.bv_const("res", 4);
    expr v = c.bv_const("v", 4);

    auto functions = { Z3_mk_bvadd, Z3_mk_bvsub, Z3_mk_bvmul, Z3_mk_bvudiv, Z3_mk_bvurem,
                       Z3_mk_bvshl, Z3_mk_bvashr, Z3_mk_bvlshr,
                       Z3_mk_bvand, Z3_mk_bvor, Z3_mk_bvxor };

    auto signs = { std::make_tuple(SIGN_MIN, Z3_mk_bvsgt, "Signed min"),
                   { SIGN_MAX, Z3_mk_bvslt, "Signed max" },
                   { UNSIGN_MIN, Z3_mk_bvugt, "Unsigned min" },
                   { UNSIGN_MAX, Z3_mk_bvult, "Unsigned max" }
    };

    SECTION( "f(t, u)")
    {
        for (const auto& f : functions)
        {
            auto expr_f = [&] (z3::expr& x, z3::expr& y) { return to_expr(c, f(c, x, y)); };

            for (auto& [goal, pred, goalString] : signs)
            {
                auto pred_f = [&c, pred] (z3::expr& x, z3::expr& y) { return to_expr(c, pred(c, x, y)); };

                expr original = expr_f(t, u);

                expr original_v = expr_f(t, v);
                UnconstrainedVariableSimplifier simplifier(c, original_v);
                simplifier.SetMulReplacementMode(MASK);
                simplifier.MarkConstrained({"t"});
                simplifier.SetGoalUnconstrained(true);
                simplifier.ForceGoal(goal);
                simplifier.SimplifyIte();
                expr simplified = simplifier.GetExpr();

                INFO( " Checking " + original.to_string() + " " + goalString + " extremeness");

                s.push();
                s.add(original == res && forall(v, pred_f(simplified, res)));
                REQUIRE(CheckUnsatOrPrintModel(s));
                s.pop();

                INFO( " Checking " + original.to_string() + " " + goalString + " correctness");

                s.push();
                s.add(simplified == res && forall(u, original != res));
                REQUIRE(CheckUnsatOrPrintModel(s));
                s.pop();
            }
        }
    }

    SECTION( "f(u, t)")
    {
        for (const auto& f : functions)
        {
            auto expr_f = [&] (z3::expr& x, z3::expr& y) { return to_expr(c, f(c, x, y)); };

            for (auto& [goal, pred, goalString] : signs)
            {
                auto pred_f = [&c, pred] (z3::expr& x, z3::expr& y) { return to_expr(c, pred(c, x, y)); };

                expr original = expr_f(u, t);

                expr original_v = expr_f(v, t);
                UnconstrainedVariableSimplifier simplifier(c, original_v);
                simplifier.SetMulReplacementMode(MASK);
                simplifier.MarkConstrained({"t"});
                simplifier.SetGoalUnconstrained(true);
                simplifier.ForceGoal(goal);
                simplifier.SimplifyIte();
                expr simplified = simplifier.GetExpr();

                INFO( " Checking " + original.to_string() + " " + goalString + " extremeness");

                s.push();
                s.add(original == res && forall(v, pred_f(simplified, res)));
                REQUIRE(CheckUnsatOrPrintModel(s));
                s.pop();

                INFO( " Checking " + original.to_string() + " " + goalString + " correctness");

                s.push();
                s.add(simplified == res && forall(u, original != res));
                REQUIRE(CheckUnsatOrPrintModel(s));
                s.pop();
            }
        }
    }
}