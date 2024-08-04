#include "lw/telegram/update/long_polling.hpp"

namespace lw::telegram::update {

// interface
boost::signals2::connection long_polling::connect(slot_t slot)
{
    return signal_.connect(std::move(slot));
}

} // namespace lw::telegram::update
