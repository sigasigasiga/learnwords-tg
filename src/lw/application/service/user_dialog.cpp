#include "lw/application/service/user_dialog.hpp"

namespace lw::application::service {

user_dialog::user_dialog(telegram::connection &conn, telegram::update::polymorphic_update &update)
    : conn_{conn}
    , update_{update}
{
}

// initializable_service_base
void user_dialog::async_init(init_handler_t cb)
{
    update_.async_resume(std::bind_front(&user_dialog::on_update, this));
    boost::asio::post(conn_.get_executor(), std::bind_front(std::move(cb), nullptr));
}

void user_dialog::on_update(std::exception_ptr ep, boost::json::object update)
{
    if(ep) {
        // TODO: handle `telegram::exception`
        std::rethrow_exception(ep);
    }

    update_.async_resume(std::bind_front(&user_dialog::on_update, this));
}

} // namespace lw::application::service
