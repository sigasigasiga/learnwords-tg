#pragma once

#include "lw/application/service_base.hpp"
#include "lw/database/database.hpp"
#include "lw/telegram/update/polymorphic_update.hpp"

namespace lw::application::service {

class user_dialog : public initializable_service_base
{
public:
    user_dialog(
        telegram::connection &tg,
        telegram::update::polymorphic_update &update,
        database::database &db
    );

private: // initializable_service_base
    void async_init(init_handler_t cb) final;

private:
    void on_update(std::exception_ptr ep, boost::json::object update);

private:
    telegram::connection &telegram_;
    telegram::update::polymorphic_update &update_;
    database::database &database_;
};

} // namespace lw::application::service
