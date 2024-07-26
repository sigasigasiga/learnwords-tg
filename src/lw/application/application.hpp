#pragma once

#include <boost/program_options.hpp>

#include "lw/application/inventory.hpp"
#include "lw/error/code.hpp"

namespace lw::application {

class application
{
public:
    application(int argc, const char *argv[]);

public:
    error::code run();

private:
    boost::asio::awaitable<void> async_init();

private:
    boost::asio::io_context io_;
    boost::program_options::options_description desc_;
    boost::program_options::variables_map args_;
    boost::mysql::any_connection mysql_;
    std::optional<inventory> inventory_;
};

} // namespace lw::application
