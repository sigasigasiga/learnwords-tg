#pragma once

namespace lw::telegram::proto {

struct error
{
    struct response_parameters
    {
        std::optional<std::int64_t> migrate_to_chat_id;
        std::optional<std::chrono::seconds> retry_after;
    };

    std::string description;
    std::intmax_t error_code;
    std::optional<response_parameters> parameters;
};

} // namespace lw::telegram::proto
