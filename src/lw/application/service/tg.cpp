#include "lw/application/service/tg.hpp"

namespace lw::application::service {

tg::tg(
    telegram::connection::executor_type ex,
    boost::asio::ssl::context &ssl_ctx,
    std::string bot_token,
    update_method /* method */
)
    : conn_{std::move(ex), ssl_ctx, std::move(bot_token)}
    , update_{std::make_unique<telegram::update::polymorphic_long_polling>(
          conn_,
          std::chrono::seconds(100)
      )}
{
    assert(!conn_.get_token().empty());
    assert(conn_.get_token().back() != '\n');
}

// initializable_service_base
void tg::init(init_handler_t callback)
{
    boost::asio::co_spawn(
        conn_.get_executor(),
        conn_.async_connect(boost::asio::use_awaitable),
        std::move(callback)
    );
}

} // namespace lw::application::service
