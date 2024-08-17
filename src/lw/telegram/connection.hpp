#pragma once

#include "lw/telegram/process_response.hpp"
#include "lw/util/asio/co_initiate.hpp"
#include "lw/util/http/json_body.hpp"

namespace lw::telegram {

class connection
{
public:
    using connect_signature_t = void(boost::system::error_code);
    using request_signature_t = void(boost::system::error_code, processed_result);
    using stream_t = boost::beast::ssl_stream<boost::beast::tcp_stream>;

public:
    using executor_type = stream_t::executor_type;

public:
    connection(executor_type ex, boost::asio::ssl::context &ssl_ctx, std::string bot_token);

public:
    template<typename CompletionToken>
    requires boost::asio::completion_token_for<CompletionToken, connect_signature_t>
    auto async_connect(CompletionToken &&handler);

    // clang-format off
    template<typename CompletionToken>
    requires boost::asio::completion_token_for<CompletionToken, request_signature_t>
    auto async_request(
        std::string_view method,
        boost::json::object json,
        CompletionToken &&handler
    );
    // clang-format on

    decltype(auto) get_executor();
    const auto &get_token() const noexcept;

private:
    stream_t stream_;
    std::string bot_token_;
};

// -------------------------------------------------------------------------------------------------

inline connection::connection(
    executor_type ex,
    boost::asio::ssl::context &ssl_ctx,
    std::string bot_token
)
    : stream_{std::move(ex), ssl_ctx}
    , bot_token_{std::move(bot_token)}
{
}

template<typename CompletionToken>
requires boost::asio::completion_token_for<CompletionToken, connection::connect_signature_t>
auto connection::async_connect(CompletionToken &&handler)
{
    assert(!bot_token_.empty());

    auto impl = [](auto state, stream_t &stream) -> void {
        boost::system::error_code ret;

        try {
            boost::asio::ip::tcp::resolver resolver{stream.get_executor()};
            const auto endpoints =
                co_await resolver.async_resolve("api.telegram.org", "443", boost::asio::deferred);

            auto &lowest = boost::beast::get_lowest_layer(stream);
            std::ignore = co_await lowest.async_connect(endpoints, boost::asio::deferred);

            co_await stream.async_handshake(
                boost::asio::ssl::stream_base::client,
                boost::asio::deferred
            );
        } catch(const boost::system::system_error &err) {
            ret = err.code();
        }

        co_return {ret};
    };

    return util::asio::co_initiate<connect_signature_t>(
        std::forward<CompletionToken>(handler),
        std::move(impl),
        std::ref(stream_)
    );
}

template<typename CompletionToken>
requires boost::asio::completion_token_for<CompletionToken, connection::request_signature_t>
auto connection::async_request(
    std::string_view method,
    boost::json::object json,
    CompletionToken &&handler
)
{
    assert(!bot_token_.empty());

    auto impl = [](auto state,
                   stream_t &stream,
                   std::string_view token,
                   std::string_view method,
                   boost::json::object json) -> void {
        boost::system::error_code ec;
        processed_result ret;

        try {
            const auto path = fmt::format("/bot{}/{}", token, method);

            spdlog::trace("lw::telegram::connection --> /{}, {}", method, fmt::streamed(json));

            boost::beast::http::request<util::http::json_body> req;
            req.method(boost::beast::http::verb::post);
            req.target(path);
            req.set(boost::beast::http::field::host, "api.telegram.org");
            req.set(boost::beast::http::field::content_type, "application/json");
            req.body() = std::move(json);
            req.prepare_payload();

            co_await boost::beast::http::async_write(stream, req, boost::asio::deferred);

            boost::beast::flat_buffer buffer;
            boost::beast::http::response<util::http::json_body> response;
            co_await boost::beast::http::async_read(
                stream,
                buffer,
                response,
                boost::asio::deferred
            );

            spdlog::trace(
                "lw::telegram::connection <-- /{}, {}",
                method,
                fmt::streamed(response.body())
            );

            ret = process_response(std::move(response).body());
        } catch(const boost::system::system_error &err) {
            ec = err.code();
        }

        co_return {ec, std::move(ret)};
    };

    return util::asio::co_initiate<request_signature_t>(
        std::forward<CompletionToken>(handler),
        std::move(impl),
        std::ref(stream_),
        bot_token_,
        method,
        std::move(json)
    );
}

inline decltype(auto) connection::get_executor()
{
    return stream_.get_executor();
}

inline const auto &connection::get_token() const noexcept
{
    return bot_token_;
}

} // namespace lw::telegram
