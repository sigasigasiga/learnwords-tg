#pragma once

#include "lw/telegram/connection.hpp"
#include "lw/telegram/process_response.hpp"
#include "lw/telegram/update/allowed_updates.hpp"
#include "lw/telegram/update/interface.hpp"

namespace lw::telegram::update {

class long_polling : public interface
{
public:
    using start_signature_t = void(boost::system::error_code, error);

public:
    long_polling() = default;

public:
    template<typename CompletionToken>
    requires boost::asio::completion_token_for<CompletionToken, start_signature_t>
    auto async_start(
        connection &conn,
        std::chrono::seconds timeout,
        int limit,
        util::flag_set<allowed_updates> allowed,
        CompletionToken &&handler
    );

public: // interface
    [[nodiscard]] boost::signals2::scoped_connection connect(slot_t slot) final;

private:
    boost::signals2::signal<signature_t> signal_;
};

// -------------------------------------------------------------------------------------------------

template<typename CompletionToken>
requires boost::asio::completion_token_for<CompletionToken, long_polling::start_signature_t>
auto long_polling::async_start(
    connection &conn,
    std::chrono::seconds timeout,
    int limit,
    util::flag_set<allowed_updates> allowed,
    CompletionToken &&handler
)
{
    assert(1 <= limit && limit <= 100);
    auto impl = [](auto state,
                   connection &conn,
                   std::chrono::seconds timeout,
                   int limit,
                   util::flag_set<allowed_updates> allowed,
                   boost::signals2::signal<interface::signature_t> &sig) -> void {
        std::optional<std::int64_t> offset;

        for(;;) {
            boost::json::object params{
                {"limit",   limit          },
                {"timeout", timeout.count()},
            };

            if(allowed) {
                params.emplace("allowed_updates", make_allowed_updates(allowed));
            }

            if(offset) {
                params.emplace("offset", *offset);
            }

            auto [ec, updates_result] = co_await conn.async_request(
                "getUpdates",
                std::move(params),
                boost::asio::as_tuple(boost::asio::deferred)
            );

            if(ec) {
                co_return {ec, error{}};
            }

            if(updates_result) {
                auto updates = std::move(*updates_result).as_array();
                if(!updates.empty()) {
                    offset = updates.back().at("update_id").as_int64() + 1;
                }

                // FIXME?: if an exception is thrown, some updates won't be discarded
                std::ranges::for_each(updates | std::views::as_rvalue, std::ref(sig));
            } else {
                co_return {boost::system::error_code{}, std::move(updates_result).error()};
            }
        }
    };

    return util::asio::co_initiate<start_signature_t>(
        std::forward<CompletionToken>(handler),
        std::move(impl),
        std::ref(conn),
        timeout,
        limit,
        allowed,
        std::ref(signal_)
    );
}

} // namespace lw::telegram::update
