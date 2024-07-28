#pragma once

#include "lw/util/http/json_body.hpp"

namespace lw::telegram {

class connection
{
public:
    connection(boost::asio::any_io_executor ex, boost::asio::ssl::context &ssl_ctx);

public:
    using connect_signature_t = void(boost::system::error_code);
    using request_signature_t = void(boost::system::error_code, boost::json::value);

public:
    template<typename CompletionToken>
    requires boost::asio::completion_token_for<CompletionToken, connect_signature_t>
    auto async_connect(CompletionToken &&handler);

    // TODO: add params
    template<typename CompletionToken>
    requires boost::asio::completion_token_for<CompletionToken, request_signature_t>
    auto async_request(std::string_view token, std::string_view method, CompletionToken &&handler);

private:
    using stream_t = boost::beast::ssl_stream<boost::beast::tcp_stream>;

private:
    stream_t stream_;
};

// -------------------------------------------------------------------------------------------------

inline connection::connection(boost::asio::any_io_executor ex, boost::asio::ssl::context &ssl_ctx)
    : stream_{std::move(ex), ssl_ctx}
{
}

template<typename CompletionToken>
requires boost::asio::completion_token_for<CompletionToken, connection::connect_signature_t>
auto connection::async_connect(CompletionToken &&handler)
{
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

    return boost::asio::async_initiate<CompletionToken, connect_signature_t>(
        boost::asio::experimental::co_composed<connect_signature_t>(std::move(impl), stream_),
        handler,
        std::ref(stream_)
    );
}

template<typename CompletionToken>
requires boost::asio::completion_token_for<CompletionToken, connection::request_signature_t>
auto connection::async_request(
    std::string_view token,
    std::string_view method,
    CompletionToken &&handler
)
{
    auto impl = [](auto state, stream_t &stream, std::string_view token, std::string_view method
                ) -> void {
        boost::system::error_code ec;
        boost::json::value json;

        try {
            const auto path = fmt::format("/bot{}/{}", token, method);

            // TODO: the body shouldn't be empty?
            boost::beast::http::request<boost::beast::http::empty_body> req;
            req.method(boost::beast::http::verb::get);
            req.target(path);
            req.set(boost::beast::http::field::host, "api.telegram.org");
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

            json = std::move(response).body();
        } catch(const boost::system::system_error &err) {
            ec = err.code();
        }

        co_return {ec, json};
    };

    return boost::asio::async_initiate<CompletionToken, request_signature_t>(
        boost::asio::experimental::co_composed<request_signature_t>(std::move(impl), stream_),
        handler,
        std::ref(stream_),
        token,
        method
    );
}

} // namespace lw::telegram
