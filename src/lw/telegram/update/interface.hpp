#pragma once

namespace lw::telegram::update {

class interface
{
public:
    virtual ~interface() = default;

public:
    using signature_t = void(boost::json::value);
    using slot_t = std::function<signature_t>;

public:
    virtual boost::signals2::connection connect(slot_t slot) = 0;
};

} // namespace lw::telegram::update
