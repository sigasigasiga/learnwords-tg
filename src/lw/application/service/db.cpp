#include "lw/application/service/db.hpp"

#include "lw/database/prepare.hpp"
#include "lw/error/code.hpp"

namespace lw::application::service {

namespace {

boost::mysql::any_address parse_mysql_address(std::string socket_address)
{
    if(socket_address.empty()) {
        throw error::exception{error::code::bad_cmdline_option, "The socket cannot be empty"};
    }

    if(std::filesystem::path{socket_address}.is_absolute()) {
        return boost::mysql::unix_path{std::move(socket_address)};
    } else {
        // TODO;
        return boost::mysql::host_and_port{};
    }
}

boost::asio::awaitable<void> initialize_db( //
    boost::mysql::any_connection &conn,
    boost::mysql::connect_params params
)
{
    co_await conn.async_connect(std::move(params), boost::asio::use_awaitable);
    co_await database::async_prepare(conn, boost::asio::use_awaitable);
}

} // anonymous namespace

db::db(
    connection::executor_type exec,
    std::string user,
    std::string password,
    std::string socket,
    boost::mysql::ssl_mode ssl
)
    : conn_{exec}
    , params_{
          .server_address = parse_mysql_address(std::move(socket)),
          .username = std::move(user),
          .password = std::move(password),
          .ssl = ssl
      }
{
}

// initializable_service_base
void db::init(init_handler_t callback)
{
    boost::asio::co_spawn(
        conn_.get_executor(),
        initialize_db(conn_, std::move(params_)),
        std::move(callback)
    );
}

} // namespace lw::application::service
