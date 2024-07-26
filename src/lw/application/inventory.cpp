#include "lw/application/inventory.hpp"

namespace lw::application {

inventory::inventory(boost::asio::any_io_executor exec, service_list_t services)
    : exec_{std::move(exec)}
    , state_{std::move(services)}
{
}

bool inventory::active() const
{
    return std::holds_alternative<service_list_t>(state_);
}

void inventory::reload()
{
    assert(active());
    std::ranges::for_each(std::get<service_list_t>(state_), &service_base::reload);
    spdlog::info("The inventory was reloaded");
}

void inventory::stop(stop_callback_t callback)
{
    assert(active());
    auto services = std::move(std::get<service_list_t>(state_));
    state_.emplace<stopper>(exec_, std::move(services), std::move(callback));
}

// stopper
inventory::stopper::stopper(
    boost::asio::any_io_executor exec,
    service_list_t &&services,
    stop_callback_t callback
)
    : exec_{std::move(exec)}
    , reversed_services_{std::move(services)}
    , callback_{std::move(callback)}
{
    reversed_services_.reverse();
    stop();
}

void inventory::stopper::stop()
{
    const auto stoppable_it = std::ranges::find_if(reversed_services_, [](const auto &svc_ptr) {
        return !!dynamic_cast<const stoppable_service_base *>(svc_ptr.get());
    });

    reversed_services_.erase(reversed_services_.begin(), stoppable_it);
    if(stoppable_it == reversed_services_.end()) {
        boost::asio::post(exec_, std::move(callback_));
    } else {
        auto &stoppable = static_cast<stoppable_service_base &>(**stoppable_it);
        stoppable.stop([this, stoppable_it] {
            reversed_services_.erase(stoppable_it);
            stop();
        });
    }
}

} // namespace lw::application
