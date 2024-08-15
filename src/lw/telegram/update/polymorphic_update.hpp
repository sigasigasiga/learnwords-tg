#pragma once

#include "lw/telegram/connection.hpp"
#include "lw/telegram/update/allowed_updates.hpp"
#include "lw/util/asio/coroutine.hpp"

namespace lw::telegram::update {

class polymorphic_update : private siga::util::scoped
{
public:
    using completion_signature = void(std::exception_ptr, boost::json::object);

    // TODO: usage of `any_completion_handler` leads to a compiler error, despite the fact
    // that it's the best option for the task. Should write a bug report probably?
#if 0
    using completion_token = boost::asio::any_completion_handler<completion_signature>;
#else
    using completion_token = std::function<completion_signature>;
#endif

public:
    virtual ~polymorphic_update() = default;

public:
    virtual void async_resume(completion_token token) = 0;
};

// -------------------------------------------------------------------------------------------------

class polymorphic_long_polling : public polymorphic_update
{
public:
    polymorphic_long_polling(
        connection &conn,
        std::chrono::seconds timeout,
        int limit = 100,
        siga::util::flag_set<allowed_updates> allowed = {}
    );

public: // polymorphic_update
    void async_resume(completion_token token) final;

private:
    util::asio::coroutine<boost::json::object> coro_;
};

} // namespace lw::telegram::update
