#pragma once

namespace lw::database::query {

inline constexpr std::string_view get_state = R"lw_cpp(
    SELECT `state`
    FROM `settings`
    WHERE `tg_user_id` = ?
)lw_cpp";

} // namespace lw::database::query
