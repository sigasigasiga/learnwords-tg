#pragma once

namespace lw::database::query {

inline constexpr std::string_view set_state = R"lw_cpp(
    UPDATE `settings`
    SET `state` = ?
    WHERE `tg_user_id` = ?
)lw_cpp";

} // namespace lw::database::query
