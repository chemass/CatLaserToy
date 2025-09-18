#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// Simple test to verify C++11 compatibility
TEST_CASE("Basic doctest functionality") {
    CHECK_EQ(1, 1);
    CHECK_NE(1, 2);
    CHECK_LT(1, 2);
    CHECK_GT(2, 1);
    CHECK(true);
}