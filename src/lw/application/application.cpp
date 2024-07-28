#include "lw/application/application.hpp"

#include <pwd.h>

#include "lw/database/prepare.hpp"
#include "lw/error/code.hpp"

namespace lw::application {

namespace {

namespace po = boost::program_options;

constexpr const char help_opt[] = "help,h";
constexpr const char mysql_user_opt[] = "mysql-user,u";
constexpr const char mysql_password_opt[] = "mysql-password,p";
constexpr const char mysql_socket_opt[] = "mysql-socket,s";
constexpr const char mysql_ssl_opt[] = "mysql-ssl,S";
constexpr const char loglevel_opt[] = "loglevel";

std::string get_os_username()
{
    struct passwd *pw = ::getpwuid(::geteuid());

    // we use this function only for defaulting command line args, no need to error out here
    if(pw && pw->pw_name) {
        return pw->pw_name;
    } else {
        return "";
    }
}

boost::mysql::ssl_mode parse_ssl_mode(std::string_view mode)
{
    if(mode == "require") {
        return boost::mysql::ssl_mode::require;
    } else if(mode == "enable") {
        return boost::mysql::ssl_mode::enable;
    } else if(mode == "disable") {
        return boost::mysql::ssl_mode::disable;
    } else {
        throw error::exception{error::code::bad_cmdline_option, "invalid mysql-ssl value"};
    }
}

auto make_options_description()
{
    constexpr std::array loglevel_names = SPDLOG_LEVEL_NAMES;
    static const auto loglevel_msg =
        fmt::format("available options: {}", fmt::join(loglevel_names, ", "));

    constexpr const char mysql_ssl_msg[] = "available values: require, enable, disable";

    po::options_description desc{"learnwords-tg options"};
    // clang-format off
    desc.add_options()
        (help_opt, "produce help message")
        (mysql_socket_opt, po::value<std::string>()->default_value("/tmp/mysql.sock"))
        (loglevel_opt, po::value<std::string>(), loglevel_msg.c_str())
        (mysql_user_opt, po::value<std::string>()->default_value(get_os_username()))
        (mysql_password_opt, po::value<std::string>()->default_value(""))
        (mysql_ssl_opt, po::value<std::string>()->default_value("require"), mysql_ssl_msg)
    ;
    // clang-format on

    return desc;
}

auto parse_cmd_line(const po::options_description &desc, int argc, const char *argv[])
{
    po::variables_map ret;
    po::store(po::parse_command_line(argc, argv, desc), ret);
    po::notify(ret);
    return ret;
}

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

} // anonymous namespace

application::application(int argc, const char *argv[])
    : desc_{make_options_description()}
    , args_{parse_cmd_line(desc_, argc, argv)}
    , mysql_{io_.get_executor()}
{
}

error::code application::run()
{
    if(args_.count("help")) {
        std::ostringstream oss;
        oss << desc_;
        fmt::println("{}", oss.str());

        return error::code::ok;
    }

    boost::asio::co_spawn(io_.get_executor(), async_init(), boost::asio::detached);

    io_.run();
    return error::code::ok;
}

boost::asio::awaitable<void> application::async_init()
{
    auto mysql_user = args_["mysql-user"].as<std::string>();
    auto mysql_password = args_["mysql-password"].as<std::string>();
    auto mysql_socket = args_["mysql-socket"].as<std::string>();
    auto ssl_mode = parse_ssl_mode(args_["mysql-ssl"].as<std::string>());

    boost::mysql::connect_params params{
        .server_address = parse_mysql_address(std::move(mysql_socket)),
        .username = std::move(mysql_user),
        .password = std::move(mysql_password),
        .ssl = ssl_mode
    };

    co_await mysql_.async_connect(params, boost::asio::use_awaitable);
    co_await database::async_prepare(mysql_, boost::asio::use_awaitable);

#if 0
    inventory_builder builder{io_.get_executor()};

    // TODO: add services

    inventory_ = builder.make_inventory();
    inventory_->reload();
#endif
}

} // namespace lw::application
