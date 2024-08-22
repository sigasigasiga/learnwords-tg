#pragma once

namespace lw::util::json {

namespace detail {

template<typename Like, typename T>
static constexpr auto ptr_to_optional_like(T *ptr)
{
    static_assert(!std::is_rvalue_reference_v<Like>);
    using copied_t = siga::util::copy_cv_ref_t<Like, T>;
    static_assert(!std::is_rvalue_reference_v<copied_t>);
    using ret_t = beman::optional26::optional<copied_t>; // either `optional<T &>` or `optional<T>`

    if(ptr) {
        return ret_t(std::forward_like<Like>(*ptr));
    } else {
        return ret_t();
    }
}

} // namespace detail

#define MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR(OP)                                                    \
    inline constexpr struct                                                                        \
    {                                                                                              \
        template<siga::util::decayable_to<boost::json::value> T>                                   \
        constexpr static auto operator()(T &&json)                                                 \
        {                                                                                          \
            return detail::ptr_to_optional_like<T>(json.OP());                                     \
        }                                                                                          \
    } OP

MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR(if_array);
MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR(if_bool);
MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR(if_double);
MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR(if_int64);
MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR(if_object);
MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR(if_string);
MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR(if_uint64);

#undef MAKE_JSON_VALUE_COND_ACCESS_FUNCTOR

class if_contains_t
{
public:
    template<siga::util::decayable_to<boost::json::object> T>
    constexpr static auto operator()(T &&object, std::string_view key)
    {
        return detail::ptr_to_optional_like<T>(object.if_contains(key));
    }

    template<siga::util::decayable_to<boost::json::array> T>
    constexpr static auto operator()(T &&array, std::size_t index)
    {
        return detail::ptr_to_optional_like<T>(array.if_contains(index));
    }
};

inline constexpr if_contains_t if_contains;

inline auto if_contains_with(auto i)
{
    return [i](auto &&v) {
        return if_contains(std::forward<decltype(v)>(v), i);
    };
}

} // namespace lw::util::json
