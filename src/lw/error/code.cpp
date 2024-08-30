#include "lw/error/code.hpp"

namespace lw::error {

std::string_view get_description(code c) noexcept
{
    switch(c) {
        case code::ok: {
            return "Everything is okay";
        }

        case code::caught_exception: {
            return "Some exception was caught";
        }

        case code::bad_cmdline_option: {
            return "Bad command line opiton";
        }

        case code::no_credentials: {
            return "No credentials were given. "
                   "See https://systemd.io/CREDENTIALS/ for more details";
        }

        case code::inconsistent_db: {
            return "Inconsistent database structure";
        }
    }

    return "If you ever see this message, contact the developer";
}

const char *category::name() const noexcept
{
    return "learnwords-tg";
}

std::string category::message(int condition) const
{
    return std::string(get_description(static_cast<code>(condition)));
}

exception::exception(enum code c)
    : std::runtime_error{fmt::format("{} ({})", get_description(c), std::to_underlying(c))}
    , c_{c}
{
}

exception::exception(enum code c, std::string_view prefix)
    : std::runtime_error{fmt::format(
          "{}: {} ({})",
          prefix,
          get_description(c),
          std::to_underlying(c)
      )}
    , c_{c}
{
}

std::error_condition make_error_condition(code c)
{
    static const category cat;
    return std::error_condition{std::to_underlying(c), cat};
}

} // namespace lw::error
