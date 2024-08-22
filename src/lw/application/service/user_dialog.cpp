#include "lw/application/service/user_dialog.hpp"

#include "lw/util/json/functional.hpp"

namespace lw::application::service {

namespace {

boost::asio::awaitable<void>
process_update(telegram::connection &tg, database::database &db, boost::json::object update)
{
    auto message = util::json::if_contains(update, "message") //
                       .and_then(util::json::if_object);
    if(!message) {
        co_return;
    }

    auto text = util::json::if_contains(*message, "text") //
                    .and_then(util::json::if_string);

    if(!text) {
        co_return;
    }

    auto entities = util::json::if_contains(*message, "entities") //
                        .and_then(util::json::if_array);

    auto entity_type_extractor = //
        std::views::transform(util::json::if_object) | std::views::join |
        std::views::transform(util::json::if_contains_with("type")) | std::views::join |
        std::views::transform(util::json::if_string) | std::views::join;

    // TODO:
    siga::util::ignore(entities, entity_type_extractor);
}

} // namespace

user_dialog::user_dialog(
    telegram::connection &tg,
    telegram::update::polymorphic_update &update,
    database::database &db
)
    : telegram_{tg}
    , update_{update}
    , database_{db}
{
}

// initializable_service_base
void user_dialog::async_init(init_handler_t cb)
{
    update_.async_resume(std::bind_front(&user_dialog::on_update, this));
    boost::asio::post(telegram_.get_executor(), std::bind_front(std::move(cb), nullptr));
}

void user_dialog::on_update(std::exception_ptr ep, boost::json::object update)
{
    if(ep) {
        // TODO: handle `telegram::exception`
        std::rethrow_exception(ep);
    }

    boost::asio::co_spawn(
        telegram_.get_executor(),
        process_update(telegram_, database_, std::move(update)),
        boost::asio::detached
    );

    update_.async_resume(std::bind_front(&user_dialog::on_update, this));
}

} // namespace lw::application::service
