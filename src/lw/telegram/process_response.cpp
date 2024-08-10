#include "lw/telegram/process_response.hpp"

namespace lw::telegram {

processed_result process_response(boost::json::value raw_response)
{
    const auto response = std::move(raw_response).as_object();

    if(response.at("ok").as_bool()) {
        return std::move(response).at("result");
    } else {
        proto::error ret;
        ret.description = response.at("description").as_string();
        ret.error_code = response.at("error_code").as_int64();
        if(const auto *raw_params = response.if_contains("parameters")) {
            proto::error::response_parameters parameters;

            if(const auto *raw_migrate = response.if_contains("migrate_to_chat_id")) {
                parameters.migrate_to_chat_id = raw_migrate->as_int64();
            }

            if(const auto *raw_retry = response.if_contains("retry_after")) {
                parameters.retry_after = std::chrono::seconds{raw_retry->as_int64()};
            }

            ret.parameters = std::move(parameters);
        }

        return std::unexpected(std::move(ret));
    }
}

} // namespace lw::telegram
