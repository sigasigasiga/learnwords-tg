#pragma once

namespace lw::telegram::proto {

struct error
{
    struct response_parameters
    {
        util::optional<std::int64_t> migrate_to_chat_id;
        util::optional<std::chrono::seconds> retry_after;
    };

    std::string description;
    std::intmax_t error_code;
    util::optional<response_parameters> parameters;
};

} // namespace lw::telegram::proto
