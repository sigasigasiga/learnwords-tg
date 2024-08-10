#include "lw/telegram/exception.hpp"

namespace lw::telegram {

exception::exception(proto::error err)
    : boost::base_from_member<proto::error>{std::move(err)}
    , std::runtime_error{fmt::format("{} ({})", member.description, member.error_code)}
{
}

const proto::error &exception::get_error() const noexcept
{
    return member;
}

} // namespace lw::telegram
