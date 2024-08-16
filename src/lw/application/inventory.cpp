#include "lw/application/inventory.hpp"

namespace lw::application {

namespace {

class initializer : public siga::util::shared_from_this_base,
                    private siga::util::scoped
{
public:
    initializer(
        sftb_tag tag,
        boost::asio::any_io_executor exec,
        inventory::service_list_t &services,
        inventory::init_handler_t callback
    )
        : shared_from_this_base{tag}
        , exec_{std::move(exec)}
        , begin_{services.begin()}
        , end_{services.end()}
        , callback_{std::move(callback)}
    {
    }

public:
    void start(std::exception_ptr ep = nullptr)
    {
        if(ep) {
            return post(std::move(ep));
        }

        begin_ = std::ranges::find_if(
            begin_,
            end_,
            siga::util::dynamic_value_cast<initializable_service_base *>,
            &inventory::service_ptr_t::get
        );

        if(begin_ == end_) {
            spdlog::info("The inventory was initialized");
            post(nullptr);
        } else {
            auto &svc = dynamic_cast<initializable_service_base &>(**begin_);
            svc.init(std::bind_front(&initializer::start, shared_from_this()));

            ++begin_;
        }
    }

private:
    void post(std::exception_ptr ep)
    {
        boost::asio::post(exec_, std::bind_front(std::move(callback_), std::move(ep)));
    }

private:
    boost::asio::any_io_executor exec_;
    inventory::service_list_t::iterator begin_;
    inventory::service_list_t::iterator end_;
    inventory::init_handler_t callback_;
};

// TODO: `initializer` and `stopper` both look very similar.
// Is there a way to make them more generic?
class stopper : public siga::util::shared_from_this_base,
                private siga::util::scoped
{
public:
    stopper(
        sftb_tag tag,
        boost::asio::any_io_executor exec,
        inventory::service_list_t &services,
        inventory::stop_handler_t callback
    )
        : shared_from_this_base{tag}
        , exec_{std::move(exec)}
        , begin_{services.rbegin()}
        , end_{services.rend()}
        , callback_{std::move(callback)}
    {
    }

public:
    void start()
    {
        begin_ = std::ranges::find_if(
            begin_,
            end_,
            siga::util::dynamic_value_cast<stoppable_service_base *>,
            &inventory::service_ptr_t::get
        );

        if(begin_ == end_) {
            spdlog::info("The inventory was stopped");
            return boost::asio::post(exec_, std::move(callback_));
        } else {
            auto &svc = dynamic_cast<stoppable_service_base &>(**begin_);
            svc.stop(std::bind_front(&stopper::start, shared_from_this()));

            ++begin_;
        }
    }

private:
    boost::asio::any_io_executor exec_;
    inventory::service_list_t::reverse_iterator begin_;
    inventory::service_list_t::reverse_iterator end_;
    inventory::stop_handler_t callback_;
};

} // namespace

inventory::inventory(boost::asio::any_io_executor exec, service_list_t services)
    : exec_{std::move(exec)}
    , services_{std::move(services)}
    , state_{state::init}
{
    assert(not services_.empty());
}

void inventory::init(init_handler_t callback)
{
    assert(state_ == state::init);
    // TODO: change the state after the `init` is done (or should we just throw it away?)
    siga::util::make_shared<initializer>(exec_, services_, std::move(callback))->start();
}

void inventory::reload()
{
    // clang-format off
    auto reloadables = services_
        | std::views::transform(&service_ptr_t::get)
        | std::views::transform(siga::util::dynamic_value_cast<reloadable_service_base *>)
        | std::views::filter(siga::util::not_equal_to(nullptr));
    // clang-format on

    std::ranges::for_each(reloadables, &reloadable_service_base::reload);

    spdlog::info("The inventory was reloaded");
}

void inventory::stop(stop_handler_t callback)
{
    siga::util::make_shared<stopper>(exec_, services_, std::move(callback))->start();
}

} // namespace lw::application
