#pragma once

#include <boost/iostreams/device/mapped_file.hpp>

namespace lw::systemd {

class credential
{
public:
    credential(boost::iostreams::mapped_file_source source)
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

// -------------------------------------------------------------------------------------------------

// https://systemd.io/CREDENTIALS/
class credentials_storage
{
public:
    credentials_storage(std::filesystem::path path)
        : path_{std::move(path)}
    {
    }

public:
    std::optional<credential> operator()(std::string_view key) const;

private:
    std::filesystem::path path_;
};

std::optional<credentials_storage> make_credentials_storage();

} // namespace lw::systemd
