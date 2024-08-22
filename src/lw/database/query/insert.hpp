#pragma once

namespace lw::database::query {

inline constexpr std::string_view add_sentence = R"lw_cpp(
    INSERT INTO sentences(sentence) VALUES(?)
)lw_cpp";

} // namespace lw::database::query
