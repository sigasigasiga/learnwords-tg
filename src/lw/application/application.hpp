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
    void on_init(std::exception_ptr ep);

private:
    boost::asio::io_context io_;
    boost::asio::ssl::context ssl_ctx_;
    boost::program_options::options_description desc_;
    boost::program_options::variables_map args_;
    util::optional<inventory> inventory_;
};

} // namespace lw::application
