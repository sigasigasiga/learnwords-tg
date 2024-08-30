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
    std::optional<boost::iostreams::mapped_file_source> operator()(std::string_view key) const;

private:
    std::filesystem::path path_;
};

std::optional<credentials> make_credentials();

} // namespace lw::util
