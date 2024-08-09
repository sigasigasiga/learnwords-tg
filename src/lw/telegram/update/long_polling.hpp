#pragma once

#include "lw/telegram/connection.hpp"
#include "lw/telegram/update/allowed_updates.hpp"

namespace lw::telegram::update {

using update_result = std::expected<boost::json::object, error>;

[[nodiscard]] boost::asio::experimental::coro<update_result, update_result> long_polling(
    connection &conn,
    std::chrono::seconds timeout,
    int limit,
    util::flag_set<allowed_updates> allowed
);

} // namespace lw::telegram::update
