#include "lw/telegram/update/long_polling.hpp"

#include "lw/telegram/exception.hpp"

namespace lw::telegram::update {

util::asio::coroutine<boost::json::object> long_polling(
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

        if(!updates_result) {
            throw exception{std::move(updates_result).error()};
        }

        auto &updates = updates_result->as_array();
        if(!updates.empty()) {
            offset = updates.back().at("update_id").as_int64() + 1;
        }

        for(auto &upd : updates) {
            co_yield std::move(upd).as_object();
            // TODO: discard the update somehow
        }
    }
}

} // namespace lw::telegram::update
