#pragma once

#include "lw/util/scoped.hpp"

namespace lw::application {

class service_base : private util::scoped
{
public:
    virtual ~service_base() = default;

public:
    virtual void reload() {}
};

class stoppable_service_base : public service_base
{
public:
    using stop_handler_t = boost::asio::any_completion_handler<void()>;

public:
    virtual void stop(stop_handler_t callback) = 0;
};

} // namespace lw::application
