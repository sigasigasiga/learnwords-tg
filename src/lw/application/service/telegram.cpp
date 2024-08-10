#include "lw/application/service/telegram.hpp"

namespace lw::application::service {

namespace {

std::unique_ptr<lw::telegram::update::polymorphic_update>
make_update(telegram::update_method um, lw::telegram::connection &conn)
{
    switch(um) {
        case telegram::update_method::long_polling: {
            return std::make_unique<lw::telegram::update::polymorphic_long_polling>(
                conn,
                std::chrono::seconds{25}
            );
        }
    }

    assert(false);
    std::unreachable();
}

} // namespace

telegram::telegram(
    boost::asio::any_io_executor exec,
    boost::asio::ssl::context &ssl_ctx,
    std::string token,
    update_method um
)
    : token_{std::move(token)}
    , conn_{exec, ssl_ctx}
    , update_{make_update(um, conn_)}
{
}

// service_base
void telegram::reload()
{
    conn_.async_connect(token_, boost::asio::detached);
}

} // namespace lw::application::service
