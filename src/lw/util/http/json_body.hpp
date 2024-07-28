#pragma once

namespace lw::util::http {

class json_body
{
public:
    using value_type = boost::json::value;

    class writer
    {
    public:
        using const_buffers_type = boost::asio::const_buffer;

    public:
        template<bool isRequest, class Fields>
        writer(const boost::beast::http::header<isRequest, Fields> &h, const value_type &body)
        {
            // The serializer holds a pointer to the value, so all we need to do is to reset it.
            serializer.reset(&body);
        }

    public:
        void init(boost::system::error_code &ec)
        {
            // The serializer always works, so no error can occur here.
            ec = {};
        }

        boost::optional<std::pair<const_buffers_type, bool>> get(boost::system::error_code &ec)
        {
            ec = {};
            // We serialize as much as we can with the buffer. Often that'll suffice
            const auto len = serializer.read(buffer, sizeof(buffer));
            return std::make_pair(
                boost::asio::const_buffer(len.data(), len.size()),
                !serializer.done()
            );
        }

    private:
        boost::json::serializer serializer;
        // half of the probable networking buffer, let's leave some space for headers
        char buffer[32'768];
    };

    class reader
    {
    public:
        template<bool isRequest, class Fields>
        reader(boost::beast::http::header<isRequest, Fields> &h, value_type &body) : body(body)
        {
        }

    public:
        void
        init(const boost::optional<std::uint64_t> &content_length, boost::system::error_code &ec)
        {
            // If we know the content-length, we can allocate a monotonic resource to increase the
            // parsing speed. We're using it rather then a static_resource, so a consumer can modify
            // the resulting value. It is also only assumption that the parsed json will be smaller
            // than the serialize one, it might not always be the case.
            if(content_length) {
                parser.reset(boost::json::make_shared_resource<boost::json::monotonic_resource>(
                    *content_length
                ));
            }
            ec = {};
        }

        template<class ConstBufferSequence>
        std::size_t put(const ConstBufferSequence &buffers, boost::system::error_code &ec)
        {
            ec = {};
            // The parser just uses the `ec` to indicate errors, so we don't need to do anything.
            return parser.write_some(static_cast<const char *>(buffers.data()), buffers.size(), ec);
        }

        void finish(boost::system::error_code &ec)
        {
            ec = {};
            // We check manually if the json is complete.
            if(parser.done()) {
                body = parser.release();
            } else {
                ec = boost::json::error::incomplete;
            }
        }

    private:
        boost::json::stream_parser parser;
        value_type &body;
    };
};

} // namespace lw::util::http
