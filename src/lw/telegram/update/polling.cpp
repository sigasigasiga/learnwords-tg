#include "lw/telegram/update/polling.hpp"

namespace lw::telegram::update {

namespace {

boost::asio::awaitable<void> start_poll(
    polling::delegate &delegate,
    connection &conn,
    std::string_view token,
    std::chrono::seconds interval,
    boost::signals2::signal<interface::signature_t> &sig
)
{
    boost::asio::steady_timer timer{conn.get_executor()};
    for(;;) {
        timer.expires_after(interval);
        co_await timer.async_wait(boost::asio::use_awaitable);

        // TODO: pass parameters somehow
        auto raw_update =
            co_await conn.async_request(token, "getUpdates", {}, boost::asio::use_awaitable);

        if(auto update = process_response(std::move(raw_update))) {
            std::ranges::for_each(std::move(*update).as_array(), std::ref(sig));
        } else {
            delegate.on_error(std::move(update).error());
        }
    }
}

} // namespace

polling::polling(
    delegate &deleg,
    connection &conn,
    std::string_view token,
    std::chrono::seconds interval
)
{
    boost::asio::co_spawn(
        conn.get_executor(),
        start_poll(deleg, conn, token, interval, signal_),
        boost::asio::detached
    );
}

// interface
boost::signals2::connection polling::connect(slot_t slot)
{
    return signal_.connect(std::move(slot));
}

} // namespace lw::telegram::update
