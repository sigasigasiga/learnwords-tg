#pragma once

#include <boost/iostreams/device/mapped_file.hpp>

namespace lw::util {

// https://systemd.io/CREDENTIALS/
class credentials
{
public:
    credentials(std::filesystem::path path)
        : path_{std::move(path)}
    {
    }

public:
    class value
    {
    public:
        value(boost::iostreams::mapped_file_source source)
            : source_{std::move(source)}
        {
        }

    public:
        const char *begin() const noexcept;
        const char *end() const noexcept;

        operator std::string_view() const & { return std::string_view{begin(), end()}; }
        operator std::string_view() && = delete;
        operator std::string() const { return std::string{begin(), end()}; }

    private:
        boost::iostreams::mapped_file_source source_;
    };

public:
    std::optional<value> operator()(std::string_view key) const;

private:
    std::filesystem::path path_;
};

std::optional<credentials> make_credentials();

} // namespace lw::util
