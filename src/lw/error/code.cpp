#include "lw/error/code.hpp"

namespace lw::error {

std::string_view get_description(code c) noexcept
{
    switch(c) {
        case code::bad_cmdline_option: {
            return "Bad command line opiton";
        }

        default: {
            return "If you ever see this message, contact the developer";
        }
    }
}

const char *category::name() const noexcept
{
    return "learnwords-tg";
}

std::string category::message(int condition) const
{
    return std::string(get_description(static_cast<code>(condition)));
}

exception::exception(code c)
    : std::runtime_error{fmt::format("{} ({})", get_description(c), std::to_underlying(c))}
{
}

exception::exception(code c, std::string_view prefix)
    : std::runtime_error{
          fmt::format("{}: {} ({})", prefix, get_description(c), std::to_underlying(c))
      }
{
}

std::error_condition make_error_condition(code c)
{
    static const category cat;
    return std::error_condition{std::to_underlying(c), cat};
}

} // namespace lw::error
