#pragma once

#include "lw/telegram/proto/error.hpp"

namespace lw::telegram {

class exception : private boost::base_from_member<proto::error>,
                  public std::runtime_error
{
public:
    exception(proto::error err);

public:
    const proto::error &get_error() const noexcept;
};

} // namespace lw::telegram
