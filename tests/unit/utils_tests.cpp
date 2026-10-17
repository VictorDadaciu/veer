#include <cppunit.h>

#include <vql/utils.h>

struct utils_tests : public Cppunit
{
    void test_next_multiple_of_cache_line_size()
    {
        CHECK(vql::next_multiple_of_cache_line_size(0), 0);
        CHECK(vql::next_multiple_of_cache_line_size(1), 64);
        CHECK(vql::next_multiple_of_cache_line_size(64), 64);
        CHECK(vql::next_multiple_of_cache_line_size(65), 128);
        CHECK(vql::next_multiple_of_cache_line_size(129), 192);
    }

    void test_next_power_of_2()
    {
        CHECK(vql::next_power_of_2(0), 0);
        CHECK(vql::next_power_of_2(1), 1);
        CHECK(vql::next_power_of_2(2), 2);
        CHECK(vql::next_power_of_2(3), 4);
        CHECK(vql::next_power_of_2(128), 128);
        CHECK(vql::next_power_of_2(129), 256);
    }

    void test_fuzzy_eq()
    {
        CHECKT(ve::fuzzy_eq(0.1f, 0.1));
        CHECKT(ve::fuzzy_eq(0.1, 0.1f));
        CHECKT(!ve::fuzzy_eq(30.5, 1.9f, ve::tolerance<1>));
        CHECKT(ve::fuzzy_eq(10.5, 1.9f, ve::tolerance<1>));
        CHECKT(ve::fuzzy_eq(0.1, 0.9f, ve::tolerance<0>));
        CHECKT(!ve::fuzzy_eq(0.1, 0.9f, ve::tolerance<-1>));
        CHECKT(ve::fuzzy_eq(0.001, 0.009f, ve::tolerance<-2>));
    }

    void test_list() override
    {
        test_next_multiple_of_cache_line_size();
        test_next_power_of_2();
        test_fuzzy_eq();
    }
};