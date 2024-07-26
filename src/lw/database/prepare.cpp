#include "lw/database/prepare.hpp"

namespace lw::database {

namespace {

constexpr std::string_view create_db = R"lw_cpp(
    CREATE DATABASE IF NOT EXISTS `learnwords_tg`
)lw_cpp";

constexpr std::string_view use_db = R"lw_cpp(
    USE `learnwords_tg`
)lw_cpp";

constexpr std::string_view create_sentences = R"lw_cpp(
    CREATE TABLE IF NOT EXISTS `sentences`(
        `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
        `date_added` DATE DEFAULT(CURRENT_DATE),
        `sentence` TINYTEXT NOT NULL,

        PRIMARY KEY(`id`)
    )
        CHARACTER SET utf8 COLLATE utf8_general_ci
)lw_cpp";

constexpr std::string_view create_settings = R"lw_cpp(
    CREATE TABLE IF NOT EXISTS `settings`(
        `tg_user_id` BIGINT UNSIGNED NOT NULL,
        `remind_time` TIME NOT NULL CHECK(HOUR(`remind_time`) < 24),

        PRIMARY KEY(`tg_user_id`)
    )
)lw_cpp";

constexpr std::string_view create_user_data = R"lw_cpp(
    CREATE TABLE IF NOT EXISTS `user_data`(
        `tg_user_id` BIGINT UNSIGNED NOT NULL,
        `sentence_id` BIGINT UNSIGNED NOT NULL,

        PRIMARY KEY(`tg_user_id`, `sentence_id`),
        CONSTRAINT
            FOREIGN KEY(`tg_user_id`) REFERENCES `settings`(`tg_user_id`)
            ON DELETE CASCADE
            ON UPDATE CASCADE,
        CONSTRAINT
            FOREIGN KEY(`sentence_id`) REFERENCES `sentences`(`id`)
            ON DELETE CASCADE
            ON UPDATE CASCADE
    )
)lw_cpp";

} // anonymous namespace

boost::asio::awaitable<void> prepare(boost::mysql::any_connection &conn)
{
    boost::mysql::results _;
    co_await conn.async_execute(create_db, _, boost::asio::use_awaitable);
    co_await conn.async_execute(use_db, _, boost::asio::use_awaitable);
    co_await conn.async_execute(create_sentences, _, boost::asio::use_awaitable);
    co_await conn.async_execute(create_settings, _, boost::asio::use_awaitable);
    co_await conn.async_execute(create_user_data, _, boost::asio::use_awaitable);
}

} // namespace lw::database
