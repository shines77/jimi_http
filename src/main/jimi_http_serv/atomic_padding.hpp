
#pragma once

#include <atomic>
#include <type_traits>

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE  64
#endif

#pragma pack(push)
#pragma pack(1)

namespace jimi {

////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

//
// See: http://en.cppreference.com/w/cpp/experimental/type_trait_variable_templates
// See: http://en.cppreference.com/w/cpp/types/is_arithmetic
// See: http://en.cppreference.com/w/cpp/types/is_class
// See: http://en.cppreference.com/w/cpp/types/is_union (union and enum class can't be inherite)
//

template <typename T, bool isArithmetic = false>
struct is_inheritable : std::integral_constant<bool,
                        std::is_class<T>::value  &&
#if (defined(__cplusplus) && (__cplusplus >= 201300L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900L))
                        !std::is_final<T>::value &&
#endif // std::is_final<T> only can use in C++ 14.
                        !std::is_volatile<T>::value> {};

//
// See: http://en.cppreference.com/w/cpp/types/decay
//
// std::decay<T>: remove the const and volatile, reference and move statement, convert array and function to pointer type.
//
//      Applies lvalue-to-rvalue, array-to-pointer, and function-to-pointer implicit conversions to the type T,
//      removes cv-qualifiers, and defines the resulting type as the member typedef type.
//
template <typename T, bool isArithmetic = false>
struct is_inheritable_decay : std::integral_constant<bool,
                        std::is_class<typename std::decay<T>::type>::value  &&
#if (defined(__cplusplus) && (__cplusplus >= 201300L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900L))
                        !std::is_final<typename std::decay<T>::type>::value &&
#endif // std::is_final<T> only can use in C++ 14.
                        !std::is_volatile<typename std::decay<T>::type>::value> {};

} // namespace detail

////////////////////////////////////////////////////////////////////////////////////////

struct root_padding_data {};

template <typename T>
struct is_padding_data
{
    enum { value = std::is_base_of<T, root_padding_data>::value };
};

template <typename T, std::size_t CacheLineSize>
struct base_padding_data : public root_padding_data
{
    typedef T value_type;

    static const std::size_t kCacheLineSize = CacheLineSize;
    static const std::size_t kSizeOfData = sizeof(value_type);
    static const std::size_t kPaddingBytes =
        (kSizeOfData <= kCacheLineSize) ? (kCacheLineSize - kSizeOfData)
        : (((kSizeOfData - 1) / kCacheLineSize + 1) * kCacheLineSize - kSizeOfData);
};

template <typename T, std::size_t CacheLineSize>
struct base_padding_data_decay : public base_padding_data<typename std::decay<T>::type, CacheLineSize>
{
    typedef typename std::decay<T>::type value_type;
};

////////////////////////////////////////////////////////////////////////////////////////

template <typename T, std::size_t CacheLineSize, bool isInheritable = true>
struct alignas(CacheLineSize) padding_data_impl : public std::decay<T>::type,
                                                  public base_padding_data_decay<T, CacheLineSize>
{
    typedef typename std::decay<T>::type value_type;

    typedef base_padding_data_decay<T, CacheLineSize> base_type;
    static const std::size_t kCacheLineSize = base_type::kCacheLineSize;
    static const std::size_t kSizeOfData = base_type::kSizeOfData;
    static const std::size_t kPaddingBytes = base_type::kPaddingBytes;

    // Cacheline padding
    char padding[kPaddingBytes];

    padding_data_impl(value_type const & value) : value_type(value) {}
    ~padding_data_impl() {}
};

template <typename T, std::size_t CacheLineSize>
struct padding_data_impl<T, CacheLineSize, false> : public base_padding_data_decay<T, CacheLineSize>
{
    typedef typename std::decay<T>::type value_type;

    typedef base_padding_data_decay<T, CacheLineSize> base_type;
    static const std::size_t kCacheLineSize = base_type::kCacheLineSize;
    static const std::size_t kSizeOfData = base_type::kSizeOfData;
    static const std::size_t kPaddingBytes = base_type::kPaddingBytes;

    // T aligned to cacheline size
    alignas(CacheLineSize) value_type data;

    // Cacheline padding
    char padding[kPaddingBytes];

    padding_data_impl(value_type value) : data(value) {}
    ~padding_data_impl() {}
};

template <typename T, std::size_t CacheLineSize = CACHE_LINE_SIZE>
struct padding_data : public padding_data_impl<T, CacheLineSize, detail::is_inheritable_decay<T>::value> {};

////////////////////////////////////////////////////////////////////////////////////////

template <typename T, std::size_t CacheLineSize, bool isInheritable = true>
struct alignas(CacheLineSize) volatile_padding_data_impl : public base_padding_data_decay<T, CacheLineSize>
{
    typedef typename std::decay<T>::type value_type;

    typedef base_padding_data_decay<T, CacheLineSize> base_type;
    static const std::size_t kCacheLineSize = base_type::kCacheLineSize;
    static const std::size_t kSizeOfData = base_type::kSizeOfData;
    static const std::size_t kPaddingBytes = base_type::kPaddingBytes;

    // (volatile T) aligned to cacheline size
    alignas(CacheLineSize) volatile value_type data;

    // Cacheline padding
    char padding[kPaddingBytes];

    volatile_padding_data_impl(value_type const & value) : data(value) {}
    ~volatile_padding_data_impl() {}
};

template <typename T, std::size_t CacheLineSize>
struct volatile_padding_data_impl<T, CacheLineSize, false> : public base_padding_data_decay<T, CacheLineSize>
{
    typedef typename std::decay<T>::type value_type;

    typedef base_padding_data_decay<T, CacheLineSize> base_type;
    static const std::size_t kCacheLineSize = base_type::kCacheLineSize;
    static const std::size_t kSizeOfData = base_type::kSizeOfData;
    static const std::size_t kPaddingBytes = base_type::kPaddingBytes;

    // (volatile T) aligned to cacheline size
    alignas(CacheLineSize) volatile value_type data;

    // Cacheline padding
    char padding[kPaddingBytes];

    volatile_padding_data_impl(value_type value) : data(value) {}
    ~volatile_padding_data_impl() {}
};

template <typename T, std::size_t CacheLineSize = CACHE_LINE_SIZE>
struct volatile_padding_data : public volatile_padding_data_impl<T, CacheLineSize,
                                      detail::is_inheritable_decay<T>::value> {};

////////////////////////////////////////////////////////////////////////////////////////

template <typename T, std::size_t CacheLineSize = CACHE_LINE_SIZE>
struct alignas(CacheLineSize) atomic_padding : public std::atomic<typename std::decay<T>::type>,
                                               public base_padding_data_decay<T, CacheLineSize>
{
    typedef typename std::decay<T>::type value_type;
    typedef std::atomic<value_type> atomic_type;

    typedef base_padding_data<atomic_type, CacheLineSize> base_type;
    static const std::size_t kCacheLineSize = base_type::kCacheLineSize;
    static const std::size_t kSizeOfData = base_type::kSizeOfData;
    static const std::size_t kPaddingBytes = base_type::kPaddingBytes;

    // Cacheline padding
    char padding[kPaddingBytes];

    atomic_padding(value_type const & value) : std::atomic<value_type>(value) {}
    ~atomic_padding() {}

    atomic_padding & operator = (value_type const & that) {
        atomic_type * pThisAtomic =  static_cast<atomic_type *>(this);
        if ((void *)&that != (void *)pThisAtomic)
            *pThisAtomic = that;
        return *this;
    }

    value_type & get() const {
        atomic_type * pThisAtomic = static_cast<atomic_type *>(const_cast<atomic_padding *>(this));
        value_type * pThisValue = static_cast<value_type *>(&(pThisAtomic->_My_val));
        assert(pThisValue != nullptr);
        return *pThisValue;
    }

    atomic_type & getAtomic() const {
        atomic_type * pThisAtomic = static_cast<atomic_type *>(const_cast<atomic_padding *>(this));
        assert(pThisAtomic != nullptr);
        return *pThisAtomic;
    }
};

template <typename T, std::size_t CacheLineSize = CACHE_LINE_SIZE>
struct atomic_padding_wrapper : public base_padding_data_decay<T, CacheLineSize>
{
    typedef typename std::decay<T>::type value_type;
    typedef std::atomic<value_type> atomic_type;

    typedef base_padding_data<atomic_type, CacheLineSize> base_type;
    static const std::size_t kCacheLineSize = base_type::kCacheLineSize;
    static const std::size_t kSizeOfData = base_type::kSizeOfData;
    static const std::size_t kPaddingBytes = base_type::kPaddingBytes;

    // std::atomic<T> aligned to cacheline size
    alignas(CacheLineSize) atomic_type data;

    // Cacheline padding
    char padding[kPaddingBytes];

    atomic_padding_wrapper(value_type value) : data(value) {}
    ~atomic_padding_wrapper() {}

    atomic_padding_wrapper & operator = (value_type value) {
        data = value;
        return *this;
    }

    value_type & get() const {
        value_type * pAtomicValue = const_cast<value_type *>(&(data._My_val));
        assert(pAtomicValue != nullptr);
        return *pAtomicValue;
    }

    atomic_type & getAtomic() const {
        return *(const_cast<atomic_type *>(&data));
    }
};

////////////////////////////////////////////////////////////////////////////////////////

} // namespace jimi

#pragma pack(pop)

#undef CACHE_LINE_SIZE
