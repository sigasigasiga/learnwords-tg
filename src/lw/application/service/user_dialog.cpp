#include "lw/application/service/user_dialog.hpp"

namespace lw::application::service {

namespace {

auto make_update(telegram::connection &conn, user_dialog::update_method method)
{
    switch(method) {
        case user_dialog::update_method::long_polling:
            return std::make_unique<telegram::update::polymorphic_long_polling>(
                conn,
                std::chrono::seconds{25}
            );
    }

    std::unreachable();
}

} // namespace

user_dialog::user_dialog(telegram::connection &conn, update_method method)
    : update_{make_update(conn, method)}
{
}

// service_base
void user_dialog::reload()
{
    if(first_.get()) {
        // TODO: wrap the callback
        update_->async_resume(std::bind_front(&user_dialog::on_update, this));
    }
}

void user_dialog::on_update(std::exception_ptr ep, boost::json::object update)
{
    if(ep) {
        // TODO: handle `telegram::exception`
        std::rethrow_exception(ep);
    }

    update_->async_resume(std::bind_front(&user_dialog::on_update, this));
}

} // namespace lw::application::service
