#include "lw/telegram/process_response.hpp"

#include "lw/util/json/functional.hpp"

namespace lw::telegram {

namespace {

proto::error::response_parameters make_params(const boost::json::object &obj)
{
    proto::error::response_parameters parameters;

    auto migrate = util::json::if_contains(obj, "migrate_to_chat_id") //
                       .and_then(util::json::if_int64);
    if(migrate) {
        parameters.migrate_to_chat_id = *migrate;
    }

    auto retry = util::json::if_contains(obj, "retry_after") //
                     .and_then(util::json::if_int64)
                     .transform(siga::util::construct<std::chrono::seconds>);
    if(retry) {
        parameters.retry_after = *retry;
    }

    return parameters;
}

} // anonymous namespace

processed_result process_response(boost::json::value raw_response)
{
    auto response = std::move(raw_response).as_object();

    if(response.at("ok").as_bool()) {
        return std::move(response).at("result");
    } else {
        proto::error ret;
        ret.description = response.at("description").as_string();
        ret.error_code = response.at("error_code").as_int64();

        auto p = util::json::if_contains(response, "parameters") //
                     .and_then(util::json::if_object)
                     .transform(make_params);

        if(p) {
            ret.parameters = *std::move(p);
        }

        return std::unexpected{std::move(ret)};
    }
}

} // namespace lw::telegram
