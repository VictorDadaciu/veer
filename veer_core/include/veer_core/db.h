#pragma once

#include "utils.h"

#include <cassert>
#include <concepts>
#include <limits>
#include <meta>
#include <numeric>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <vector>

namespace ve
{
using meta_info = std::meta::info;
using meta_infos = std::vector<meta_info>;

struct null_type final {};

template<typename T>
struct debug_type;

template<typename... Ts>
struct typelist { static constexpr size_t count = sizeof...(Ts); };

consteval bool has_physical_size(meta_info);
consteval bool is_template_of(meta_info, meta_info);
consteval bool is_in_types(meta_info, meta_infos);
consteval bool is_label(meta_info);
consteval bool is_property(meta_info);
consteval bool is_in_property_tree(meta_info);
consteval bool is_descendent_of(meta_info, meta_info);
consteval bool is_nested_in(meta_info, meta_info);

consteval meta_info base_of(meta_info, size_t=0zu);
consteval meta_info table_from(meta_info);

consteval meta_infos get_columns(auto);
consteval meta_infos get_properties(auto);
consteval meta_infos get_labels(auto);
consteval meta_infos get_property_impls_from_columns(auto);
consteval meta_infos get_properties_from_property_impls(auto);
consteval meta_infos make_pointers(auto);

struct label_root {};
template<typename T>
concept Label = is_label(^^T);

template<typename T>
concept PhysicalData = std::is_default_constructible_v<T> && std::movable<T> && has_physical_size(^^T) && !is_in_property_tree(^^T) && !Label<T>;
template<typename T>
concept Property = is_property(^^T);

template<PhysicalData T>
struct property_root
{
    using target_type = T;
    using property_root_type = property_root<T>;
};
template<typename T>
concept PropertyRoot = is_template_of(^^T, ^^property_root);

template<typename T>
concept PropertyBranch = Property<T> || PropertyRoot<T>;

template<PropertyBranch P, PhysicalData U=P::target_type>
struct property_impl final
{
    using target_type = P::target_type;
    using property_root_type = P::property_root_type;
    using property_type = P;
    using underlying_type = U;

    U value{};

    property_impl() = default;

    property_impl(const U& other) requires (std::copyable<U>) : value(other) {}
    property_impl(const property_impl& other) requires (std::copyable<U>) : value(other.value) {}
    property_impl(U&& other) : value(std::move(other)) {}
    property_impl(property_impl&& other) : value(std::move(other.value)) {}

    property_impl& operator=(const U& other) requires (std::copyable<U>)
    {
        value = other;
        return *this;
    }

    property_impl& operator=(const property_impl& other) requires (std::copyable<U>)
    {
        value = other.value;
        return *this;
    }

    property_impl& operator=(U&& other)
    {
        value = std::move(other);
        return *this;
    }

    property_impl& operator=(property_impl&& other)
    {
        value = std::move(other.value);
        return *this;
    }
};
template<typename T>
concept PropertyImpl = is_template_of(^^T, ^^property_impl);

template<PropertyImpl... PropertyImpls> requires (sizeof...(PropertyImpls) > 0)
struct packed_column final : public std::tuple<PropertyImpls...> {};
template<typename T>
concept PackedColumn = is_template_of(^^T, ^^packed_column);

template<PropertyImpl... PropertyImpls> requires (sizeof...(PropertyImpls) > 0)
struct alignas(next_power_of_2(sizeof(std::tuple<PropertyImpls...>))) padded_column final : public std::tuple<PropertyImpls...> {};
template<typename T>
concept PaddedColumn = is_template_of(^^T, ^^padded_column);

template<typename T>
concept QueryDescriptor = Property<T> || Label<T>;

template<typename T>
concept TableDescriptor = PackedColumn<T> || PaddedColumn<T> || Label<T>;

template<QueryDescriptor... QueryDescriptors>
struct query_base;

template<>
struct query_base<>
{
    using properties_list = typelist<>;
    using labels_list = typelist<>;
};

template<QueryDescriptor... QueryDescriptors>
struct query_base : public typelist<QueryDescriptors...>
{ 
private:
    static constexpr auto properties_array = std::define_static_array(get_properties(std::vector{^^QueryDescriptors...}));
    static constexpr auto labels_array = std::define_static_array(get_labels(std::vector{^^QueryDescriptors...}));

public:
    using properties_list = [:substitute(^^typelist, properties_array):];
    using labels_list = [:substitute(^^typelist, labels_array):];
};

template<QueryDescriptor... QueryDescriptors>
struct select : query_base<QueryDescriptors...>
{
    using query_base<QueryDescriptors...>::properties_list;
    using query_base<QueryDescriptors...>::labels_list;
};
template<typename T>
concept Selection = is_template_of(^^T, ^^select);

template<QueryDescriptor... QueryDescriptors>
struct dont_select : query_base<QueryDescriptors...>
{
    using query_base<QueryDescriptors...>::properties_list;
    using query_base<QueryDescriptors...>::labels_list;
};
template<typename T>
concept DontSelect = is_template_of(^^T, ^^dont_select);

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
struct table;
template<typename T>
concept Table = is_template_of(^^T, ^^table);

template<Table... Tables> requires (sizeof...(Tables) > 0)
class database;

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
struct table
{
private:
    template<Table... Tables> requires (sizeof...(Tables) > 0)
    friend class database;

    static constexpr auto descriptor_array = std::define_static_array(std::vector{^^TableDescriptors...});
    static constexpr auto columns_array = std::define_static_array(get_columns(descriptor_array));
    static constexpr auto property_impls_array = std::define_static_array(get_property_impls_from_columns(columns_array));
    static constexpr auto properties_array = std::define_static_array(get_properties_from_property_impls(property_impls_array));
    static constexpr auto labels_array = std::define_static_array(get_labels(descriptor_array));

    struct property_location
    {
        size_t column_index;
        size_t internal_index;
    };

    static consteval bool is_property_of(meta_info r) { return flat_property_index(r) < properties_array.size(); }
    static consteval bool is_label_of(meta_info);
    static consteval bool matches_labels(meta_infos);
    static consteval bool contains_labels(meta_infos);

    static consteval meta_info underlying_type_of(meta_info r) { return template_arguments_of(property_impls_array[flat_property_index(r)])[1]; }
    static consteval meta_info get_selection_indices(meta_infos);

    static consteval size_t flat_property_index(meta_info, std::vector<size_t> = {});
    static consteval property_location property_location_from_flat_index(size_t);

    struct handle_data
    {
        size_t row_index = std::numeric_limits<size_t>::max();
        size_t generation = std::numeric_limits<size_t>::max();
    };

public:
    class handle final
    {
        size_t data_index = std::numeric_limits<size_t>::max();
        size_t generation = std::numeric_limits<size_t>::max();
    };

    using columns_list = [:substitute(^^typelist, columns_array):];
    static_assert(columns_list::count > 0, "Table must have at least one column!");
    using properties_list = [:substitute(^^typelist, properties_array):];
    using labels_list = [:substitute(^^typelist, labels_array):];

    class sheet;
private:
    class db_sheet;

    static constexpr auto column_pointers_array = std::define_static_array(make_pointers(columns_array));
    static constexpr size_t blocks_count = columns_list::count;
    static constexpr size_t handle_data_index_block_index = columns_list::count;
    static constexpr size_t remove_flag_block_index = columns_list::count + 1;

public:
    template<Property P> static constexpr bool is_property_of() noexcept { return is_property_of(^^P); }
    template<Label L> static constexpr bool is_label_of() noexcept { return is_label_of(^^L); }
    template<Property P> static constexpr property_location property_location_of() noexcept { return property_location_from_flat_index(flat_property_index(^^P)); }
    template<Selection S> static constexpr bool matches() noexcept;
    template<Selection S> static constexpr bool contains() noexcept;

    VEER_STRONG_TYPEDEF(size_t, index);

    class row final
    {
        friend class sheet;
        friend class db_sheet;

    public:
        template<Property P> requires (is_property_of(^^P))
        auto& cell() noexcept
        {
            static constexpr auto [column_index, internal_index] = property_location_of<P>();
            return std::get<internal_index>(std::get<column_index>(m_cols)).value;
        }
        template<Property P> requires (is_property_of(^^P))
        const auto& cell() const noexcept
        {
            static constexpr auto [column_index, internal_index] = property_location_of<P>();
            return std::get<internal_index>(std::get<column_index>(m_cols)).value;
        }

        template<Property P, typename U> requires (is_property_of(^^P) && std::is_convertible_v<U, typename[:underlying_type_of(^^P):]>)
        void set(const U& value) noexcept
        {
            static constexpr auto [column_index, internal_index] = property_location_of<P>();
            using ConvertTo = typename[:underlying_type_of(^^P):];
            std::get<internal_index>(std::get<column_index>(m_cols)).value = static_cast<ConvertTo>(value);
        }

        template<Property P, typename U> requires (is_property_of(^^P) && std::is_convertible_v<U, typename[:underlying_type_of(^^P):]>)
        void set(U&& value) noexcept
        {
            static constexpr auto [column_index, internal_index] = property_location_of<P>();
            using ConvertTo = typename[:underlying_type_of(^^P):];
            std::get<internal_index>(std::get<column_index>(m_cols)).value = std::move(static_cast<ConvertTo>(std::move(value)));
        }

    private:
        typename [:substitute(^^std::tuple, columns_array):] m_cols;
    };

    class sheet
    {
    public:
        sheet(size_t=1zu);

        sheet(const sheet&) requires (std::copyable<TableDescriptors> && ...);
        sheet(sheet&&);
        sheet& operator=(const sheet&) requires (std::copyable<TableDescriptors> && ...);
        sheet& operator=(sheet&&);

        size_t capacity() const noexcept { return m_capacity; }
        size_t count() const noexcept { return m_count; }

        template<Property P> requires (is_property_of<P>())
        auto& cell(size_t i) noexcept
        {
            assert(i < m_count);
            static constexpr auto [column_index, internal_index] = property_location_of<P>();
            return std::get<internal_index>(std::get<column_index>(m_views)[i]).value;
        }
        
        template<Property P> requires (is_property_of<P>())
        const auto& cell(size_t i) const noexcept
        {
            assert(i < m_count);
            static constexpr auto [column_index, internal_index] = property_location_of<P>();
            return std::get<internal_index>(std::get<column_index>(m_views)[i]).value;
        }

        template<Property P, typename U> requires (is_property_of<P>() && std::is_convertible_v<U, typename[:underlying_type_of(^^P):]>)
        void set(size_t i, const U& value) noexcept
        {
            assert(i < m_count);
            static constexpr auto [column_index, internal_index] = property_location_of<P>();
            using ConvertTo = typename[:underlying_type_of(^^P):];
            std::get<internal_index>(std::get<column_index>(m_views)[i]).value = static_cast<ConvertTo>(value);
        }

        template<Property P, typename U> requires (is_property_of<P>() && std::is_convertible_v<U, typename[:underlying_type_of(^^P):]>)
        void set(size_t i, U&& value) noexcept
        {
            assert(i < m_count);
            static constexpr auto [column_index, internal_index] = property_location_of<P>();
            using ConvertTo = typename[:underlying_type_of(^^P):];
            std::get<internal_index>(std::get<column_index>(m_views)[i]).value = std::move(static_cast<ConvertTo>(std::move(value)));
        }
        
        void set(size_t, const row&) noexcept requires (std::copyable<TableDescriptors> && ...);
        void set(size_t, row&&) noexcept;
        void set(size_t, const sheet&) noexcept requires (std::copyable<TableDescriptors> && ...);
        void set(size_t, sheet&&) noexcept;

        size_t push_back();
        size_t push_back(const row&) requires (std::copyable<TableDescriptors> && ...);
        size_t push_back(row&&);

        size_t append(size_t);
        size_t append(size_t, const row&) requires (std::copyable<TableDescriptors> && ...);
        size_t append(const sheet&) requires (std::copyable<TableDescriptors> && ...);
        size_t append(sheet&&);

        void clear() noexcept;
        void reserve(size_t);

        template<typename Func>
        void iterate(Func&& func)
        {
            iterate_helper<>::iterate(std::move(func), *this);
        }

        template<Selection S, typename Func> requires (matches<S>())
        void iterate(Func&& func)
        {
            if constexpr (S::properties_list::count == 0) iterate(std::move(func));
            else
            {
                using Ps = S::properties_list;
                [:substitute(^^iterate_helper, template_arguments_of(get_selection_indices(template_arguments_of(dealias(^^Ps))))):]::iterate(std::move(func), *this);
            }
        }

        ~sheet() { clear(); }

    protected:
        void allocate_and_initialize_views();
        void reallocate(size_t);
        size_t calculate_new_capacity_for(size_t) const noexcept;

        template<size_t I> requires (I < properties_array.size())
        auto& cell(size_t i) noexcept
        {
            assert(i < m_count);
            static constexpr auto [column_index, internal_index] = table::property_location_from_flat_index(I);
            return std::get<internal_index>(std::get<column_index>(m_views)[i]).value;
        }

        template<size_t I> requires (I < properties_array.size())
        const auto& cell(size_t i) const noexcept
        {
            assert(i < m_count);
            static constexpr auto [column_index, internal_index] = property_location_from_flat_index(I);
            return std::get<internal_index>(std::get<column_index>(m_views)[i]).value;
        }

        template<size_t... Indices>
        struct iterate_helper
        {
            template<typename Func>
            static void iterate(Func&& func, sheet& s)
            {
                for (size_t i = 0zu; i < s.m_count; ++i)
                    func(index(i), s.cell<Indices>(i)...);
            }
        };

        typename[:substitute(^^std::tuple, column_pointers_array):] m_views{};
        size_t m_capacity{};
        size_t m_count{};

        std::unique_ptr<cache_aligned_bytes[]> m_data{};
    };

private:
    class db_sheet : public sheet
    {
    public:
        db_sheet(size_t=1zu);

        db_sheet(const sheet&) = delete;
        db_sheet(const db_sheet&) = delete;
        db_sheet(sheet&&) = delete;
        db_sheet(db_sheet&&) = delete;
        db_sheet& operator=(const sheet&) = delete;
        db_sheet& operator=(sheet&&) = delete;
        db_sheet& operator=(const db_sheet&) = delete;
        db_sheet& operator=(db_sheet&&) = delete;

        size_t push_back();
        size_t push_back(const row&) requires (std::copyable<TableDescriptors> && ...);
        size_t push_back(row&&);

        size_t append(size_t);
        size_t append(size_t, const row&) requires (std::copyable<TableDescriptors> && ...);
        size_t append(const sheet&) requires (std::copyable<TableDescriptors> && ...);
        size_t append(sheet&&);

        void clear() noexcept;
        void reserve(size_t);

        bool will_be_removed(size_t) const noexcept;
        void remove(size_t) noexcept;
        void remove(size_t, size_t) noexcept;

        void vacuum() noexcept;

        bool handle_valid(const handle&) noexcept;
        handle handle_from(size_t);
        index index_from(const handle& h) noexcept;

    private:
        void allocate_and_initialize_views();
        void reallocate(size_t);
        size_t vacuum_helper() noexcept;

        using sheet::m_views;
        using sheet::m_capacity;
        using sheet::m_count;
        using sheet::m_data;

        struct internal_data
        {
            void invalidate_handle(size_t) noexcept;
            void refresh_handle(size_t) noexcept;
            size_t get_free_handle_data_index();
            void clear() noexcept;

            std::vector<handle_data> handle_datas{};
            size_t free_handle_data_index = std::numeric_limits<size_t>::max();
            size_t* handle_data_indices{};
            bool* remove_flag{};
            bool row_has_been_removed{};
        } m_internal_data;
    };
};

template<Table... Tables>
struct from : public typelist<Tables...> {};
template<Table... Tables>
struct not_from : public typelist<Tables...> {};
template<typename T>
concept From = is_template_of(^^T, ^^from);
template<typename T>
concept NotFrom = is_template_of(^^T, ^^not_from);

template<typename T>
concept TableIndex = is_nested_in(^^T, ^^table) && (std::meta::identifier_of(^^T) == "index");
template<typename T>
concept TableHandle = is_nested_in(^^T, ^^table) && (std::meta::identifier_of(^^T) == "handle");
template<typename T>
concept TableRow = is_nested_in(^^T, ^^table) && (std::meta::identifier_of(^^T) == "row");
template<typename T>
concept TableSheet = is_nested_in(^^T, ^^table) && (std::meta::identifier_of(^^T) == "sheet");

template<Table... Tables> requires (sizeof...(Tables) > 0)
class database
{
    static constexpr std::array tables_array = { ^^Tables... };

    static consteval size_t index_of_table(meta_info);
    inline static consteval bool is_in_db(meta_info r)
    {
        return index_of_table(r) < tables_array.size();
    }

public:
    using tables_list = typename [:substitute(^^typelist, tables_array):];

    template<Table T> requires (is_in_db(^^T))
    inline static size_t count() noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).count();
    }

    template<Table T> requires (is_in_db(^^T))
    inline static size_t capacity() noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).capacity();
    }

    template<Table T> requires (is_in_db(^^T))
    inline static void remove(T::index i) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).remove(i);
    }

    template<Table T> requires (is_in_db(^^T))
    inline static void remove(T::index from, T::index to) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).remove(from, to);
    }

    template<Table T> requires (is_in_db(^^T))
    inline static bool will_be_removed(T::index i) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).will_be_removed(i);
    }

    template<Table T> requires (is_in_db(^^T))
    inline static void vacuum(T::index i) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).vacuum();
    }

    template<TableHandle H, Table T = typename [:table_from(^^H):]> requires (is_in_db(^^T))
    inline static bool handle_valid(H h) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).handle_valid(h);
    }

    template<TableIndex I, Table T = typename [:table_from(^^I):]> requires (is_in_db(^^T))
    inline static T::handle handle_from(I i) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).handle_from(i);
    }

    template<TableHandle H, Table T = typename [:table_from(^^H):]> requires (is_in_db(^^T))
    inline static T::index index_from(H h) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).index_from(h);
    }

    inline static void vacuum_all() noexcept
    {
        template for (constexpr auto i : std::views::iota(0zu, tables_array.size()))
            return std::get<i>(m_tables).vacuum();
    }

    template<Property P, TableIndex I, Table T = typename [:table_from(^^I):]> requires (is_in_db(^^T))
    inline static auto& cell(I i) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).template cell<P>(i);
    }

    template<Property P, TableIndex I, Table T = typename [:table_from(^^I):]> requires (is_in_db(^^T))
    inline static void set(I i, const auto& value) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).template set<P>(i, value);
    }

    template<Property P, TableIndex I, Table T = typename [:table_from(^^I):]> requires (is_in_db(^^T))
    inline static void set(I i, auto&& value) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).template set<P>(i, std::move(value));
    }

    template<TableIndex I, Table T = typename [:table_from(^^I):]> requires (is_in_db(^^T))
    inline static void set(I i, const T::row& r) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).set(i, r);
    }

    template<TableIndex I, Table T = typename [:table_from(^^I):]> requires (is_in_db(^^T))
    inline static void set(I i, T::row&& r) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).set(i, std::move(r));
    }

    template<TableIndex I, Table T = typename [:table_from(^^I):]> requires (is_in_db(^^T))
    inline static void set(I i, const T::sheet& sheet) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).set(i, sheet);
    }

    template<TableIndex I, Table T = typename [:table_from(^^I):]> requires (is_in_db(^^T))
    inline static void set(I i, T::sheet&& sheet) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).set(i, std::move(sheet));
    }

    template<Table T> requires (is_in_db(^^T))
    inline static T::index push_back() noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).push_back();
    }

    template<TableRow R, Table T = typename [:table_from(^^R):]> requires (is_in_db(^^T))
    inline static T::index push_back(const R& r) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).push_back(r);
    }

    template<TableRow R, Table T = typename [:table_from(^^R):]> requires (is_in_db(^^T))
    inline static T::index push_back(R&& r) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).push_back(std::move(r));
    }

    template<Table T> requires (is_in_db(^^T))
    inline static T::index append(size_t n) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).append(n);
    }

    template<TableRow R, Table T = typename [:table_from(^^R):]> requires (is_in_db(^^T))
    inline static T::index append(size_t n, const R& r) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).append(n, r);
    }

    template<TableSheet S, Table T = typename [:table_from(^^S):]> requires (is_in_db(^^T))
    inline static T::index append(const S& sheet) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).append(sheet);
    }

    template<TableSheet S, Table T = typename [:table_from(^^S):]> requires (is_in_db(^^T))
    inline static T::index append(S&& sheet) noexcept
    {
        return std::get<index_of_table(^^T)>(m_tables).append(std::move(sheet));
    }

    template<typename Func>
    inline static void iterate(Func&& func)
    {
        template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
            std::get<i>(m_tables).template iterate(std::move(func));
    }

    template<Selection S, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (S::count == 0)
            iterate(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
            {
                static constexpr meta_info table = tables_array[i];
                if constexpr ([:table:]::template matches<S>())
                    std::get<i>(m_tables).template iterate<S>(std::move(func));
            }
    }

    template<DontSelect N, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (N::count == 0)
            iterate(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
            {
                static constexpr meta_info table = tables_array[i];
                if constexpr (![:table:]::template contains<N>())
                    std::get<i>(m_tables).iterate(std::move(func));
            }
    }

    template<Selection S, DontSelect N, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (S::count == 0 && N::count == 0)
            iterate(std::move(func));
        else if constexpr (N::count == 0)
            iterate<S>(std::move(func));
        else if constexpr (S::count == 0)
            iterate<N>(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
            {
                static constexpr meta_info table = tables_array[i];
                if constexpr ([:table:]::template matches<S>() && ![:table:]::template contains<N>())
                    std::get<i>(m_tables).template iterate<S>(std::move(func));
            }
    }

    template<From F, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (F::count == 0)
            iterate(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, F::count))
            {
                static constexpr size_t I = index_of_table(template_arguments_of(^^F)[i]);
                if constexpr (I < tables_array.size())
                    std::get<I>(m_tables).iterate(std::move(func));
            }
    }

    template<NotFrom F, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (F::count == 0)
            iterate(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
                if constexpr (!is_in_types(tables_array[i], template_arguments_of(^^F)))
                    std::get<i>(m_tables).iterate(std::move(func));
    }

    template<Selection S, From F, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (S::count == 0 && F::count == 0)
            iterate(std::move(func));
        else if constexpr (F::count == 0)
            iterate<S>(std::move(func));
        else if constexpr (S::count == 0)
            iterate<F>(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, F::count))
            {
                static constexpr size_t I = index_of_table(template_arguments_of(^^F)[i]);
                if constexpr (I < tables_array.size())
                {
                    static constexpr meta_info table = tables_array[I];
                    if constexpr ([:table:]::template matches<S>())
                       std::get<I>(m_tables).template iterate<S>(std::move(func));
                }
            }
    }

    template<Selection S, NotFrom F, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (S::count == 0 && F::count == 0)
            iterate(std::move(func));
        else if constexpr (F::count == 0)
            iterate<S>(std::move(func));
        else if constexpr (S::count == 0)
            iterate<F>(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
                if constexpr (!is_in_types(tables_array[i], template_arguments_of(^^F)))
                {
                    static constexpr meta_info table = tables_array[i];
                    if constexpr ([:table:]::template matches<S>())
                        std::get<i>(m_tables).template iterate<S>(std::move(func));
                }
    }

    template<DontSelect N, NotFrom F, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (N::count == 0 && F::count == 0)
            iterate(std::move(func));
        else if constexpr (F::count == 0)
            iterate<N>(std::move(func));
        else if constexpr (N::count == 0)
            iterate<F>(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
                if constexpr (!is_in_types(tables_array[i], template_arguments_of(^^F)))
                {
                    static constexpr meta_info table = tables_array[i];
                    if constexpr (![:table:]::template contains<N>())
                       std::get<i>(m_tables).iterate(std::move(func));
                }
    }

    template<Selection S, DontSelect N, NotFrom F, typename Func>
    inline static void iterate(Func&& func)
    {
        if constexpr (S::count == 0 && N::count == 0 && F::count == 0)
            iterate(std::move(func));
        else if constexpr (N::count == 0 && F::count == 0)
            iterate<S>(std::move(func));
        else if constexpr (S::count == 0 && F::count == 0)
            iterate<N>(std::move(func));
        else if constexpr (S::count == 0 && N::count == 0)
            iterate<F>(std::move(func));
        else if constexpr (F::count == 0)
            iterate<S, N>(std::move(func));
        else if constexpr (N::count == 0)
            iterate<S, F>(std::move(func));
        else if constexpr (S::count == 0)
            iterate<N, F>(std::move(func));
        else
            template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
                if constexpr (!is_in_types(tables_array[i], template_arguments_of(^^F)))
                {
                    static constexpr meta_info table = tables_array[i];
                    if constexpr ([:table:]::template matches<S>() && ![:table:]::template contains<N>())
                       std::get<i>(m_tables).template iterate<S>(std::move(func));
                }
    }

    template<Table T> requires (is_in_db(^^T))
    inline static void reserve(size_t new_capacity) noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).reserve(new_capacity);
    }

    template<Table T> requires (is_in_db(^^T))
    inline static void clear() noexcept
    {
        std::get<index_of_table(^^T)>(m_tables).clear();
    }

    inline static void clear_all() noexcept
    {
        template for (constexpr size_t i : std::views::iota(0zu, tables_array.size()))
            std::get<i>(m_tables).clear();
    }

private:
    inline static std::tuple<typename Tables::db_sheet...> m_tables;
};
}

#define VEER_DERIVE_PROP(name, base)                                    \
struct name : public base                                    \
{                                                                       \
    static_assert(ve::PropertyBranch<base>);                 \
    using target_type = base::target_type;                   \
    using property_root_type = base::property_root_type;     \
    using property_type = name;                                         \
}

#define VEER_TABLE(...) ve::table<__VA_ARGS__>

#define VEER_PACKED_COLUMN(...) ve::packed_column<__VA_ARGS__>
#define VEER_PADDED_COLUMN(...) ve::padded_column<__VA_ARGS__>

#define VEER_IMPL(prop) ve::property_impl<prop>
#define VEER_IMPL_AS(prop, underlying) ve::property_impl<prop, underlying>
#define VEER_COLUMN_IMPL(prop) VEER_PACKED_COLUMN(VEER_IMPL(prop))
#define VEER_COLUMN_IMPL_AS(prop, underlying) VEER_PACKED_COLUMN(VEER_IMPL_AS(prop, underlying))

#define VEER_SELECT(...)        ve::select<__VA_ARGS__>
#define VEER_DONT_SELECT(...)   ve::dont_select<__VA_ARGS__>
#define VEER_FROM(...)          ve::from<__VA_ARGS__>
#define VEER_NOT_FROM(...)      ve::not_from<__VA_ARGS__>

#define VEER_IF_DECLTYPE_IS(auto_var, type) if constexpr (std::meta::dealias(std::meta::remove_cvref(^^decltype(auto_var))) == ^^type)
#define VEER_ELSE_IF_DECLTYPE_IS(auto_var, type) else VEER_IF_DECLTYPE_IS(auto_var, type)
#define VEER_PARAM_TYPE(param) std::remove_reference_t<std::remove_cv_t<decltype(param)>>

#define VEER_PROPERTY_ROOT(underlying) ve::property_root<underlying>

#define VEER_DECLARE_LABEL(name) struct name : public ve::label_root


#ifndef VEER_DONT_STRIP_PREFIX
#define TABLE                       VEER_TABLE

#define PACKED_COLUMN               VEER_PACKED_COLUMN
#define PADDED_COLUMN               VEER_PADDED_COLUMN

#define IMPL                        VEER_IMPL
#define IMPL_AS                     VEER_IMPL_AS
#define COLUMN_IMPL                 VEER_COLUMN_IMPL
#define COLUMN_IMPL_AS              VEER_COLUMN_IMPL_AS

#define SELECT                      VEER_SELECT
#define DONT_SELECT                 VEER_DONT_SELECT
#define FROM                        VEER_FROM
#define NOT_FROM                        VEER_NOT_FROM

#define PARAM_TYPE                  VEER_PARAM_TYPE
#define IF_DECLTYPE_IS              VEER_IF_DECLTYPE_IS
#define ELSE_IF_DECLTYPE_IS         VEER_ELSE_IF_DECLTYPE_IS

#define PROPERTY_ROOT               VEER_PROPERTY_ROOT
#define DERIVE_PROP                 VEER_DERIVE_PROP
#define DECLARE_LABEL               VEER_DECLARE_LABEL
#endif

#include "db.inl"
