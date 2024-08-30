#include "lw/util/credentials.hpp"

namespace lw::util {

auto credentials::operator()(std::string_view key) const //
    -> std::optional<boost::iostreams::mapped_file_source>
{
    auto cred_path = path_ / key;
    if(!std::filesystem::is_regular_file(cred_path)) {
        return std::nullopt;
    }

    return boost::iostreams::mapped_file_source{cred_path.native()};
}

std::optional<credentials> make_credentials()
{
    auto dir = std::getenv("CREDENTIALS_DIRECTORY");
    if(!dir) {
        return std::nullopt;
    }

    std::filesystem::path p = dir;
    if(std::filesystem::is_directory(p)) {
        return credentials{std::move(p)};
    } else {
        return std::nullopt;
    }
}

} // namespace lw::util
