#pragma once

#include <cstring>

namespace ve
{
consteval bool has_physical_size(meta_info r)
{
    return is_arithmetic_type(r) || nonstatic_data_members_of(r, std::meta::access_context::unchecked()).size() > 0;
}

consteval bool is_template_of(meta_info type, meta_info tmpl)
{
    return has_template_arguments(type) && template_of(type) == tmpl;
}

consteval bool is_label(meta_info base)
{
    if (has_physical_size(base))
        return false;
    do
    {
        base = base_of(base);
        if (base == ^^null_type)
            return false;
        else if (base == ^^label_root)
            return true;
    } while(true);
    return false;
}

consteval bool is_property(meta_info base)
{
    do
    {
        base = base_of(base);
        if (base == ^^null_type)
            return false;
        else if (is_template_of(base, ^^property_root))
            return true;
    } while(true);
    return false;
}

consteval bool is_in_property_tree(meta_info r)
{
    return is_template_of(r, ^^property_root) || is_property(r) || is_template_of(r, ^^property_impl);
}

consteval bool is_descendent_of(meta_info descendent, meta_info ancestor)
{
    do
    {
        if (descendent == ^^null_type)
            return false;
        if (descendent == ancestor)
            return true;
        descendent = base_of(descendent);
    } while(true);
}

consteval bool is_nested_in(meta_info nested, meta_info nest)
{
    return has_parent(nested) && is_template_of(parent_of(nested), nest);
}

consteval meta_info base_of(meta_info r, size_t index)
{
    if (is_fundamental_type(r))
        return ^^null_type;
    auto bases = bases_of(r, std::meta::access_context::unchecked());
    return (index < bases.size() ? type_of(bases[index]) : ^^null_type);
}

consteval meta_info table_from(meta_info r)
{
    if (!is_nested_in(r, ^^table))
        return ^^null_type;
    return parent_of(r);
}

consteval meta_infos get_columns(auto specs)
{
    meta_infos columns;
    for (auto spec : specs)
        if (is_template_of(spec, ^^packed_column) || is_template_of(spec, ^^padded_column))
            columns.push_back(spec);
    return columns;
}

consteval meta_infos get_labels(auto specs)
{
    meta_infos columns;
    for (auto spec : specs)
        if (is_label(spec))
            columns.push_back(spec);
    return columns;
}

consteval meta_infos get_properties(auto specs)
{
    meta_infos props;
    for (auto spec : specs)
        if (is_property(spec))
            props.push_back(spec);
    return props;
}

consteval meta_infos get_property_impls_from_columns(auto columns)
{
    meta_infos property_impls;
    for (auto column : columns)
    {
        meta_infos prop_impls = template_arguments_of(column);
        for (size_t i = 0; i < prop_impls.size(); ++i)
           property_impls.push_back(prop_impls[i]);
    }
    return property_impls;
}

consteval meta_infos get_properties_from_property_impls(auto prop_impls)
{
    meta_infos properties;
    for (auto prop_impl : prop_impls)
        properties.push_back(template_arguments_of(prop_impl)[0]);
    return properties;
}

consteval meta_infos make_pointers(auto columns_array)
{
    meta_infos column_pointers;
    for (meta_info r : columns_array)
        column_pointers.push_back(std::meta::add_pointer(r));
    return column_pointers;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
consteval size_t table<TableDescriptors...>::flat_property_index(meta_info r, std::vector<size_t> exclude)
{
    size_t flat_index = properties_array.size();
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
        meta_info prop = properties_array[i];
        if (r == prop)
            return i;
        if (is_descendent_of(r, prop)) // TODO: actually do depth, this only selects the last one
            flat_index = i;
    }
    return flat_index;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
consteval table<TableDescriptors...>::property_location table<TableDescriptors...>::property_location_from_flat_index(size_t flat_index)
{
    size_t current_flat_index = 0;
    for (size_t column_index = 0; column_index < columns_array.size(); ++column_index)
    {
        std::meta::info column = columns_array[column_index];
        size_t next_flat_index = current_flat_index + template_arguments_of(column).size();
        if (flat_index < next_flat_index)
            return table<TableDescriptors...>::property_location{ column_index, flat_index - current_flat_index };
        current_flat_index = next_flat_index;
    }
    throw std::meta::exception("Invalid flat index provided", ^^table, std::source_location::current());
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
consteval meta_info table<TableDescriptors...>::get_selection_indices(meta_infos props)
{
    if (props.size() == 0)
        return ^^std::index_sequence<>;
        
    std::vector<size_t> indices;
    for (auto prop : props)
    {
        size_t flat_index = flat_property_index(prop, indices);
        if (flat_index == properties_array.size())
            return ^^null_type;
        indices.push_back(flat_index);
    }
    std::vector ret{std::meta::reflect_constant(indices[0])};
    for (size_t i = 1; i < indices.size(); ++i)
        ret.push_back(std::meta::reflect_constant(indices[i]));
    return substitute(^^std::index_sequence, ret);
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
consteval bool table<TableDescriptors...>::is_label_of(meta_info label)
{
    for (meta_info table_label : labels_array)
        if (label == table_label)
            return true;
    return false;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
consteval bool table<TableDescriptors...>::matches_labels(meta_infos labels)
{
    for (meta_info label : labels)
        if (!is_label_of(label))
            return false;
    return true;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
template<Selection S>
constexpr bool table<TableDescriptors...>::matches() noexcept
{
    using Ps = S::properties_list;
    using Ls = S::labels_list;
    return matches_labels(std::meta::template_arguments_of(std::meta::dealias(^^Ls))) &&
        get_selection_indices(std::meta::template_arguments_of(std::meta::dealias(^^Ps))) != ^^null_type;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
table<TableDescriptors...>::sheet::sheet(size_t initial_capacity) : m_capacity(initial_capacity), m_count(0zu)
{
    assert(initial_capacity > 0zu);
    allocate_and_initialize_views();
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
table<TableDescriptors...>::sheet::sheet(const sheet& other) requires (std::copyable<TableDescriptors> && ...) : m_capacity(other.m_capacity), m_count(other.m_count)
{
    allocate_and_initialize_views();
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_copy_n(std::get<i>(other.m_views), m_count, std::get<i>(m_views));
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
table<TableDescriptors...>::sheet::sheet(sheet&& other) :
    m_views(std::move(other.m_views)),
    m_capacity(std::move(other.m_capacity)),
    m_count(std::move(other.m_count)),
    m_data(std::move(other.m_data)) {}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
table<TableDescriptors...>::sheet& table<TableDescriptors...>::sheet::operator=(const sheet& other)  requires (std::copyable<TableDescriptors> && ...)
{
    m_capacity = other.m_capacity;
    m_count = other.m_count;
    m_data.reset();
    allocate_and_initialize_views();
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_copy_n(std::get<i>(other.m_views), m_count, std::get<i>());
    return *this;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
table<TableDescriptors...>::sheet& table<TableDescriptors...>::sheet::operator=(sheet&& other)
{
    m_views = std::move(other.m_views);
    m_capacity = other.m_capacity;
    m_count = other.m_count;
    m_data = std::move(other.m_data);
    return *this;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::sheet::set(size_t i, const table<TableDescriptors...>::row& r) noexcept requires (std::copyable<TableDescriptors> && ...)
{
    assert(i < m_count);
    template for (constexpr size_t I : std::views::iota(0zu, columns_list::count))
        std::get<i>(m_views)[i] = std::get<i>(r.m_cols);
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::sheet::set(size_t i, table<TableDescriptors...>::row&& r) noexcept
{
    assert(i < m_count);
    template for (constexpr size_t I : std::views::iota(0zu, columns_list::count))
        std::get<i>(m_views)[i] = std::move(std::get<i>(r.m_cols));
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::sheet::set(size_t at, const table<TableDescriptors...>::sheet& other) noexcept requires (std::copyable<TableDescriptors> && ...)
{
    assert(at + other.m_count <= m_count);
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::copy_n(std::get<i>(other.m_views), other.m_count, std::get<i>(m_views) + at);
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::sheet::set(size_t at, table<TableDescriptors...>::sheet&& other) noexcept
{
    assert(at + other.m_count <= m_count);
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::copy_n(std::make_move_iterator(std::get<i>(other.m_views)), other.m_count, std::get<i>(m_views) + at);
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::sheet::push_back()
{
    size_t ret = m_count++;
    if (m_count > m_capacity)
        reallocate(calculate_new_capacity_for(m_count));
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::construct_at(std::get<i>(m_views) + ret);
    return ret;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::sheet::push_back(const table<TableDescriptors...>::row& r) requires (std::copyable<TableDescriptors> && ...)
{
    size_t ret = m_count++;
    if (m_count > m_capacity)
        reallocate(calculate_new_capacity_for(m_count));
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::construct_at(std::get<i>(m_views) + ret, std::get<i>(r.m_cols));
    return ret;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::sheet::push_back(table<TableDescriptors...>::row&& r)
{
    size_t ret = m_count++;
    if (m_count > m_capacity)
        reallocate(calculate_new_capacity_for(m_count));
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::construct_at(std::get<i>(m_views) + ret, std::move(std::get<i>(r.m_cols)));
    return ret;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::sheet::append(size_t n)
{
    assert(n > 0);
    size_t new_count = m_count + n;
    if (new_count > m_capacity)
        reallocate(calculate_new_capacity_for(new_count));
    size_t old_count = m_count;
    m_count = new_count;
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_default_construct_n(std::get<i>(m_views) + old_count, n);
    return old_count;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::sheet::append(size_t n, const table<TableDescriptors...>::row& r) requires (std::copyable<TableDescriptors> && ...)
{
    assert(n > 0);
    size_t new_count = m_count + n;
    if (new_count > m_capacity)
        reallocate(calculate_new_capacity_for(new_count));
    size_t old_count = m_count;
    m_count = new_count;
    template for (constexpr size_t I : std::views::iota(0zu, columns_list::count))
        for (size_t i = old_count; i < m_count; ++i)
            std::construct_at(std::get<I>(m_views) + i, std::get<I>(r.m_cols));
    return old_count;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::sheet::append(const table<TableDescriptors...>::sheet& other) requires (std::copyable<TableDescriptors> && ...)
{
    size_t new_count = m_count + other.m_count;
    if (new_count > m_capacity)
        reallocate(calculate_new_capacity_for(new_count));
    size_t old_count = m_count;
    m_count = new_count;
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_copy_n(std::get<i>(other.m_views), other.m_count, std::get<i>(m_views) + old_count);
    return old_count;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::sheet::append(table<TableDescriptors...>::sheet&& other)
{
    size_t new_count = m_count + other.m_count;
    if (new_count > m_capacity)
        reallocate(calculate_new_capacity_for(new_count));
    size_t old_count = m_count;
    m_count = new_count;
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_move_n(std::get<i>(other.m_views), other.m_count, std::get<i>(m_views) + old_count);
    return old_count;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::sheet::clear() noexcept
{
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        if constexpr (!std::is_trivially_destructible_v<typename [:columns_array[i]:]>)
            std::ranges::destroy_n(std::get<i>(m_views), m_count);
    m_count = 0zu;
    m_capacity = 1zu;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::sheet::reserve(size_t new_capacity)
{
    if (new_capacity > m_capacity)
        reallocate(new_capacity);
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::sheet::allocate_and_initialize_views()
{
    std::array<size_t, blocks_count> sizes{};
    template for (constexpr auto i : std::views::iota(0zu, columns_list::count))
    {
        constexpr auto r = column_pointers_array[i];
        sizes[i] = next_multiple_of_cache_line_size(size_of(remove_pointer(r)) * m_capacity);
    }
    m_data = allocate_smart_cache_aligned_bytes(std::accumulate(sizes.cbegin(), sizes.cend(), 0zu));

    std::byte* block = reinterpret_cast<std::byte*>(m_data.get());
    template for (constexpr auto i : std::views::iota(0zu, columns_list::count))
    {
        constexpr auto r = column_pointers_array[i];
        std::get<i>(m_views) = reinterpret_cast<typename[:r:]>(block);
        block += sizes[i];
    }
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::sheet::reallocate(size_t new_capacity)
{
    sheet new_sheet(new_capacity);
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_move_n(std::get<i>(m_views), m_count, std::get<i>(new_sheet.m_views));
    m_data = std::move(new_sheet.m_data);
    m_views = std::move(new_sheet.m_views);
    m_capacity = new_capacity;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::sheet::calculate_new_capacity_for(size_t needed) const noexcept
{
    assert(m_capacity < needed);
    return next_power_of_2(needed);
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
table<TableDescriptors...>::db_sheet::db_sheet(size_t initial_capacity)
{
    assert(initial_capacity > 0zu);
    m_capacity = initial_capacity;
    allocate_and_initialize_views();
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::push_back()
{
    size_t ret = m_count++;
    if (m_count > m_capacity)
        reallocate(sheet::calculate_new_capacity_for(m_count));
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::construct_at(std::get<i>(m_views) + ret);
    m_internal_data.handle_data_indices[ret] = std::numeric_limits<size_t>::max();
    m_internal_data.remove_flag[ret] = false;
    return ret;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::push_back(const table<TableDescriptors...>::row& r) requires (std::copyable<TableDescriptors> && ...)
{
    size_t ret = m_count++;
    if (m_count > m_capacity)
        reallocate(sheet::calculate_new_capacity_for(m_count));
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::construct_at(std::get<i>(m_views) + ret, std::get<i>(r.m_cols));
    m_internal_data.handle_data_indices[ret] = std::numeric_limits<size_t>::max();
    m_internal_data.remove_flag[ret] = false;
    return ret;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::push_back(table<TableDescriptors...>::row&& r)
{
    size_t ret = m_count++;
    if (m_count > m_capacity)
        reallocate(sheet::calculate_new_capacity_for(m_count));
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::construct_at(std::get<i>(m_views) + ret, std::move(std::get<i>(r.m_cols)));
    m_internal_data.handle_data_indices[ret] = std::numeric_limits<size_t>::max();
    m_internal_data.remove_flag[ret] = false;
    return ret;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::append(size_t n)
{
    assert(n > 0);
    size_t new_count = m_count + n;
    if (new_count > m_capacity)
        reallocate(sheet::calculate_new_capacity_for(new_count));
    size_t old_count = m_count;
    m_count = new_count;
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_default_construct_n(std::get<i>(m_views) + old_count, n);
    memset(m_internal_data.handle_data_indices + old_count, 0b11111111, n * sizeof(size_t));
    memset(m_internal_data.remove_flag + old_count, 0b00000000, n * sizeof(bool));
    return old_count;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::append(size_t n, const table<TableDescriptors...>::row& r) requires (std::copyable<TableDescriptors> && ...)
{
    assert(n > 0);
    size_t new_count = m_count + n;
    if (new_count > m_capacity)
        reallocate(sheet::calculate_new_capacity_for(new_count));
    size_t old_count = m_count;
    m_count = new_count;
    template for (constexpr size_t I : std::views::iota(0zu, columns_list::count))
        for (size_t i = old_count; i < m_count; ++i)
            std::construct_at(std::get<I>(m_views) + i, std::get<I>(r.m_cols));
    memset(m_internal_data.handle_data_indices + old_count, 0b11111111, n * sizeof(size_t));
    memset(m_internal_data.remove_flag + old_count, 0b00000000, n * sizeof(bool));
    return old_count;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::append(const table<TableDescriptors...>::sheet& other) requires (std::copyable<TableDescriptors> && ...)
{
    size_t new_count = m_count + other.m_count;
    if (new_count > m_capacity)
        reallocate(sheet::calculate_new_capacity_for(new_count));
    size_t old_count = m_count;
    m_count = new_count;
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_copy_n(std::get<i>(other.m_views), other.m_count, std::get<i>(m_views) + old_count);
    memset(m_internal_data.handle_data_indices + old_count, 0b11111111, other.m_count * sizeof(size_t));
    memset(m_internal_data.remove_flag + old_count, 0b00000000, other.m_count * sizeof(bool));
    return old_count;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::append(table<TableDescriptors...>::sheet&& other)
{
    size_t new_count = m_count + other.m_count;
    if (new_count > m_capacity)
        reallocate(sheet::calculate_new_capacity_for(new_count));
    size_t old_count = m_count;
    m_count = new_count;
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_move_n(std::get<i>(other.m_views), other.m_count, std::get<i>(m_views) + old_count);
    memset(m_internal_data.handle_data_indices + old_count, 0b11111111, other.m_count * sizeof(size_t));
    memset(m_internal_data.remove_flag + old_count, 0b00000000, other.m_count * sizeof(bool));
    return old_count;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
bool table<TableDescriptors...>::db_sheet::will_be_removed(size_t i) const noexcept
{
    assert(i < m_count);
    return m_internal_data.remove_flag[i];
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::remove(size_t i) noexcept
{
    assert(i < m_count);
    m_internal_data.remove_flag[i] = true;
    m_internal_data.row_has_been_removed = true;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::remove(size_t from, size_t to) noexcept
{
    size_t n = to - from;
    assert(n > 0zu && to <= m_count);
    memset(m_internal_data.remove_flag + from, true, n * sizeof(bool));
    m_internal_data.row_has_been_removed = true;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::vacuum() noexcept
{
    if (!m_internal_data.row_has_been_removed)
        return;
    m_count = vacuum_helper();
    m_internal_data.row_has_been_removed = false;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::clear() noexcept
{
    sheet::clear();
    m_internal_data.clear();
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::reserve(size_t new_capacity)
{
    if (new_capacity > m_capacity)
        reallocate(new_capacity);
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
bool table<TableDescriptors...>::db_sheet::handle_valid(const table<TableDescriptors...>::handle& h) noexcept
{
    assert(h.data_index < m_internal_data.handle_datas.size());
    return h.generation == m_internal_data.handle_datas[h.data_index].generation;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
table<TableDescriptors...>::handle table<TableDescriptors...>::db_sheet::handle_from(size_t i)
{
    assert(i < m_count);
    table<TableDescriptors...>::handle ret;
    ret.data_index = m_internal_data.handle_data_indices[i];
    if (ret.data_index != std::numeric_limits<size_t>::max())
    {
        m_internal_data.handle_data_indices[i] = ret.data_index;
        ret.generation = m_internal_data.handle_datas[ret.data_index].generation;
        return ret;
    }
    ret.data_index = m_internal_data.get_free_handle_data_index();
    handle_data& hd = m_internal_data.handle_datas[ret.data_index];
    hd.row_index = i;
    m_internal_data.handle_data_indices[i] = ret.data_index;
    ret.generation = hd.generation;
    return ret;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
table<TableDescriptors...>::index table<TableDescriptors...>::db_sheet::index_from(const table<TableDescriptors...>::handle& h) noexcept
{
    assert(h.data_index < m_internal_data.handle_datas.size());
    return handle_valid(h) ?
        index(m_internal_data.handle_datas[h.data_index].row_index) :
        index(std::numeric_limits<size_t>::max());
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::allocate_and_initialize_views()
{
    std::array<size_t, blocks_count + 2> sizes{};
    template for (constexpr auto i : std::views::iota(0zu, columns_list::count))
    {
        constexpr auto r = column_pointers_array[i];
        sizes[i] = next_multiple_of_cache_line_size(size_of(remove_pointer(r)) * m_capacity);
    }
    sizes[handle_data_index_block_index] = next_multiple_of_cache_line_size(sizeof(size_t) * m_capacity);
    sizes[remove_flag_block_index] = next_multiple_of_cache_line_size(sizeof(bool) * m_capacity);
    m_data = allocate_smart_cache_aligned_bytes(std::accumulate(sizes.cbegin(), sizes.cend(), 0zu));

    std::byte* block = reinterpret_cast<std::byte*>(m_data.get());
    template for (constexpr auto i : std::views::iota(0zu, columns_list::count))
    {
        constexpr auto r = column_pointers_array[i];
        std::get<i>(m_views) = reinterpret_cast<typename[:r:]>(block);
        block += sizes[i];
    }
    m_internal_data.handle_data_indices = reinterpret_cast<size_t*>(block);
    block += sizes[handle_data_index_block_index];
    m_internal_data.remove_flag = reinterpret_cast<bool*>(block);
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::reallocate(size_t new_capacity)
{
    db_sheet new_sheet(new_capacity);
    template for (constexpr size_t i : std::views::iota(0zu, columns_list::count))
        std::uninitialized_move_n(std::get<i>(m_views), m_count, std::get<i>(new_sheet.m_views));
    memcpy(new_sheet.m_internal_data.handle_data_indices, m_internal_data.handle_data_indices, m_count * sizeof(size_t));
    memcpy(new_sheet.m_internal_data.remove_flag, m_internal_data.remove_flag, m_count);
    m_internal_data = std::move(new_sheet.m_internal_data);
    m_data = std::move(new_sheet.m_data);
    m_views = std::move(new_sheet.m_views);
    m_capacity = new_capacity;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::vacuum_helper() noexcept
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

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::internal_data::invalidate_handle(size_t row_index) noexcept
{
    size_t handle_data_index = handle_data_indices[row_index];
    if (handle_data_index >= handle_datas.size())
        return;
    handle_datas[handle_data_index].row_index = free_handle_data_index;
    free_handle_data_index = handle_data_index;
    ++handle_datas[handle_data_index].generation;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::internal_data::refresh_handle(size_t row_index) noexcept
{
    size_t handle_data_index = handle_data_indices[row_index];
    if (handle_data_index >= handle_datas.size())
        return;
    handle_datas[handle_data_index].row_index = row_index;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
size_t table<TableDescriptors...>::db_sheet::internal_data::get_free_handle_data_index()
{
    if (free_handle_data_index == std::numeric_limits<size_t>::max())
    {
        size_t ret = handle_datas.size();
        handle_datas.push_back(handle_data(std::numeric_limits<size_t>::max(), 0zu));
        return ret;
    }
    size_t ret = free_handle_data_index;
    free_handle_data_index = handle_datas[free_handle_data_index].row_index;
    return ret;
}

template<TableDescriptor... TableDescriptors> requires (sizeof...(TableDescriptors) > 0)
void table<TableDescriptors...>::db_sheet::internal_data::clear() noexcept
{
    handle_datas.clear();
    free_handle_data_index = std::numeric_limits<size_t>::max();
    row_has_been_removed = false;
}

template<Table... Tables> requires (sizeof...(Tables) > 0)
consteval size_t database<Tables...>::index_of_table(meta_info r)
{
    for (size_t i = 0zu; i < tables_array.size(); ++i)
    {
        auto table = tables_array[i];
        if (table == r)
            return i;
    }
    return tables_array.size();
}
}