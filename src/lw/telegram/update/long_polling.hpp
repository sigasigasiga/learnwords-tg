#pragma once

#include "lw/telegram/connection.hpp"
#include "lw/telegram/update/allowed_updates.hpp"
#include "lw/util/asio/coroutine.hpp"

namespace lw::telegram::update {

[[nodiscard]] util::asio::coroutine<boost::json::object> long_polling(
    connection &conn,
    std::chrono::seconds timeout,
    int limit = 100,
    siga::util::flag_set<allowed_updates> allowed = {}
);

} // namespace lw::telegram::update
