#pragma once

namespace lw::telegram {

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

using result = std::expected<boost::json::value, error>;

result process_response(boost::json::value raw_response);

} // namespace lw::telegram
