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

auto make_options_description()
{
    constexpr auto raw_loglevels = magic_enum::enum_values<spdlog::level::level_enum>();
    static_assert(raw_loglevels.back() == spdlog::level::level_enum::n_levels);
    std::span loglevels{
        std::ranges::begin(raw_loglevels),
        std::ranges::prev(std::ranges::end(raw_loglevels))
    };

    static const auto loglevel_msg =
        fmt::format("available options: {}", fmt::join(loglevels, ", "));

    static const auto mysql_ssl_msg = fmt::format(
        "available options: {}",
        fmt::join(magic_enum::enum_values<boost::mysql::ssl_mode>(), ", ")
    );

    po::options_description desc{"learnwords-tg options"};

    // clang-format off
    desc.add_options()
        (help_opt, "produce help message")
        (loglevel_opt, po::value<spdlog::level::level_enum>()->default_value(spdlog::level::info), loglevel_msg.c_str())
        (mysql_ssl_opt, po::value<boost::mysql::ssl_mode>()->default_value(boost::mysql::ssl_mode::require), mysql_ssl_msg.c_str())
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

auto make_cred_or_throw()
{
    if(auto ret = util::make_credentials()) {
        return *std::move(ret);
    } else {
        throw error::exception{
            error::code::no_credentials,
            "`CREDENTIALS_DIRECTORY` environment variable is not set"
        };
    }
}

} // anonymous namespace

application::application(int argc, const char *argv[])
    : ssl_ctx_{boost::asio::ssl::context::tls_client}
    , desc_{make_options_description()}
    , args_{parse_cmd_line(desc_, argc, argv)}
    , creds_{make_cred_or_throw()}
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

    auto get_cred = [this](std::string_view cred) {
        if(auto ret = creds_(cred).transform(siga::util::construct<std::string>)) {
            return *std::move(ret);
        } else {
            throw error::exception{error::code::no_credentials, cred};
        }
    };

    inventory_ = make_inventory(
        io_.get_executor(),
        ssl_ctx_,
        get_cred("telegram-token"),
        creds_("mysql-user")
            .transform(siga::util::construct<std::string>)
            .value_or(siga::util::lazy_eval{get_os_username}),
        creds_("mysql-password") //
            .transform(siga::util::construct<std::string>)
            .value_or(""),
        get_cred("mysql-socket"),
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
