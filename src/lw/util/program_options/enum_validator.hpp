#pragma once

#include <boost/program_options.hpp>

namespace lw::util::program_options {

template<typename T>
requires std::is_enum_v<T>
void validate(boost::any &v, const std::vector<std::string> &values, T *, int)
{
    using namespace boost::program_options;

    validators::check_first_occurrence(v);
    const auto &str = validators::get_single_string(values);
    auto ret = magic_enum::enum_cast<T>(str);
    if(ret) {
        v = *ret;
    } else {
        throw validation_error{validation_error::invalid_option_value};
    }
}

} // namespace lw::util::program_options

namespace spdlog::level {

using ::lw::util::program_options::validate;
using magic_enum::iostream_operators::operator<<;

} // namespace spdlog::level

namespace boost::mysql {

using ::lw::util::program_options::validate;
using magic_enum::iostream_operators::operator<<;

} // namespace boost::mysql
