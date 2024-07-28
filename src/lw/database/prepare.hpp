#pragma once

#include <boost/asio/experimental/as_single.hpp>
#include <boost/asio/experimental/co_composed.hpp>

#include "lw/database/queries.hpp"

namespace lw::database {

template<typename MysqlConnection, typename CompletionToken>
requires boost::asio::completion_token_for<CompletionToken, void(boost::system::error_code)>
decltype(auto) async_prepare(MysqlConnection &conn, CompletionToken &&token)
{
    // honestly, i don't get how a coroutine may return `void`, but:
    // 1. it works
    // 2. asio documentation states that it should be `void`?
    auto impl = [](auto state, MysqlConnection &conn) -> void {
        boost::system::error_code ret;
        try {
            boost::mysql::results _;
            co_await conn.async_execute(query::create_db, _, boost::asio::deferred);
            co_await conn.async_execute(query::use_db, _, boost::asio::deferred);
            co_await conn.async_execute(query::create_sentences, _, boost::asio::deferred);
            co_await conn.async_execute(query::create_settings, _, boost::asio::deferred);
            co_await conn.async_execute(query::create_user_data, _, boost::asio::deferred);
        } catch(const boost::system::system_error &e) {
            ret = e.code();
        }

        co_return {ret};
    };

    return boost::asio::async_initiate<CompletionToken, void(boost::system::error_code)>(
        boost::asio::experimental::co_composed<void(boost::system::error_code)>(
            std::move(impl), // it doesn't compile without `std::move` and i don't know why
            conn
        ),
        token,
        std::ref(conn)
    );
}

} // namespace lw::database
