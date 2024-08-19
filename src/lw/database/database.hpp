#pragma once

#include "lw/database/query/create.hpp"
#include "lw/database/query/insert.hpp"
#include "lw/util/asio/co_initiate.hpp"

namespace lw::database {

class database : private siga::util::scoped
{
public:
    database(boost::mysql::any_connection &conn);

public:
    using init_handler = void(boost::system::error_code);
    using add_sentence_handler = void(boost::system::error_code, std::uint64_t);

public:
    template<boost::asio::completion_token_for<init_handler> CompletionHandler>
    auto async_init(CompletionHandler &&handler);

    template<boost::asio::completion_handler_for<add_sentence_handler> CompletionHandler>
    auto async_add_sentence(std::string sentence, CompletionHandler &&handler);

private:
    boost::mysql::any_connection &conn_;
    boost::mysql::statement add_sentence_;
};

inline database::database(boost::mysql::any_connection &conn)
    : conn_{conn}
{
}

template<boost::asio::completion_token_for<database::init_handler> CompletionHandler>
auto database::async_init(CompletionHandler &&handler)
{
    auto impl = [](auto state,
                   boost::mysql::any_connection &conn,
                   boost::mysql::statement &add_sentence) -> void {
        boost::system::error_code ret;
        try {
            boost::mysql::results _;

            // TODO: find out how to use `awaitable_operators` in order to speed up the init process
            co_await conn.async_execute(query::create_db, _, boost::asio::deferred);
            co_await conn.async_execute(query::use_db, _, boost::asio::deferred);
            co_await conn.async_execute(query::create_sentences, _, boost::asio::deferred);
            co_await conn.async_execute(query::create_settings, _, boost::asio::deferred);
            co_await conn.async_execute(query::create_user_data, _, boost::asio::deferred);

            add_sentence =
                co_await conn.async_prepare_statement(query::add_sentence, boost::asio::deferred);

            spdlog::info("The database was successfully prepared");
        } catch(const boost::system::system_error &e) {
            ret = e.code();
        }

        co_return {ret};
    };

    return util::asio::co_initiate<init_handler>(
        std::forward<CompletionHandler>(handler),
        std::move(impl),
        std::ref(conn_),
        std::ref(add_sentence_)
    );
}

template<boost::asio::completion_handler_for<database::add_sentence_handler> CompletionHandler>
auto database::async_add_sentence(std::string sentence, CompletionHandler &&handler)
{
    auto impl = [](auto state,
                   boost::mysql::any_connection &conn,
                   boost::mysql::statement &add_sentence,
                   std::string sentence) -> void {
        boost::mysql::results res;
        auto [ec] = co_await conn.async_execute(
            add_sentence.bind(std::move(sentence)),
            res,
            boost::asio::as_tuple(boost::asio::deferred)
        );

        std::uint64_t id = 0;
        if(!ec) {
            id = res.last_insert_id();
        }

        co_return {ec, id};
    };

    return util::asio::co_initiate<add_sentence_handler>(
        std::forward<CompletionHandler>(handler),
        std::ref(conn_),
        std::ref(add_sentence_),
        std::move(sentence)
    );
}

} // namespace lw::database
