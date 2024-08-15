#include "lw/telegram/update/polymorphic_update.hpp"

#include "lw/telegram/update/long_polling.hpp"

namespace lw::telegram::update {

polymorphic_long_polling::polymorphic_long_polling(
    connection &conn,
    std::chrono::seconds timeout,
    int limit,
    siga::util::flag_set<allowed_updates> allowed
)
    : coro_{long_polling(conn, timeout, limit, allowed)}
{
}

// polymorphic_update
void polymorphic_long_polling::async_resume(completion_token token)
{
    return coro_.async_resume(std::move(token));
}

} // namespace lw::telegram::update
