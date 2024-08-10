#pragma once

#include "lw/telegram/proto/error.hpp"

namespace lw::telegram {

using processed_result = std::expected<boost::json::value, proto::error>;

processed_result process_response(boost::json::value raw_response);

} // namespace lw::telegram
