#pragma once

#include "lw/application/service_base.hpp"
#include "lw/telegram/connection.hpp"
#include "lw/telegram/update/polymorphic_update.hpp"

namespace lw::application::service {

class tg : public initializable_service_base
{
public:
    enum class update_method
    {
        long_polling,
    };

public:
    tg( //
        telegram::connection::executor_type ex,
        boost::asio::ssl::context &ssl_ctx,
        std::string bot_token,
        update_method method
    );

public:
    auto &get_connection() noexcept { return conn_; }
    auto &get_update() noexcept { return *update_; }

private: // initializable_service_base
    void async_init(init_handler_t callback) final;

private:
    telegram::connection conn_;
    std::unique_ptr<telegram::update::polymorphic_update> update_;
};

} // namespace lw::application::service
