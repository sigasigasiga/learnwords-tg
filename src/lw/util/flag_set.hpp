#pragma once

namespace lw::util {

template<typename T>
requires std::is_enum_v<T>
class flag_set
{
public:
    using value_type = std::underlying_type_t<T>;

public:
    constexpr flag_set() = default;
    /* implicit */ constexpr flag_set(T value) noexcept : value_{std::to_underlying(value)} {}

public:
    constexpr flag_set &operator&=(flag_set rhs) noexcept
    {
        value_ &= rhs.value_;
        return *this;
    }

    constexpr flag_set &operator|=(flag_set rhs) noexcept
    {
        value_ |= rhs.value_;
        return *this;
    }

    friend constexpr flag_set operator&(flag_set lhs, flag_set rhs) noexcept { return lhs &= rhs; }
    friend constexpr flag_set operator|(flag_set lhs, flag_set rhs) noexcept { return lhs |= rhs; }
    friend constexpr flag_set operator~(flag_set set) noexcept { return ~set.value_; }

public:
    constexpr value_type value() const noexcept { return value_; }
    explicit constexpr operator bool() const noexcept { return value(); }

private:
    value_type value_;
};

} // namespace lw::util
