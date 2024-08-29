#include "lw/application/application.hpp"

#include <pwd.h>

#include "lw/application/service/db.hpp"
#include "lw/application/service/tg.hpp"
#include "lw/application/service/user_dialog.hpp"
#include "lw/error/code.hpp"
#include "lw/util/program_options/enum_validator.hpp"

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

auto make_options_description()
{
    constexpr auto raw_loglevels = magic_enum::enum_values<spdlog::level::level_enum>();
    static_assert(raw_loglevels.back() == spdlog::level::level_enum::n_levels);
    std::span loglevels{
        std::ranges::begin(raw_loglevels),
        std::ranges::prev(std::ranges::end(raw_loglevels))
    };

    // TODO: get available values using `magic_enum`
    static const auto loglevel_msg =
        fmt::format("available options: {}", fmt::join(loglevels, ", "));

    static const auto mysql_ssl_msg = fmt::format(
        "available options: {}",
        fmt::join(magic_enum::enum_values<boost::mysql::ssl_mode>(), ", ")
    );

    po::options_description desc{"learnwords-tg options"};
    // TODO: all the auth must be done in the systemd's fashion
    // https://systemd.io/CREDENTIALS/

    // clang-format off
    desc.add_options()
        (help_opt, "produce help message")
        (mysql_socket_opt, po::value<std::string>()->default_value("/tmp/mysql.sock"))
        (loglevel_opt, po::value<spdlog::level::level_enum>()->default_value(spdlog::level::info), loglevel_msg.c_str())
        (mysql_user_opt, po::value<std::string>()->default_value(get_os_username()))
        (mysql_password_opt, po::value<std::string>()->default_value(""))
        (mysql_ssl_opt, po::value<boost::mysql::ssl_mode>()->default_value(boost::mysql::ssl_mode::require), mysql_ssl_msg.c_str())
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

    auto &db = builder.add_service<service::db>(
        exec,
        std::move(mysql_user),
        std::move(mysql_password),
        std::move(mysql_socket),
        ssl
    );

    std::ignore = builder.add_service<service::user_dialog>(
        tg.get_connection(),
        tg.get_update(),
        db.get_database()
    );

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
        spdlog::set_level(args_[loglevel_opt].as<spdlog::level::level_enum>());
    }

    ssl_ctx_.set_default_verify_paths();

    inventory_ = make_inventory(
        io_.get_executor(),
        ssl_ctx_,
        make_token(args_),
        args_["mysql-user"].as<std::string>(),
        args_["mysql-password"].as<std::string>(),
        args_["mysql-socket"].as<std::string>(),
        args_["mysql-ssl"].as<boost::mysql::ssl_mode>()
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
