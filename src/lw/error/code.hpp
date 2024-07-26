#pragma once

namespace lw::error {

enum class code : int
{
    ok = 0,
    bad_cmdline_option = 1,

    // don't use codes bigger than 125
    // https://unix.stackexchange.com/a/418802
};

std::string_view get_description(code c) noexcept;

class category : public std::error_category
{
public: // std::error_category
    const char *name() const noexcept final;
    std::string message(int condition) const final;
};

// I don't know why there's no `std::system_error` for `std::error_condition` in STL
class exception : public std::runtime_error
{
public:
    exception(code c);
    exception(code c, std::string_view prefix);
};

std::error_condition make_error_condition(code c);

} // namespace lw::error

template<>
struct std::is_error_condition_enum<lw::error::code> : public std::true_type
{};
