#pragma once

#include "lw/application/service_base.hpp"
#include "lw/telegram/connection.hpp"
#include "lw/telegram/update/polymorphic_update.hpp"

namespace lw::application::service {

class telegram : public service_base
{
public:
    enum class update_method
    {
        long_polling,
    };

public:
    telegram(
        boost::asio::any_io_executor exec,
        boost::asio::ssl::context &ssl_ctx,
        std::string token,
        update_method um
    );

public:
    auto &connection() noexcept { return conn_; }
    auto &update() noexcept { return *update_; }

private: // service_base
    void reload() final;

private:
    std::string token_;
    lw::telegram::connection conn_;
    std::unique_ptr<lw::telegram::update::polymorphic_update> update_;
};

} // namespace lw::application::service
