#include <cppunit.h>

#include <veer_core/db.h>
#include <veer_core/utils.h>

#include <cstdlib>
#include <print>
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
        row.set<int_p>(-25);
        CHECK(row.cell<int_p>(), -25);

        CHECKT(ve::fuzzy_eq(row.cell<float_p>(), 0.f));
        row.cell<float_p>() = 25.f;
        CHECKT(ve::fuzzy_eq(row.cell<float_p>(), 25.f));
        row.set<float_p>(-25.f);
        CHECKT(ve::fuzzy_eq(row.cell<float_p>(), -25.f));
    }

    void test_sheet_constructor()
    {
        table1::sheet sheet1{};
        CHECK(sheet1.count(), 0zu);
        CHECK(sheet1.capacity(), 1zu);
        table1::sheet sheet2 = std::move(sheet1);
        CHECK(sheet2.count(), 0zu);
        CHECK(sheet2.capacity(), 1zu);
        table1::sheet sheet3(10);
        CHECK(sheet3.count(), 0zu);
        CHECK(sheet3.capacity(), 10zu);
    }

    void test_sheet_push_back()
    {
        table1::default_row = table1::row();
        table1::sheet sheet{};
        for (size_t i = 0; i < 5; ++i)
        {
            CHECK(sheet.push_back(), i);
            size_t count = i + 1;
            CHECK(sheet.count(), count);
            CHECK(sheet.capacity(), ve::next_power_of_2(count));
            CHECKT(!sheet.cell<bool_p>(i));
            CHECK(sheet.cell<int_p>(i), 0);
            CHECKT(ve::fuzzy_eq(sheet.cell<float_p>(i), 0.f));
        }
        {
            table1::row row{};
            row.set<bool_p>(true);
            row.set<int_p>(10);
            row.set<float_p>(100.f);
            table1::default_row = row;
        }
        for (size_t i = 5; i < 10; ++i)
        {
            CHECK(sheet.push_back(), i);
            size_t count = i + 1;
            CHECK(sheet.count(), count);
            CHECK(sheet.capacity(), ve::next_power_of_2(count));
            CHECKT(sheet.cell<bool_p>(i));
            CHECK(sheet.cell<int_p>(i), 10);
            CHECKT(ve::fuzzy_eq(sheet.cell<float_p>(i), 100.f));
        }
        for (size_t i = 10; i < 15; ++i)
        {
            {
                table1::row row{};
                row.set<bool_p>(i % 2);
                row.set<int_p>(i);
                row.set<float_p>(i);
                CHECK(sheet.push_back(row), i);
            }
            size_t count = i + 1;
            CHECK(sheet.count(), count);
            CHECK(sheet.capacity(), ve::next_power_of_2(count));
            CHECK(sheet.cell<bool_p>(i), i % 2);
            CHECK(sheet.cell<int_p>(i), i);
            CHECKT(ve::fuzzy_eq(sheet.cell<float_p>(i), static_cast<float>(i)));
        }
    }

    void test_sheet_append()
    {
        table1::default_row = table1::row();
        table1::sheet sheet{};
        for (size_t i = 0; i < 10; ++i)
        {
            CHECK(sheet.push_back(), i);
            CHECKT(!sheet.cell<bool_p>(i));
        }
        CHECK(sheet.count(), 10);
        CHECK(sheet.capacity(), 16);
        {
            {
                table1::row row{};
                row.set<bool_p>(true);
                table1::default_row = row;
            }
            table1::sheet new_sheet(3);
            for (size_t i = 0; i < new_sheet.capacity(); ++i)
            {
                CHECK(new_sheet.push_back(), i);
                CHECKT(new_sheet.cell<bool_p>(i));
            }
            CHECK(sheet.append(new_sheet), 10);
        }
        CHECK(sheet.count(), 13);
        CHECK(sheet.capacity(), 16);
        {
            {
                table1::row row{};
                row.set<bool_p>(true);
                table1::default_row = row;
            }
            table1::sheet new_sheet(5);
            for (size_t i = 0; i < new_sheet.capacity(); ++i)
            {
                CHECK(new_sheet.push_back(), i);
                CHECKT(new_sheet.cell<bool_p>(i));
            }
            CHECK(sheet.append(new_sheet), 13);
        }
        CHECK(sheet.count(), 18);
        CHECK(sheet.capacity(), 32);
    }

    void test_sheet_iterate()
    {
        table1::default_row = table1::row();
        table1::sheet sheet(128);
        for (size_t i = 0; i < sheet.capacity(); ++i)
            sheet.push_back();
        size_t how_many_expected{};
        sheet.iterate<SELECT(bool_p)>(
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
        sheet.iterate<SELECT(bool_p, int_p)>(
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
        sheet.iterate<SELECT(int_p, float_p)>(
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
        sheet.iterate<SELECT(float_p)>(
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
        test_sheet_constructor();
        test_sheet_push_back();
        test_sheet_append();
        test_sheet_iterate();
    }
};

namespace static_tests
{
static_assert(std::is_same_v<bool_p::target_type, bool>);
static_assert(std::is_same_v<int_p::target_type, int>);
static_assert(std::is_same_v<float_p::target_type, float>);
static_assert(sizeof(table1::row) == 12);
}
