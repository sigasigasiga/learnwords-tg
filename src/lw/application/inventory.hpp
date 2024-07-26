#pragma once

#include "lw/application/service_base.hpp"

namespace lw::application {

class inventory
{
public:
    using service_list_t = std::list<std::unique_ptr<service_base>>;
    using stop_callback_t = boost::asio::any_completion_handler<void()>;

public:
    inventory(boost::asio::any_io_executor exec, service_list_t services);

public:
    bool active() const;
    void reload();
    void stop(stop_callback_t callback);

private:
    class stopper
    {
    public:
        stopper(
            boost::asio::any_io_executor exec,
            service_list_t &&services,
            stop_callback_t callback
        );

    private:
        void stop();

    private:
        boost::asio::any_io_executor exec_;
        service_list_t reversed_services_;
        stop_callback_t callback_;
    };

private:
    boost::asio::any_io_executor exec_;
    std::variant<service_list_t, stopper> state_;
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
