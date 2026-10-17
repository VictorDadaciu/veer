#include <cppunit.h>

#include <veer_core/utils.h>

#include <vql/db.h>

#include <cstdlib>
#include <memory>
#include <print>
#include <type_traits>

struct nontrivial_type
{
    inline static size_t constructed = 0zu;
    inline static size_t destroyed = 0zu;

    nontrivial_type(size_t i) : value(i)
    {
        ++constructed;
    }

    ~nontrivial_type()
    {
        ++destroyed;
    }

    size_t value{};
};

DERIVE_PROP(bool_p, PROPERTY_ROOT(bool));
DERIVE_PROP(int_p, PROPERTY_ROOT(int));
DERIVE_PROP(float_p, PROPERTY_ROOT(float));
DERIVE_PROP(char_p, PROPERTY_ROOT(char));
DERIVE_PROP(ptr_p, PROPERTY_ROOT(std::unique_ptr<nontrivial_type>));

DECLARE_LABEL(label1) {};

using table1 = TABLE(
    COLUMN_IMPL(bool_p),
    COLUMN_IMPL(int_p),
    COLUMN_IMPL(float_p),
    label1
);

using table2 = TABLE(
    COLUMN_IMPL(bool_p),
    COLUMN_IMPL(int_p),
    COLUMN_IMPL(char_p)
);

using table3 = TABLE(
    COLUMN_IMPL(int_p),
    COLUMN_IMPL(ptr_p)
);

using db = vql::database<table1, table2, table3>;

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
        table1::sheet sheet4(sheet3);
        CHECK(sheet4.count(), 0zu);
        CHECK(sheet4.capacity(), 10zu);
    }

    void test_sheet_reserve()
    {
        table1::sheet sheet1{};
        sheet1.reserve(10zu);
        CHECK(sheet1.count(), 0zu);
        CHECK(sheet1.capacity(), 10zu);
        table1::sheet sheet2 = std::move(sheet1);
        CHECK(sheet2.count(), 0zu);
        CHECK(sheet2.capacity(), 10zu);
        sheet2.reserve(50zu);
        CHECK(sheet2.count(), 0zu);
        CHECK(sheet2.capacity(), 50zu);
        table1::sheet sheet3(10);
        CHECK(sheet3.count(), 0zu);
        CHECK(sheet3.capacity(), 10zu);
        sheet3.reserve(5zu);
        CHECK(sheet3.count(), 0zu);
        CHECK(sheet3.capacity(), 10zu);
    }

    void test_sheet_push_back()
    {
        table1::sheet sheet{};
        for (size_t i = 0; i < 5; ++i)
        {
            CHECK(sheet.push_back(), i);
            size_t count = i + 1;
            CHECK(sheet.count(), count);
            CHECK(sheet.capacity(), vql::next_power_of_2(count));
            CHECKT(!sheet.cell<bool_p>(i));
            CHECK(sheet.cell<int_p>(i), 0);
            CHECKT(ve::fuzzy_eq(sheet.cell<float_p>(i), 0.f));
        }
        {
            table1::row row{};
            row.set<bool_p>(true);
            row.set<int_p>(10);
            row.set<float_p>(100.f);
            for (size_t i = 5; i < 10; ++i)
            {
                CHECK(sheet.push_back(row), i);
                size_t count = i + 1;
                CHECK(sheet.count(), count);
                CHECK(sheet.capacity(), vql::next_power_of_2(count));
                CHECKT(sheet.cell<bool_p>(i));
                CHECK(sheet.cell<int_p>(i), 10);
                CHECKT(ve::fuzzy_eq(sheet.cell<float_p>(i), 100.f));
            }
        }
        for (size_t i = 10; i < 15; ++i)
        {
            {
                table1::row row{};
                row.set<bool_p>(i % 2);
                row.set<int_p>(i);
                row.set<float_p>(i);
                CHECK(sheet.push_back(std::move(row)), i);
            }
            size_t count = i + 1;
            CHECK(sheet.count(), count);
            CHECK(sheet.capacity(), vql::next_power_of_2(count));
            CHECK(sheet.cell<bool_p>(i), i % 2);
            CHECK(sheet.cell<int_p>(i), i);
            CHECKT(ve::fuzzy_eq(sheet.cell<float_p>(i), static_cast<float>(i)));
        }
    }

    void test_sheet_append()
    {
        table1::sheet sheet{};
        for (size_t i = 0; i < 10; ++i)
        {
            CHECK(sheet.push_back(), i);
            CHECKT(!sheet.cell<bool_p>(i));
        }
        CHECK(sheet.count(), 10);
        CHECK(sheet.capacity(), 16);
        {
            table1::row row{};
            row.set<bool_p>(true);
            table1::sheet new_sheet(3);
            for (size_t i = 0; i < new_sheet.capacity(); ++i)
            {
                CHECK(new_sheet.push_back(row), i);
                CHECKT(new_sheet.cell<bool_p>(i));
            }
            CHECK(sheet.append(new_sheet), 10);
        }
        CHECK(sheet.count(), 13);
        CHECK(sheet.capacity(), 16);
    }

    void test_sheet_iterate()
    {
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

        sheet.clear();
        CHECK(sheet.count(), 0zu);
        CHECK(sheet.capacity(), 1zu);
    }

    void test_sheet_nontrivial()
    {
        table3::sheet sheet(128);
        CHECK(sheet.count(), 0zu);
        CHECK(sheet.capacity(), 128zu);
        CHECK(nontrivial_type::constructed, 0zu);
        CHECK(nontrivial_type::destroyed, 0zu);
        for (size_t i = 0; i < sheet.capacity(); ++i)
        {
            table3::row row{};
            row.cell<int_p>() = static_cast<int>(i) - 64;
            row.cell<ptr_p>() = std::make_unique<nontrivial_type>(i);
            sheet.push_back(std::move(row));
        }
        CHECK(nontrivial_type::constructed, 128zu);
        CHECK(nontrivial_type::destroyed, 0zu);
        sheet.clear();
        CHECK(sheet.count(), 0zu);
        CHECK(sheet.capacity(), 1zu);
        CHECK(nontrivial_type::constructed, 128zu);
        CHECK(nontrivial_type::destroyed, 128zu);
    }

    void test_db_at_init()
    {
        CHECK(db::count<table1>(), db::count<table2>());
        CHECK(db::capacity<table1>(), db::capacity<table2>());
    }

    void test_list() override
    {
        test_row();
        test_sheet_constructor();
        test_sheet_reserve();
        test_sheet_push_back();
        test_sheet_append();
        test_sheet_iterate();
        test_sheet_nontrivial();
        test_db_at_init();
    }
};

namespace static_tests
{
static_assert(table1::matches<SELECT()>());
static_assert(table1::matches<SELECT(label1)>());
static_assert(table1::matches<SELECT(bool_p, label1)>());
static_assert(table1::matches<SELECT(bool_p, int_p)>());

static_assert(!table1::matches<SELECT(char_p)>());
static_assert(!table1::matches<SELECT(char_p, int_p)>());
static_assert(!table1::matches<SELECT(char_p, int_p, label1)>());

static_assert(std::is_same_v<bool_p::target_type, bool>);
static_assert(std::is_same_v<int_p::target_type, int>);
static_assert(std::is_same_v<float_p::target_type, float>);
static_assert(sizeof(table1::row) == 12);
}
