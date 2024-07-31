#pragma once

#include "lw/telegram/connection.hpp"
#include "lw/telegram/process_response.hpp"
#include "lw/telegram/update/interface.hpp"
#include "lw/util/scoped.hpp"

namespace lw::telegram::update {

class polling : public interface,
                private util::scoped
{
public:
    class delegate
    {
    public:
        virtual ~delegate() = default;

    public:
        virtual void on_error(error) = 0;
    };

public:
    polling(
        delegate &deleg,
        connection &conn,
        std::string_view token,
        std::chrono::seconds interval
    );

private: // interface
    boost::signals2::connection connect(slot_t slot) final;

private:
    boost::signals2::signal<signature_t> signal_;
};

} // namespace lw::telegram::update
