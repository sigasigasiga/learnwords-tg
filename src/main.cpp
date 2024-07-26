#include "lw/application/application.hpp"

int main(int argc, const char *argv[])
{
    lw::application::application app{argc, argv};
    return static_cast<int>(app.run());
}
