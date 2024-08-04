#pragma once

#include "lw/util/scoped.hpp"

namespace lw::telegram::update {

class interface : private util::scoped
{
public:
    virtual ~interface() = default;

public:
    using signature_t = void(boost::json::value);
    using slot_t = std::function<signature_t>;

public:
    [[nodiscard]] virtual boost::signals2::scoped_connection connect(slot_t slot) = 0;
};

} // namespace lw::telegram::update
