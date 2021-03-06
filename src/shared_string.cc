#include "shared_string.hh"
#include "buffer_utils.hh"

namespace Kakoune
{

StringDataPtr StringData::create(ArrayView<const StringView> strs)
{
    const int len = std::accumulate(strs.begin(), strs.end(), 0,
                                    [](int l, StringView s)
                                    { return l + (int)s.length(); });
    void* ptr = StringData::operator new(sizeof(StringData) + len + 1);
    auto* res = new (ptr) StringData(len);
    auto* data = reinterpret_cast<char*>(res + 1);
    for (auto& str : strs)
    {
        memcpy(data, str.begin(), (size_t)str.length());
        data += (int)str.length();
    }
    *data = 0;
    return RefPtr<StringData, PtrPolicy>{res};
}

StringDataPtr StringData::Registry::intern(StringView str)
{
    auto it = m_strings.find(str);
    if (it != m_strings.end())
        return StringDataPtr{it->second};

    auto data = StringData::create(str);
    data->refcount |= interned_flag;
    m_strings.emplace(data->strview(), data.get());
    return data;
}

void StringData::Registry::remove(StringView str)
{
    auto it = m_strings.find(str);
    kak_assert(it != m_strings.end());
    m_strings.erase(it);
}

void StringData::Registry::debug_stats() const
{
    write_to_debug_buffer("Shared Strings stats:");
    size_t total_refcount = 0;
    size_t total_size = 0;
    size_t count = m_strings.size();
    for (auto& st : m_strings)
    {
        total_refcount += st.second->refcount - 1;
        total_size += (int)st.second->length;
    }
    write_to_debug_buffer(format("  data size: {}, mean: {}", total_size, (float)total_size/count));
    write_to_debug_buffer(format("  refcounts: {}, mean: {}", total_refcount, (float)total_refcount/count));
}

}
