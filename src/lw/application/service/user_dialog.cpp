#include "lw/application/service/user_dialog.hpp"

#include "lw/util/json/functional.hpp"

namespace lw::application::service {

namespace {

enum class state : std::uint8_t
{
    initial,
    set_remind_time,
    add_sentence,

    error = std::numeric_limits<std::underlying_type_t<state>>::max(),
};

boost::asio::awaitable<void> process_text( //
    telegram::connection &tg,
    database::database &db,
    std::uint64_t user_id,
    std::string_view text
)
{
    auto raw_state = co_await db.async_get_state(user_id, boost::asio::use_awaitable);

    switch(magic_enum::enum_cast<state>(raw_state).value_or(state::error)) {
        case state::initial: {
            break;
        }

        case state::set_remind_time: {
            break;
        }

        case state::add_sentence: {
            break;
        }

        case state::error: {
            break;
        }
    }
}

boost::asio::awaitable<void> process_command( //
    telegram::connection &tg,
    database::database &db,
    std::uint64_t user_id,
    std::string_view command
)
{
    auto raw_state = co_await db.async_get_state(user_id, boost::asio::use_awaitable);

    boost::json::object msg{
        {"chat_id", user_id}
    };

    auto &text = msg["text"].emplace_string();

    util::optional<state> new_state;

    switch(magic_enum::enum_cast<state>(raw_state).value_or(state::error)) {
        case state::initial: {
            text = "Greetings!";
            break;
        }

        case state::set_remind_time: {
            text = "TODO";
            break;
        }

        case state::add_sentence: {
            text = "TODO";
            break;
        }

        case state::error: {
            spdlog::error(
                "Got an erroneous state `{}` for user `{}` from the database",
                raw_state,
                user_id
            );

            new_state = state::initial;
            text = "Got an internal error, trying to recover... Please, contact the developer";

            break;
        }
    }

    assert(!text.empty());

    if(new_state) {
        co_await //
            db.async_set_state(user_id, std::to_underlying(*new_state), boost::asio::use_awaitable);
    }

    co_await tg.async_request("sendMessage", std::move(msg), boost::asio::use_awaitable);
}

boost::asio::awaitable<void> process_update( //
    telegram::connection &tg,
    database::database &db,
    boost::json::object update
)
{
    auto message = util::json::if_contains(update, "message") //
                       .and_then(util::json::if_object);
    if(!message) {
        co_return;
    }

    auto text = util::json::if_contains(*message, "text") //
                    .and_then(util::json::if_string);

    auto user_id = util::json::if_contains(*message, "from")
                       .and_then(util::json::if_object)
                       .and_then(util::json::if_contains_with("id"))
                       .and_then(util::json::if_int64);

    if(!text || !user_id) {
        co_return;
    }

    auto raw_entities = util::json::if_contains(*message, "entities") //
                            .and_then(util::json::if_array);

    // clang-format off
    auto entities = raw_entities
        | std::views::join
        | std::views::transform(util::json::if_object)
        | std::views::join
    ;
    // clang-format on

    auto command_it = std::ranges::find(entities, "bot_command", [](const auto &entity) {
        return util::json::if_contains(entity, "type") //
            .and_then(util::json::if_string);
    });

    if(command_it == entities.end()) {
        co_await process_text(tg, db, *user_id, *text);
    } else {
        const auto offset = util::json::if_contains(*command_it, "offset") //
                                .and_then(util::json::if_int64)
                                .value();
        const auto length = util::json::if_contains(*command_it, "length") //
                                .and_then(util::json::if_int64)
                                .value();

        co_await process_command(tg, db, *user_id, text->subview(offset, length));
    }
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
        // TODO: handle `telegram::exception`?
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
