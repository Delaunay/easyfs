#ifndef PROJECT_TEST_TESTS_MULT_HEADER
#define PROJECT_TEST_TESTS_MULT_HEADER

#include <gtest/gtest.h>

#include <symbolic.h>


using namespace easyfs;

TEST(Mult, eval)
{
    auto f = mult(make_val(21), make_val(32));
    Context ctx;

    EXPECT_DOUBLE_EQ(21 * 32, f->full_eval(ctx));
}

// This test will fail because mult->derivate implementation is wrong
TEST(Mult, derivate)
{
    auto x = make_var("x");
    auto f = mult(x, make_val(32));

    Context ctx;
    ctx["x"] = make_val(2);

    auto deriv = f->derivate("x");

    EXPECT_DOUBLE_EQ(32, deriv->full_eval(ctx));
}


#endif

