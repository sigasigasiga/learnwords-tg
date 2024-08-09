#include "lw/telegram/update/long_polling.hpp"

namespace lw::telegram::update {

boost::asio::experimental::coro<update_result, update_result> long_polling(
    connection &conn,
    std::chrono::seconds timeout,
    int limit,
    util::flag_set<allowed_updates> allowed
)
{
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

        auto updates_result =
            co_await conn.async_request("getUpdates", std::move(params), boost::asio::deferred);

        if(updates_result) {
            auto &updates = updates_result->as_array();
            if(!updates.empty()) {
                offset = updates.back().at("update_id").as_int64() + 1;
            }

            for(auto &upd : updates) {
                co_yield std::move(upd).as_object();
                // TODO: discard the update somehow
            }
        } else {
            co_yield std::unexpected{std::move(updates_result).error()};
        }
    }
}

} // namespace lw::telegram::update
