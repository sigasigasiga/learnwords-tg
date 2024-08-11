#pragma once

#include <boost/program_options.hpp>

#include "lw/application/inventory.hpp"
#include "lw/error/code.hpp"
#include "lw/telegram/connection.hpp"

namespace lw::application {

class application
{
public:
    application(int argc, const char *argv[]);

public:
    error::code run();

private:
    boost::asio::awaitable<void> async_init(
        std::string mysql_user,
        std::string mysql_password,
        std::string mysql_socket,
        boost::mysql::ssl_mode ssl_mode,
        std::string telegram_token
    );

private:
    boost::asio::io_context io_;
    boost::asio::ssl::context ssl_ctx_;
    boost::program_options::options_description desc_;
    boost::program_options::variables_map args_;
    boost::mysql::any_connection mysql_;
    telegram::connection telegram_;
    std::optional<inventory> inventory_;
};

} // namespace lw::application
