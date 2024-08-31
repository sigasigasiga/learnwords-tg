#include "lw/systemd/credentials_storage.hpp"

namespace lw::systemd {

const char *credential::begin() const noexcept
{
    return source_.begin();
}

const char *credential::end() const noexcept
{
    auto begin = source_.begin();
    auto ret = source_.end();

    for(; begin != ret && siga::util::is_space(*std::prev(ret)); --ret)
        ;

    return ret;
}

// -------------------------------------------------------------------------------------------------

std::optional<credential> credentials_storage::operator()(std::string_view key) const
{
    auto cred_path = path_ / key;
    if(!std::filesystem::is_regular_file(cred_path)) {
        return std::nullopt;
    }

    return boost::iostreams::mapped_file_source{cred_path.native()};
}

std::optional<credentials_storage> make_credentials_storage()
{
    auto dir = std::getenv("CREDENTIALS_DIRECTORY");
    if(!dir) {
        return std::nullopt;
    }

    std::filesystem::path p = dir;
    if(std::filesystem::is_directory(p)) {
        return credentials_storage{std::move(p)};
    } else {
        return std::nullopt;
    }
}

} // namespace lw::systemd
