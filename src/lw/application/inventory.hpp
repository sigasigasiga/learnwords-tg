#pragma once

#include "lw/application/service_base.hpp"

namespace lw::application {

// How to use the `inventory`:
// 1. Call `init`
// 2. After the `init` is done, call `reload`
// 3. You may then call `reload` every time config changes
// 4. On shutdown call `stop`
class inventory
{
public:
    using service_ptr_t = std::unique_ptr<service_base>;
    using service_list_t = std::list<service_ptr_t>;

    using init_handler_t = boost::asio::any_completion_handler<void(std::exception_ptr)>;
    using stop_handler_t = boost::asio::any_completion_handler<void()>;

public:
    inventory(boost::asio::any_io_executor exec, service_list_t services);

public:
    void async_init(init_handler_t callback);
    void reload();
    void async_stop(stop_handler_t callback);

private:
    enum class state
    {
        init,
        in_progress,
        stopping,
    };

private:
    boost::asio::any_io_executor exec_;
    service_list_t services_;
    state state_ = state::init;
};

class inventory_builder
{
public:
    inventory_builder(boost::asio::any_io_executor exec) : exec_{std::move(exec)} {}

public:
    template<typename T, typename... Args>
    [[nodiscard]] T &add_service(Args &&...args)
    {
        static_assert(std::convertible_to<T *, service_base *>);
        auto svc_ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto &svc_ref = *svc_ptr;
        services_.emplace_back(std::move(svc_ptr));
        return svc_ref;
    }

    [[nodiscard]] inventory make_inventory()
    {
        assert(!services_.empty());
        return inventory{std::move(exec_), std::move(services_)};
    }

private:
    boost::asio::any_io_executor exec_;
    inventory::service_list_t services_;
};

} // namespace lw::application
