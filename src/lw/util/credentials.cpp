#include "lw/util/credentials.hpp"

namespace lw::util {

namespace {

bool is_space(unsigned char c)
{
    return std::isspace(c);
}

} // namespace

auto credentials::operator()(std::string_view key) const //
    -> std::optional<value>
{
    auto cred_path = path_ / key;
    if(!std::filesystem::is_regular_file(cred_path)) {
        return std::nullopt;
    }

    return boost::iostreams::mapped_file_source{cred_path.native()};
}

const char *credentials::value::begin() const noexcept
{
    return source_.begin();
}

const char *credentials::value::end() const noexcept
{
    auto begin = source_.begin();
    auto ret = source_.end();

    for(; begin != ret && is_space(*std::prev(ret)); --ret)
        ;

    return ret;
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
