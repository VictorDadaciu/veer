#pragma once

#include "utils.h"

#include <assert.h>
#include <cmath>
#include <cstring>
#include <concepts>
#include <memory>
#include <meta>
#include <numeric>
#include <ranges>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>

#define VEER_TABLE(...) ve::table<__VA_ARGS__>

#define VEER_ROW_INDEX(table, index)  table::row_index(index)
#define VEER_PACKED_COLUMN(...) ve::column<false, __VA_ARGS__>
#define VEER_PADDED_COLUMN(...) ve::column<true,  __VA_ARGS__>

#define VEER_IMPL(prop) ve::property_impl<prop>
#define VEER_IMPL_AS(prop, underlying) ve::property_impl<prop, underlying>
#define VEER_COLUMN_IMPL(prop) VEER_PACKED_COLUMN(VEER_IMPL(prop))
#define VEER_COLUMN_IMPL_AS(prop, underlying) VEER_PACKED_COLUMN(VEER_IMPL_AS(prop, underlying))

#define VEER_SELECT(...)  ve::select<__VA_ARGS__>
#define VEER_FROM(...)    ve::from<__VA_ARGS__>
#define VEER_UNSORTED()   ve::sort_by<>
#define VEER_SORT_BY(...) ve::sort_by<__VA_ARGS__>

#define VEER_IF_DECLTYPE_IS(auto_var, type) if constexpr (std::meta::dealias(std::meta::remove_cvref(^^decltype(auto_var))) == ^^type)
#define VEER_ELSE_IF_DECLTYPE_IS(auto_var, type) else VEER_IF_DECLTYPE_IS(auto_var, type)
#define VEER_PARAM_TYPE(param) std::remove_reference_t<std::remove_cv_t<decltype(param)>>

#define VEER_PROPERTY_ROOT(underlying) ve::property_root<underlying>

#define VEER_DERIVE_PROP(name, property_base_t)                            \
struct name : public property_base_t                                       \
{                                                                          \
    static_assert(ve::is_property_branch(^^property_base_t));              \
    using target_type = property_base_t::target_type;                      \
    using property_root_type = property_base_t::property_root_type;        \
    using property_type = name;                                            \
}

#ifndef VEER_KEEP_PREFIX
#define TABLE                       VEER_TABLE

#define ROW_INDEX                   VEER_ROW_INDEX
#define PACKED_COLUMN               VEER_PACKED_COLUMN
#define PADDED_COLUMN               VEER_PADDED_COLUMN

#define IMPL                        VEER_IMPL
#define IMPL_AS                     VEER_IMPL_AS
#define COLUMN_IMPL                 VEER_COLUMN_IMPL
#define COLUMN_IMPL_AS              VEER_COLUMN_IMPL_AS

#define SELECT                      VEER_SELECT
#define FROM                        VEER_FROM
#define UNSORTED                    VEER_UNSORTED
#define SORT_BY                     VEER_SORT_BY

#define PARAM_TYPE                  VEER_PARAM_TYPE
#define IF_DECLTYPE_IS              VEER_IF_DECLTYPE_IS
#define ELSE_IF_DECLTYPE_IS         VEER_ELSE_IF_DECLTYPE_IS

#define PROPERTY_ROOT               VEER_PROPERTY_ROOT
#define DERIVE_PROP                 VEER_DERIVE_PROP
#endif

namespace ve
{
static constexpr bool packed = false;
static constexpr bool padded = true;

struct null_type final {};

template<typename... type_ts>
struct type_list
{
    static constexpr size_t count = sizeof...(type_ts);
};

consteval auto base_of(std::meta::info r, size_t index=0zu)
{
    if (is_fundamental_type(r))
        return ^^null_type;
    auto bases = bases_of(r, std::meta::access_context::unchecked());
    return (index < bases.size() ? type_of(bases[index]) : ^^null_type);
}

template<class type_t>
concept simple_c = std::is_default_constructible_v<type_t> && std::copyable<type_t> && std::movable<type_t>;

consteval bool has_physical_size(std::meta::info r)
{
    return is_arithmetic_type(r) || nonstatic_data_members_of(r, std::meta::access_context::unchecked()).size() > 0;
}

template<simple_c target_t>
struct property_root;

template<class property_t, typename underlying_t=property_t::target_type>
struct property_impl;

consteval bool is_property(std::meta::info base)
{
    do
    {
        base = base_of(base);
        if (base == ^^null_type)
            return false;
        else if (is_template_of(base, ^^property_root))
            return true;
    } while(true);
}

consteval bool is_property_branch(std::meta::info r)
{
    return is_template_of(r, ^^property_root) || is_property(r);
}

consteval bool is_in_property_tree(std::meta::info r)
{
    return is_template_of(r, ^^property_root) || is_property(r) || is_template_of(r, ^^property_impl);
}

consteval bool is_descendent_of(std::meta::info descendent_r, std::meta::info ancestor_r)
{
    do
    {
        if (descendent_r == ^^null_type)
            return false;
        if (descendent_r == ancestor_r)
            return true;
        descendent_r = base_of(descendent_r);
    } while(true);
}

template<simple_c target_t>
    requires (has_physical_size(^^target_t) && !is_in_property_tree(^^target_t))
struct property_root<target_t>
{
    using target_type = target_t;
    using property_root_type = property_root<target_t>;
};

template<class property_t, simple_c underlying_t>
    requires (has_physical_size(^^underlying_t)  && !is_in_property_tree(^^underlying_t) && is_property_branch(^^property_t))
struct property_impl<property_t, underlying_t> final
{
    using target_type = property_t::target_type;
    using property_root_type = property_t::property_root_type;
    using property_type = property_t;
    using underlying_type = underlying_t;
    
    property_impl() = default;
    property_impl(const underlying_type& other) : value(other) {}
    property_impl(const property_impl& other) : value(other.value) {}
    property_impl(underlying_type&& other) : value(other) {}
    property_impl(property_impl&& other) : value(other.value) {}

    property_impl& operator=(const underlying_type& other)
    {
        value = other;
        return *this;
    }

    property_impl& operator=(underlying_type&& other)
    {
        value = other.value;
        return *this;
    }

    property_impl& operator=(const property_impl& other)
    {
        value = other.value;
        return *this;
    }

    property_impl& operator=(property_impl&& other)
    {
        value = other.value;
        return *this;
    }

    target_type get() const
    {
        return static_cast<target_type>(value);
    }

    void set(const target_type& other)
    {
        value = static_cast<underlying_type>(other);
    }

    underlying_t value{};
};

template<typename column_tuple>
consteval size_t column_alignment(bool is_padded)
{
    return is_padded ? next_power_of_2(sizeof(column_tuple)) : alignof(column_tuple);
}

template<bool is_padded_v, class... property_impl_ts>
    requires (sizeof...(property_impl_ts) > 0 && (is_template_of(^^property_impl_ts, ^^property_impl) && ...))
struct alignas(column_alignment<std::tuple<property_impl_ts...>>(is_padded_v))
    column : public std::tuple<property_impl_ts...>
{
    static constexpr bool is_padded = is_padded_v;
};

consteval bool is_table_spec(std::meta::info r)
{
    return is_template_of(r, ^^column); // TODO: labels
}

// TODO: labels
consteval auto get_columns(auto spec_rs)
{
    std::vector<std::meta::info> columns;
    for (auto spec_r : spec_rs)
        if (is_template_of(spec_r, ^^column))
            columns.push_back(spec_r);
    return columns;
}

consteval auto get_properties(auto spec_rs)
{
    std::vector<std::meta::info> props;
    for (auto spec_r : spec_rs)
        if (is_property(spec_r))
            props.push_back(spec_r);
    return props;
}

consteval std::vector<std::meta::info> get_property_impls_from_columns(auto column_rs)
{
    std::vector<std::meta::info> property_impls;
    for (auto column_r : column_rs)
    {
        std::vector<std::meta::info> prop_impl_rs = template_arguments_of(column_r);
        for (size_t i = 1; i < prop_impl_rs.size(); ++i)
           property_impls.push_back(prop_impl_rs[i]);
    }
    return property_impls;
}

consteval std::vector<std::meta::info> get_properties_from_property_impls(auto property_impls_rs)
{
    std::vector<std::meta::info> properties;
    for (auto prop_impl_r : property_impls_rs)
        properties.push_back(template_arguments_of(prop_impl_r)[0]);
    return properties;
}

// TODO: labels
template<class... spec_ts>
    requires (is_property(^^spec_ts) && ...)
struct select;

template<>
struct select<>
{
    using properties = type_list<>;
};

template<class... spec_ts>
    requires (is_property(^^spec_ts) && ...)
struct select
{
private:
    static constexpr auto properties_array = std::define_static_array(get_properties(std::vector{^^spec_ts...}));

public:
    using properties = [:substitute(^^type_list, properties_array):];
};

template<class... spec_ts>
    requires (is_table_spec(^^spec_ts) && ...)
struct table;

consteval auto make_pointers(auto columns_array)
{
    std::vector<std::meta::info> column_pointers;
    for (std::meta::info r : columns_array)
        column_pointers.push_back(std::meta::add_pointer(r));
    return column_pointers;
}

template<class table_t>
    requires (is_template_of(^^table_t, ^^table))
VEER_STRONG_TYPEDEF(size_t, row_index);

struct row_handle_data final
{
    size_t row_index = std::numeric_limits<size_t>::max();
    size_t generation = std::numeric_limits<size_t>::max();
};

template<bool is_in_database>
struct table_internal_data;

template<>
struct table_internal_data<false> {};

template<class table_t>
    requires (is_template_of(^^table_t, ^^table))
class table_row;

template<class table_t, bool is_database_table=false>
    requires (is_template_of(^^table_t, ^^table))
class table_rows;

template<class table_t>
    requires (is_template_of(^^table_t, ^^table))
class row_handle final
{
    friend class table_rows<table_t, true>;

    size_t data_index = std::numeric_limits<size_t>::max();
    size_t generation = std::numeric_limits<size_t>::max();
};

template<>
struct table_internal_data<true>
{
    void invalidate_handle(size_t row_index) noexcept
    {
        size_t handle_data_index = handle_data_indices[row_index];
        if (handle_data_index >= handle_datas.size())
            return;
        handle_datas[handle_data_index].row_index = free_handle_data_index;
        free_handle_data_index = handle_data_index;
        ++handle_datas[handle_data_index].generation;
    }

    void refresh_handle(size_t row_index) noexcept
    {
        size_t handle_data_index = handle_data_indices[row_index];
        if (handle_data_index >= handle_datas.size())
            return;
        handle_datas[handle_data_index].row_index = row_index;
    }

    size_t get_free_handle_data_index()
    {
        if (free_handle_data_index == std::numeric_limits<size_t>::max())
        {
            size_t ret = handle_datas.size();
            handle_datas.push_back(row_handle_data(std::numeric_limits<size_t>::max(), 0zu));
            return ret;
        }
        size_t ret = free_handle_data_index;
        free_handle_data_index = handle_datas[free_handle_data_index].row_index;
        return ret;
    }

    std::vector<row_handle_data> handle_datas{};
    size_t free_handle_data_index = std::numeric_limits<size_t>::max();
    size_t* handle_data_indices{};
    bool* remove_flag{};
    bool row_has_been_removed{};
};

template<class... spec_ts>
    requires (is_table_spec(^^spec_ts) && ...)
struct table
{
private:
    friend class table_row<table>;
    friend class table_rows<table, false>;
    friend class table_rows<table, true>;

    static constexpr auto columns_array = std::define_static_array(get_columns(std::vector{^^spec_ts...}));
    static constexpr auto property_impls_array = std::define_static_array(get_property_impls_from_columns(columns_array));
    static constexpr auto properties_array = std::define_static_array(get_properties_from_property_impls(property_impls_array));

public:
    using columns = [:substitute(^^type_list, columns_array):];
    static_assert(columns::count > 0, "Table must have at least one column");
    using properties = [:substitute(^^type_list, properties_array):];

    using index  = row_index<table>;
    using handle = row_handle<table>;
    using row    = table_row<table>;
    using rows   = table_rows<table>;

private:
    static consteval size_t flat_property_index(std::meta::info candidate_r, std::vector<size_t> exclude={})
    {
        size_t flat_index = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < properties_array.size(); ++i)
        {
            bool skip = false;
            for (size_t exc : exclude)
            {
                if (exc == i)
                {
                    skip = true;       
                    break;
                }
            }
            if (skip) continue;
            std::meta::info prop_r = properties_array[i];
            if (candidate_r == prop_r)
                return i;
            if (is_descendent_of(candidate_r, prop_r)) // TODO: actually do depth, this only selects the last one
                flat_index = i;
        }
        return flat_index;
    }

    struct physical_location
    {
        size_t column_index;
        size_t internal_index;
    };

    static consteval physical_location physical_location_from_flat_index(size_t flat_index)
    {
        size_t current_flat_index = 0;
        for (size_t column_index = 0; column_index < columns_array.size(); ++column_index)
        {
            std::meta::info column_r = columns_array[column_index];
            size_t next_flat_index = current_flat_index + template_arguments_of(column_r).size() - 1; // remove packed boolean
            if (flat_index < next_flat_index)
                return physical_location{ column_index, flat_index - current_flat_index };
            current_flat_index = next_flat_index;
        }
        throw std::meta::exception("Invalid flat index provided", ^^table, std::source_location::current());
    }

    static consteval bool is_property_of(std::meta::info candidate_r)
    {
        return flat_property_index(candidate_r) < std::numeric_limits<size_t>::max();
    }

    static consteval std::meta::info underlying_type_of(std::meta::info property_r)
    {
        return template_arguments_of(property_impls_array[flat_property_index(property_r)])[1];
    }

    static consteval std::meta::info get_selection_indices(std::vector<std::meta::info> selection_property_rs)
    {
        if (selection_property_rs.size() == 0)
            return ^^std::index_sequence<>;
            
        std::vector<size_t> indices;
        for (auto property_r : selection_property_rs)
        {
            size_t flat_index = flat_property_index(property_r, indices);
            if (flat_index == std::numeric_limits<size_t>::max())
                return ^^null_type;
            indices.push_back(flat_index);
        }
        std::vector ret{std::meta::reflect_constant(indices[0])};
        for (size_t i = 1; i < indices.size(); ++i)
            ret.push_back(std::meta::reflect_constant(indices[i]));
        return substitute(^^std::index_sequence, ret);
    }

public:
    template<class candidate_property_t>
        requires (is_property(^^candidate_property_t))
    static constexpr bool is_property_of()
    {
        return is_property_of(^^candidate_property_t);
    }

    template<class candidate_property_t>
        requires (is_property_of(^^candidate_property_t))
    static constexpr physical_location physical_location_of()
    {
        return physical_location_from_flat_index(flat_property_index(^^candidate_property_t));
    }

    template<class candidate_selection_t>
        requires (is_template_of(^^candidate_selection_t, ^^select))
    static constexpr bool matches()
    {
        using selection_properties = candidate_selection_t::properties;
        return get_selection_indices(std::meta::template_arguments_of(std::meta::dealias(^^selection_properties))) != ^^null_type;
    }
};

template<class table_t>
    requires (is_template_of(^^table_t, ^^table))
class table_row
{
    friend class table_rows<table_t, false>;
    friend class table_rows<table_t, true>;

public:
    template<class candidate_property_t>
        requires (table_t::template is_property_of<candidate_property_t>())
    auto& cell() noexcept
    {
        static constexpr auto [column_index, internal_index] = table_t::template physical_location_of<candidate_property_t>();
        return std::get<internal_index>(std::get<column_index>(m_cols)).value;
    }
    template<class candidate_property_t>
        requires (table_t::template is_property_of<candidate_property_t>())
    const auto& cell() const noexcept
    {
        static constexpr auto [column_index, internal_index] = table_t::template physical_location_of<candidate_property_t>();
        return std::get<internal_index>(std::get<column_index>(m_cols)).value;
    }

    template<class candidate_property_t, typename value_t>
        requires (table_t::template is_property_of<candidate_property_t>() && std::is_convertible_v<value_t, typename[:table_t::underlying_type_of(^^candidate_property_t):]>)
    void set_cell(const value_t& value) noexcept
    {
        static constexpr auto [column_index, internal_index] = table_t::template physical_location_of<candidate_property_t>();
        using convert_to_t = typename[:table_t::underlying_type_of(^^candidate_property_t):];
        std::get<internal_index>(std::get<column_index>(m_cols)).value = static_cast<convert_to_t>(value);
    }

private:
    using columns = table_t::columns;
    typename [:substitute(^^std::tuple, template_arguments_of(dealias(^^columns))):] m_cols{};
};

#define _VEER_IF_IN_DATABASE template<bool t_ = is_database_table, std::enable_if_t<t_, bool> = true>
#define _VEER_IF_NOT_IN_DATABASE template<bool t_ = is_database_table, std::enable_if_t<!t_, bool> = true>

template<class table_t, bool is_database_table>
    requires (is_template_of(^^table_t, ^^table))
class table_rows
{
    friend class table_rows<table_t, !is_database_table>;

    static constexpr auto column_pointers_array = std::define_static_array(make_pointers(table_t::columns_array));
    static constexpr size_t blocks_count = table_t::columns::count + (is_database_table ? 2 : 0);
    static constexpr size_t handle_data_index_block_index = blocks_count - 2;
    static constexpr size_t remove_flag_block_index = blocks_count - 1;

    void allocate_and_initialize_views()
    {
        std::array<size_t, blocks_count> sizes{};
        template for (constexpr auto i : std::views::iota(0zu, table_t::columns::count))
        {
            constexpr auto r = column_pointers_array[i];
            sizes[i] = next_multiple_of_cache_line_size(size_of(remove_pointer(r)) * m_capacity);
        }
        if constexpr (is_database_table)
        {
            sizes[handle_data_index_block_index] = next_multiple_of_cache_line_size(sizeof(size_t) * m_capacity);
            sizes[remove_flag_block_index] = next_multiple_of_cache_line_size(sizeof(bool) * m_capacity);
        }
        m_data = allocate_smart_cache_aligned_bytes(std::accumulate(sizes.cbegin(), sizes.cend(), 0zu));

        std::byte* block = reinterpret_cast<std::byte*>(m_data.get());
        template for (constexpr auto i : std::views::iota(0zu, table_t::columns::count))
        {
            constexpr auto r = column_pointers_array[i];
            std::get<i>(m_views) = reinterpret_cast<typename[:r:]>(block);
            block += sizes[i];
        }
        if constexpr (is_database_table)
        {
            m_internal_data.handle_data_indices = reinterpret_cast<size_t*>(block);
            block += sizes[handle_data_index_block_index];
            
            m_internal_data.remove_flag = reinterpret_cast<bool*>(block);
            block += sizes[remove_flag_block_index];
        }
    }

    void reallocate(size_t new_capacity)
    {
        table_rows new_rows(new_capacity);
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::uninitialized_move_n(std::get<i>(m_views), m_count, std::get<i>(new_rows.m_views));
        if constexpr (is_database_table)
        {
            std::uninitialized_move_n(m_internal_data.handle_data_indices, m_count, new_rows.m_internal_data.handle_data_indices);
            std::uninitialized_move_n(m_internal_data.remove_flag, m_count, new_rows.m_internal_data.remove_flag);
            m_internal_data = std::move(new_rows.m_internal_data);
        }
        m_data = std::move(new_rows.m_data);
        m_views = std::move(new_rows.m_views);
        m_capacity = new_capacity;
    }

    size_t calculate_new_capacity_for(size_t needed) const noexcept
    {
        assert(m_capacity < needed);
        size_t new_capacity = m_capacity;
        do new_capacity *= 2; while (new_capacity < needed);
        return new_capacity;
    }

    _VEER_IF_IN_DATABASE
    size_t vacuum_helper() noexcept
    {
        size_t l = 0;
        size_t r = m_count - 1;
        bool* remove_flag = m_internal_data.remove_flag;
        while (true)
        {
            while(true)
            {
                if (l > r) return l;
                if (remove_flag[l]) break;
                m_internal_data.invalidate_handle(l++);
            }

            while(true)
            {
                if (l >= r) return l;
                if (!remove_flag[r]) break;
                --r;
            }

            template for (constexpr size_t i : std::views::iota(0zu, column_pointers_array.size() - 1))
            {
                std::destroy_at(std::get<i>(m_views) + l);
                std::construct_at(std::get<i>(m_views) + l, std::move(std::get<i>(m_views)[r]));
            }
            remove_flag[l] = false;
            remove_flag[r] = true;
            std::swap(m_internal_data.handle_data_indices[l], m_internal_data.handle_data_indices[r]);
            m_internal_data.invalidate_handle(r--);
            m_internal_data.refresh_handle(l++);
        }
    }

    template<size_t prop_index>
        requires (prop_index < table_t::properties_array.size())
    auto& cell(size_t index) noexcept
    {
        assert(index < m_count);
        static constexpr auto [column_index, internal_index] = table_t::physical_location_from_flat_index(prop_index);
        return std::get<internal_index>(std::get<column_index>(m_views)[index]).value;
    }

    template<size_t prop_index>
        requires (prop_index < table_t::properties_array.size())
    const auto& cell(size_t index) const noexcept
    {
        assert(index < m_count);
        static constexpr auto [column_index, internal_index] = table_t::template physical_location_from_flat_index(prop_index);
        return std::get<internal_index>(std::get<column_index>(m_views)[index]).value;
    }

    template<size_t... indices>
    struct iterate_helper
    {
        template<typename func_t>
        static void iterate(func_t&& func, table_rows& rows)
        {
            for (size_t i = 0zu; i < rows.m_count; ++i)
                func(row_index<table_t>(i), rows.cell<indices>(i)...);
        }
    };

public:
    table_rows(size_t initial_capacity=1zu) 
        : m_capacity(initial_capacity), m_count(0zu)
    {
        assert(initial_capacity > 0zu);
        allocate_and_initialize_views();
    }

    _VEER_IF_IN_DATABASE
    table_rows(const table_rows&) = delete;
    _VEER_IF_IN_DATABASE
    table_rows(table_rows&&) = delete;
    _VEER_IF_IN_DATABASE
    table_rows& operator=(const table_rows&) = delete;
    _VEER_IF_IN_DATABASE
    table_rows& operator=(table_rows&&) = delete;

    _VEER_IF_NOT_IN_DATABASE
    table_rows(const table_rows& other)
        : m_capacity(other.m_capacity), m_count(other.m_count)
    {
        allocate_and_initialize_views();
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::uninitialized_copy_n(std::get<i>(other.m_views), m_count, std::get<i>());
    }

    _VEER_IF_NOT_IN_DATABASE
    table_rows(table_rows&& other) noexcept
        : m_views(std::move(other.m_views)), m_capacity(other.m_capacity), m_count(other.m_count), m_data(std::move(other.m_data)) {}

    _VEER_IF_NOT_IN_DATABASE
    table_rows& operator=(const table_rows& other)
    {
        m_capacity = other.m_capacity;
        m_count = other.m_count;
        m_data.reset();
        allocate_and_initialize_views();
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::uninitialized_copy_n(std::get<i>(other.m_views), m_count, std::get<i>());
        return *this;
    }

    _VEER_IF_NOT_IN_DATABASE
    table_rows& operator=(table_rows&& other) noexcept
    {
        m_views = std::move(other.m_views);
        m_capacity = other.m_capacity;
        m_count = other.m_count;
        m_data = std::move(other.m_data);
        return *this;
    }

    size_t capacity() const noexcept 
    { 
        return m_capacity;
    }

    size_t count() const noexcept 
    { 
        return m_count;
    }

    _VEER_IF_IN_DATABASE
    bool will_be_removed(size_t index) const noexcept
    {
        assert(index < m_count);
        return std::get<0>(std::get<table_t::columns::count>(m_views)[index]).value;
    }

    _VEER_IF_IN_DATABASE
    void remove(size_t index) noexcept
    {
        assert(index < m_count);
        m_internal_data.remove_flag[index] = true;
        m_internal_data.row_has_been_removed = true;
    }

    _VEER_IF_IN_DATABASE
    void remove(size_t from, size_t to) noexcept
    {
        size_t n = to - from;
        assert(n > 0zu && to <= m_count);
        memset(reinterpret_cast<void*>(m_internal_data.remove_flag + from), true, n * sizeof(bool));
        m_internal_data.row_has_been_removed = true;
    }

    template<class candidate_property_t>
        requires (table_t::template is_property_of<candidate_property_t>())
    auto& cell(size_t index) noexcept
    {
        assert(index < m_count);
        static constexpr auto [column_index, internal_index] = table_t::template physical_location_of<candidate_property_t>();
        return std::get<internal_index>(std::get<column_index>(m_views)[index]).value;
    }

    template<class candidate_property_t>
        requires (table_t::template is_property_of<candidate_property_t>())
    const auto& cell(size_t index) const noexcept
    {
        assert(index < m_count);
        static constexpr auto [column_index, internal_index] = table_t::template physical_location_of<candidate_property_t>();
        return std::get<internal_index>(std::get<column_index>(m_views)[index]).value;
    }

    template<class candidate_property_t, typename value_t>
        requires (table_t::template is_property_of<candidate_property_t>() && std::is_convertible_v<value_t, typename[:table_t::underlying_type_of(^^candidate_property_t):]>)
    void set_cell(size_t index, const value_t& value) const noexcept
    {
        assert(index < m_count);
        static constexpr auto [column_index, internal_index] = table_t::template physical_location_of<candidate_property_t>();
        using convert_to_t = typename[:table_t::underlying_type_of(^^candidate_property_t):];
        std::get<internal_index>(std::get<column_index>(m_views)[index]).value = static_cast<convert_to_t>(value);
    }

    table_t::row row(size_t index) const noexcept
    {
        assert(index < m_count);
        typename table_t::row ret;
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::get<i>(ret.m_cols) = std::get<i>(m_views)[index];
        return ret;
    }

    table_t::row operator[](size_t index) const noexcept
    {
        return row(index);
    }

    void set_row(size_t index, const table_t::row& row) noexcept
    {
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::get<i>(m_views)[index] = std::get<i>(row.m_cols);
    }

    table_t::rows rows(size_t from, size_t to) const noexcept
    {
        size_t n = to - from;
        assert(n > 0zu && to <= m_count);
        typename table_t::rows ret(n);
        ret.m_count = n;
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::uninitialized_copy_n(std::get<i>(m_views) + from, n, std::get<i>(ret.m_views));
        return ret;
    }

    table_t::rows operator[](size_t from, size_t to) const noexcept
    {
        return rows(from, to);
    }

    void set_rows(size_t at, const table_t::rows& other) noexcept
    {
        assert(at + other.m_count <= m_count);
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::copy_n(std::get<i>(other.m_views), other.m_count, std::get<i>(m_views) + at);
    }
    
    size_t push_back(const table_t::row& row)
    {
        size_t ret = m_count++;
        if (m_count > m_capacity)
            reallocate(calculate_new_capacity_for(m_count));
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::construct_at(std::get<i>(m_views) + ret, std::get<i>(row.m_cols));
        if constexpr (is_database_table)
        {
            m_internal_data.handle_data_indices[ret] = std::numeric_limits<size_t>::max();
            m_internal_data.remove_flag[ret] = false;
        }
        return ret;
    }

    size_t append(const table_t::rows& other)
    {
        size_t new_count = m_count + other.m_count;
        if (new_count > m_capacity)
            reallocate(calculate_new_capacity_for(new_count));
        size_t old_count = m_count;
        m_count = new_count;
        template for (constexpr size_t i : std::views::iota(0zu, table_t::columns::count))
            std::uninitialized_copy_n(std::get<i>(other.m_views), other.m_count, std::get<i>(m_views) + old_count);
        if constexpr (is_database_table)
        {
            memset(reinterpret_cast<void*>(m_internal_data.handle_data_indices + old_count), 0b11111111, other.m_count * sizeof(size_t));
            memset(reinterpret_cast<void*>(m_internal_data.remove_flag + old_count), 0b00000000, other.m_count * sizeof(bool));
        }
        return old_count;
    }

    template<class selection_t, typename func_t>
        requires (is_template_of(^^selection_t, ^^select) && table_t::template matches<selection_t>())
    void iterate(func_t&& func)
    {
        [:substitute(^^iterate_helper, template_arguments_of(table_t::get_selection_indices(template_arguments_of(^^selection_t)))):]::iterate(std::move(func), *this);
    }

    _VEER_IF_IN_DATABASE
    void vacuum() noexcept
    {
        if (!m_internal_data.row_has_been_removed)
            return;
        m_count = vacuum_helper();
        m_internal_data.row_has_been_removed = false;
    }

    _VEER_IF_IN_DATABASE
    row_handle<table_t> handle_from(size_t index)
    {
        assert(index < m_count);
        row_handle<table_t> ret;
        ret.data_index = m_internal_data.handle_data_indices[index];
        if (ret.data_index != std::numeric_limits<size_t>::max())
        {
            m_internal_data.handle_data_indices[index] = ret.data_index;
            ret.generation = m_internal_data.handle_datas[ret.data_index].generation;
            return ret;
        }
        ret.data_index = m_internal_data.get_free_handle_data_index();
        row_handle_data& handle_data = m_internal_data.handle_datas[ret.data_index];
        handle_data.row_index = index;
        m_internal_data.handle_data_indices[index] = ret.data_index;
        ret.generation = handle_data.generation;
        return ret;
    }

    _VEER_IF_IN_DATABASE
    bool handle_valid(const table_t::handle& handle) noexcept
    {
        assert(handle.data_index < m_internal_data.handle_datas.size());
        return handle.generation == m_internal_data.handle_datas[handle.data_index].generation;
    }

    _VEER_IF_IN_DATABASE
    table_t::index index_from(const table_t::handle& handle) noexcept
    {
        assert(handle.data_index < m_internal_data.handle_datas.size());
        if (!handle_valid(handle))
            return row_index<table_t>(std::numeric_limits<size_t>::max());
        return row_index<table_t>(m_internal_data.handle_datas[handle.data_index].row_index);
    }

private:
    typename[:substitute(^^std::tuple, column_pointers_array):] m_views{};
    size_t m_capacity{};
    size_t m_count{};

    std::unique_ptr<cache_aligned_bytes[]> m_data{};
    [[no_unique_address]] table_internal_data<is_database_table> m_internal_data;
};

template<class... table_ts>
    requires (is_template_of(^^table_ts, ^^table) && ...)
struct from : public type_list<table_ts...> {};

template<class... table_ts>
    requires (sizeof...(table_ts) > 0 && (is_template_of(^^table_ts, ^^table) && ...))
class database final
{
    static constexpr std::array tables_array = { ^^table_ts... };
    using storage_tuple_type = std::tuple<table_rows<table_ts, true>...>;
    inline static storage_tuple_type m_tables;

    inline static consteval size_t index_of_table(std::meta::info r)
    {
        for (size_t i = 0zu; i < tables_array.size(); ++i)
        {
            auto table_r = tables_array[i];
            if (table_r == r)
               return i;
        }
        return tables_array.size();
    }

    inline static consteval bool is_in_database(std::meta::info r)
    {
        return index_of_table(r) < tables_array.size();
    }

public:
    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static size_t count() noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).count();
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static size_t capacity() noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).capacity();
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static void remove(row_index<table_t> index) noexcept
    {
        std::get<index_of_table(^^table_t)>(m_tables).remove(index);
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static void remove(row_index<table_t> from, row_index<table_t> to) noexcept
    {
        std::get<index_of_table(^^table_t)>(m_tables).remove(from, to);
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static bool will_be_removed(row_index<table_t> index) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).will_be_removed(index);
    }

    template<class candidate_property_t, class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static auto& cell(row_index<table_t> index) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).template cell<candidate_property_t>(index);
    }

    template<class candidate_property_t, class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static void set_cell(row_index<table_t> index, const auto& value) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).template set_cell<candidate_property_t>(index, value);
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static table_t::row row(row_index<table_t> index) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables)[index];
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static void set_row(row_index<table_t> index, const table_row<table_t>& to_set) noexcept
    {
        std::get<index_of_table(^^table_t)>(m_tables).set_row(index, to_set);
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static table_t::rows rows(row_index<table_t> from, row_index<table_t> to) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables)[from, to];
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static void set_rows(row_index<table_t> index, const table_rows<table_t>& to_set) noexcept
    {
        std::get<index_of_table(^^table_t)>(m_tables).set_rows(index, to_set);
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static size_t push_back(const table_row<table_t>& to_push) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).push_back(to_push);
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static size_t append(const table_rows<table_t>& to_push) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).append(to_push);
    }

    template<class selection_t, class from_t, typename func_t>
        requires (is_template_of(^^selection_t, ^^select) && is_template_of(^^from_t, ^^from))
    inline static void iterate(func_t&& func)
    {
        if constexpr (from_t::count == 0)
        {
            template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
            {
                static constexpr std::meta::info table_r = tables_array[i];
                if constexpr ([:table_r:]::template matches<selection_t>())
                   std::get<i>(m_tables).template iterate<selection_t>(std::move(func));
            }
        }
        else
        {
            template for (constexpr size_t i : std::views::iota(0zu, from_t::count))
               std::get<index_of_table(template_arguments_of(^^from_t)[i])>(m_tables).template iterate<selection_t>(std::move(func));
        }
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    inline static void vacuum() noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).vacuum();
    }

    inline static void vacuum_all() noexcept
    {
        template for (constexpr auto i : std::views::iota(0zu, tables_array.size()))
            return std::get<i>(m_tables).vacuum();
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    static row_handle<table_t> handle_from(row_index<table_t> index)
    {
        return std::get<index_of_table(^^table_t)>(m_tables).handle_from(index);
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    static bool handle_valid(row_handle<table_t> handle) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).handle_valid(handle);
    }

    template<class table_t>
        requires (is_template_of(^^table_t, ^^table) && is_in_database(^^table_t))
    static row_index<table_t> index_from(row_handle<table_t> handle) noexcept
    {
        return std::get<index_of_table(^^table_t)>(m_tables).index_from(handle);
    }
};
}
