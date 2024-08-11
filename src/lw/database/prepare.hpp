#pragma once

#include "lw/database/query/create.hpp"
#include "lw/util/asio/co_initiate.hpp"

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

            spdlog::info("The database was successfully prepared");
        } catch(const boost::system::system_error &e) {
            ret = e.code();
        }

        co_return {ret};
    };

    return util::asio::co_initiate<async_prepare_completion_t>(
        std::forward<CompletionToken>(token),
        std::move(impl),
        std::ref(conn)
    );
}

} // namespace lw::database
