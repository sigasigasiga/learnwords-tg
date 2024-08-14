#pragma once

#include "lw/application/service_base.hpp"

namespace lw::application::service {

class db : public initializable_service_base
{
public:
    using connection = boost::mysql::any_connection;

public:
    db( //
        connection::executor_type exec,
        std::string user,
        std::string password,
        std::string socket,
        boost::mysql::ssl_mode ssl
    );

public:
    auto &get_mysql() noexcept { return conn_; }

private: // initializable_service_base
    void init(init_handler_t callback) final;

private:
    connection conn_;
    boost::mysql::connect_params params_;
};

} // namespace lw::application::service
