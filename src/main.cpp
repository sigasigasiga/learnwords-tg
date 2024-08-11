#include "lw/application/application.hpp"
#include "lw/error/code.hpp"

int main(int argc, const char *argv[])
try {
    lw::application::application app{argc, argv};
    return std::to_underlying(app.run());
} catch(const lw::error::exception &ex) {
    spdlog::critical("{}", ex.what());
    return std::to_underlying(ex.code());
} catch(const std::exception &ex) {
    spdlog::critical("{}", ex.what());
    return std::to_underlying(lw::error::code::caught_exception);
} catch(...) {
    spdlog::critical("Unknown exception");
    return std::to_underlying(lw::error::code::caught_exception);
}
