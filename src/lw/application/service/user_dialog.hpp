#pragma once

#include "lw/application/service_base.hpp"
#include "lw/telegram/update/polymorphic_update.hpp"
#include "lw/util/first_flag.hpp"

namespace lw::application::service {

class user_dialog : public service_base
{
public:
    enum class update_method
    {
        long_polling,
    };

public:
    user_dialog(telegram::connection &conn, update_method method);

private: // service_base
    void reload() final;

private:
    void on_update(std::exception_ptr ep, boost::json::object update);

private:
    util::first_flag first_;
    std::unique_ptr<lw::telegram::update::polymorphic_update> update_;
};

} // namespace lw::application::service
