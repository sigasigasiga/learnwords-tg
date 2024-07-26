#pragma once

namespace lw::database {

boost::asio::awaitable<void> prepare(boost::mysql::any_connection &conn);

} // namespace lw::database
