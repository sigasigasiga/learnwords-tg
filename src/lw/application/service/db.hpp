#pragma once

#include "lw/application/service_base.hpp"
#include "lw/database/database.hpp"

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
    auto &get_database() noexcept { return database_; }

private: // initializable_service_base
    void async_init(init_handler_t callback) final;

private:
    connection conn_;
    boost::mysql::connect_params params_;
    database::database database_;
};

} // namespace lw::application::service
