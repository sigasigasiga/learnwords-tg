#include "lw/application/application.hpp"

#include <pwd.h>

#include "lw/application/service/db.hpp"
#include "lw/application/service/tg.hpp"
#include "lw/application/service/user_dialog.hpp"
#include "lw/error/code.hpp"

namespace lw::application {

namespace {

namespace po = boost::program_options;

constexpr const char help_opt[] = "help,h";
constexpr const char mysql_user_opt[] = "mysql-user,u";
constexpr const char mysql_password_opt[] = "mysql-password,p";
constexpr const char mysql_socket_opt[] = "mysql-socket,s";
constexpr const char mysql_ssl_opt[] = "mysql-ssl,S";
constexpr const char telegram_token_opt[] = "telegram-token,t";
constexpr const char loglevel_opt[] = "loglevel";

constexpr std::array loglevel_names = SPDLOG_LEVEL_NAMES;

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
    static const auto loglevel_msg =
        fmt::format("available options: {}", fmt::join(loglevel_names, ", "));

    constexpr const char mysql_ssl_msg[] = "available values: require, enable, disable";

    po::options_description desc{"learnwords-tg options"};
    // TODO: all the auth must be done in the systemd's fashion
    // https://systemd.io/CREDENTIALS/

    // clang-format off
    desc.add_options()
        (help_opt, "produce help message")
        (mysql_socket_opt, po::value<std::string>()->default_value("/tmp/mysql.sock"))
        (loglevel_opt, po::value<std::string>(), loglevel_msg.c_str())
        (mysql_user_opt, po::value<std::string>()->default_value(get_os_username()))
        (mysql_password_opt, po::value<std::string>()->default_value(""))
        (mysql_ssl_opt, po::value<std::string>()->default_value("require"), mysql_ssl_msg)
        (telegram_token_opt, po::value<std::string>())
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

void set_loglevel(const std::string &loglevel_str)
{
    if(std::ranges::find(loglevel_names, loglevel_str) == loglevel_names.end()) {
        throw error::exception{error::code::bad_cmdline_option, "Unknown loglevel"};
    }

    const auto loglevel = spdlog::level::from_str(loglevel_str);
    spdlog::set_level(loglevel);
}

std::string make_token(const boost::program_options::variables_map &args)
{
    if(const auto token_arg = args["telegram-token"].as<std::string>(); !token_arg.empty()) {
        return token_arg;
    } else if(const auto token_env = std::getenv("LEARNWORDS_TELEGRAM_TOKEN")) {
        return token_env;
    } else {
        throw error::exception{
            error::code::no_telegram_token,
            "No telegram token. Set it with `-t` or `LEARNWORDS_TELEGRAM_TOKEN` env var"
        };
    }
}

inventory make_inventory(
    boost::asio::io_context::executor_type exec,
    boost::asio::ssl::context &ssl_ctx,
    std::string bot_token,
    std::string mysql_user,
    std::string mysql_password,
    std::string mysql_socket,
    boost::mysql::ssl_mode ssl
)
{
    inventory_builder builder{exec};

    auto &tg = builder.add_service<service::tg>(
        exec,
        ssl_ctx,
        std::move(bot_token),
        service::tg::update_method::long_polling
    );

    std::ignore = builder.add_service<service::db>(
        exec,
        std::move(mysql_user),
        std::move(mysql_password),
        std::move(mysql_socket),
        ssl
    );

    std::ignore = builder.add_service<service::user_dialog>(tg.get_connection(), tg.get_update());

    return builder.make_inventory();
}

} // anonymous namespace

application::application(int argc, const char *argv[])
    : ssl_ctx_{boost::asio::ssl::context::tls_client}
    , desc_{make_options_description()}
    , args_{parse_cmd_line(desc_, argc, argv)}
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

    if(args_.count(loglevel_opt)) {
        const auto &loglevel_str = args_[loglevel_opt].as<std::string>();
        set_loglevel(loglevel_str);
    }

    ssl_ctx_.set_default_verify_paths();

    inventory_ = make_inventory(
        io_.get_executor(),
        ssl_ctx_,
        make_token(args_),
        args_["mysql-user"].as<std::string>(),
        args_["mysql-password"].as<std::string>(),
        args_["mysql-socket"].as<std::string>(),
        parse_ssl_mode(args_["mysql-ssl"].as<std::string>())
    );

    inventory_->async_init(std::bind_front(&application::on_init, this));

    io_.run();
    return error::code::ok;
}

void application::on_init(std::exception_ptr ep)
{
    if(ep) {
        std::rethrow_exception(ep);
    }

    inventory_->reload();
}

} // namespace lw::application
