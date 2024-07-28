#pragma once

#include <boost/asio/experimental/as_single.hpp>
#include <boost/asio/experimental/co_composed.hpp>

#include "lw/database/query/create.hpp"

namespace lw::database {

using async_prepare_completion_t = void(boost::system::error_code);

template<typename MysqlConnection, typename CompletionToken>
requires boost::asio::completion_token_for<CompletionToken, async_prepare_completion_t>
decltype(auto) async_prepare(MysqlConnection &conn, CompletionToken &&token)
{
    // honestly, i don't get how a coroutine may return `void`, but:
    // 1. it works
    // 2. asio documentation states that it should be `void`?
    auto impl = [](auto state, MysqlConnection &conn) -> void {
        boost::system::error_code ret;
        try {
            boost::mysql::results _;

            // i don't know why we need to use `asio::deferred` here instead of
            // `asio::use_awaitable`
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

    return boost::asio::async_initiate<CompletionToken, async_prepare_completion_t>(
        boost::asio::experimental::co_composed<async_prepare_completion_t>(
            std::move(impl), // it doesn't compile without `std::move` and i don't know why
            conn
        ),
        token,
        std::ref(conn)
    );
}

} // namespace lw::database
