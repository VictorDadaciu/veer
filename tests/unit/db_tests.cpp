#include <cppunit.h>

#include <veer_core/db.h>
#include <veer_core/utils.h>

#include <cstdlib>
#include <type_traits>

DERIVE_PROP(bool_p, PROPERTY_ROOT(bool));
DERIVE_PROP(int_p, PROPERTY_ROOT(int));
DERIVE_PROP(float_p, PROPERTY_ROOT(float));

using table1 = TABLE(
    COLUMN_IMPL(bool_p),
    COLUMN_IMPL(int_p),
    COLUMN_IMPL(float_p)
);

struct db_tests : public Cppunit
{
    void test_row()
    {
        table1::row row{};
        CHECKT(!row.cell<bool_p>());
        row.cell<bool_p>() = true;
        CHECKT(row.cell<bool_p>());
        
        CHECK(row.cell<int_p>(), 0);
        row.cell<int_p>() = 25;
        CHECK(row.cell<int_p>(), 25);
        row.set_cell<int_p>(-25);
        CHECK(row.cell<int_p>(), -25);

        CHECKT(ve::fuzzy_eq(row.cell<float_p>(), 0.f));
        row.cell<float_p>() = 25.f;
        CHECKT(ve::fuzzy_eq(row.cell<float_p>(), 25.f));
        row.set_cell<float_p>(-25.f);
        CHECKT(ve::fuzzy_eq(row.cell<float_p>(), -25.f));
    }

    void test_rows_constructor()
    {
        table1::rows rows1{};
        CHECK(rows1.count(), 0zu);
        CHECK(rows1.capacity(), 1zu);
        table1::rows rows2 = std::move(rows1);
        CHECK(rows2.count(), 0zu);
        CHECK(rows2.capacity(), 1zu);
        table1::rows rows3(10);
        CHECK(rows3.count(), 0zu);
        CHECK(rows3.capacity(), 10zu);
    }

    void test_rows_push_back()
    {
        table1::default_row = table1::row();
        table1::rows rows{};
        for (size_t i = 0; i < 5; ++i)
        {
            CHECK(rows.push_back(), i);
            size_t count = i + 1;
            CHECK(rows.count(), count);
            CHECK(rows.capacity(), ve::next_power_of_2(count));
            CHECKT(!rows.cell<bool_p>(i));
            CHECK(rows.cell<int_p>(i), 0);
            CHECKT(ve::fuzzy_eq(rows.cell<float_p>(i), 0.f));
            {
                table1::row row = rows[i];
                CHECKT(!row.cell<bool_p>());
                CHECK(row.cell<int_p>(), 0);
                CHECKT(ve::fuzzy_eq(row.cell<float_p>(), 0.f));
            }
        }
        {
            table1::row row{};
            row.set_cell<bool_p>(true);
            row.set_cell<int_p>(10);
            row.set_cell<float_p>(100.f);
            table1::default_row = row;
        }
        for (size_t i = 5; i < 10; ++i)
        {
            CHECK(rows.push_back(), i);
            size_t count = i + 1;
            CHECK(rows.count(), count);
            CHECK(rows.capacity(), ve::next_power_of_2(count));
            CHECKT(rows.cell<bool_p>(i));
            CHECK(rows.cell<int_p>(i), 10);
            CHECKT(ve::fuzzy_eq(rows.cell<float_p>(i), 100.f));
            {
                table1::row row = rows[i];
                CHECKT(row.cell<bool_p>());
                CHECK(row.cell<int_p>(), 10);
                CHECKT(ve::fuzzy_eq(row.cell<float_p>(), 100.f));
            }
        }
        for (size_t i = 10; i < 15; ++i)
        {
            {
                table1::row row{};
                row.set_cell<bool_p>(i % 2);
                row.set_cell<int_p>(i);
                row.set_cell<float_p>(i);
                CHECK(rows.push_back(row), i);
            }
            size_t count = i + 1;
            CHECK(rows.count(), count);
            CHECK(rows.capacity(), ve::next_power_of_2(count));
            CHECK(rows.cell<bool_p>(i), i % 2);
            CHECK(rows.cell<int_p>(i), i);
            CHECKT(ve::fuzzy_eq(rows.cell<float_p>(i), static_cast<float>(i)));
            {    
                table1::row row = rows[i];
                CHECK(row.cell<bool_p>(), i % 2);
                CHECK(row.cell<int_p>(), i);
                CHECKT(ve::fuzzy_eq(row.cell<float_p>(), static_cast<float>(i)));
            }
        }
    }

    void test_rows_append()
    {
        table1::default_row = table1::row();
        table1::rows rows{};
        for (size_t i = 0; i < 10; ++i)
        {
            CHECK(rows.push_back(), i);
            CHECKT(!rows.cell<bool_p>(i));
        }
        CHECK(rows.count(), 10);
        CHECK(rows.capacity(), 16);
        {
            {
                table1::row row{};
                row.set_cell<bool_p>(true);
                table1::default_row = row;
            }
            table1::rows new_rows(3);
            for (size_t i = 0; i < new_rows.capacity(); ++i)
            {
                CHECK(new_rows.push_back(), i);
                CHECKT(new_rows.cell<bool_p>(i));
            }
            CHECK(rows.append(new_rows), 10);
        }
        CHECK(rows.count(), 13);
        CHECK(rows.capacity(), 16);
        {
            {
                table1::row row{};
                row.set_cell<bool_p>(true);
                table1::default_row = row;
            }
            table1::rows new_rows(5);
            for (size_t i = 0; i < new_rows.capacity(); ++i)
            {
                CHECK(new_rows.push_back(), i);
                CHECKT(new_rows.cell<bool_p>(i));
            }
            CHECK(rows.append(new_rows), 13);
        }
        CHECK(rows.count(), 18);
        CHECK(rows.capacity(), 32);
    }

    void test_rows_iterate()
    {
        table1::default_row = table1::row();
        table1::rows rows(128);
        for (size_t i = 0; i < rows.capacity(); ++i)
            rows.push_back();
        size_t how_many_expected{};
        rows.iterate<SELECT(bool_p)>(
            [&how_many_expected](auto e, auto& b)
            {
                size_t index = e;
                b = (std::rand() / (index + 1)) % 2;
                how_many_expected += b;
            }
        );
        CHECKT(how_many_expected > 0);
        size_t how_many{};
        size_t isum_expected{};
        rows.iterate<SELECT(bool_p, int_p)>(
            [&how_many, &isum_expected](auto e, auto& b, auto& i)
            {
                size_t index = e;
                how_many += b;
                i = index + b * (std::rand() % 100);
                isum_expected += i;
            }
        );
        CHECKT(isum_expected > 0);
        CHECK(how_many, how_many_expected);
        size_t isum{};
        double dsum_expected{};
        rows.iterate<SELECT(int_p, float_p)>(
            [&isum, &dsum_expected](auto e, auto& i, auto& f)
            {
                size_t index = e;
                isum += i;
                f = i / 3.14159 + index;
                dsum_expected += f;
            }
        );
        CHECKT(dsum_expected > 0.0);
        CHECK(isum, isum_expected);
        double dsum{};
        rows.iterate<SELECT(float_p)>(
            [&dsum](auto, auto& f)
            {
                dsum += f;
            }
        );
        CHECKT(ve::fuzzy_eq(dsum, dsum_expected));
    }

    void test_list() override
    {
        test_row();
        test_rows_constructor();
        test_rows_push_back();
        test_rows_append();
        test_rows_iterate();
    }
};

namespace static_tests
{
static_assert(std::is_same_v<bool_p::target_type, bool>);
static_assert(std::is_same_v<int_p::target_type, int>);
static_assert(std::is_same_v<float_p::target_type, float>);
static_assert(sizeof(table1::row) == 12);
}
