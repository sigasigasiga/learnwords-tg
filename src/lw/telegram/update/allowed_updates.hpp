#pragma once

#include "lw/util/flag_set.hpp"

namespace lw::telegram::update {

enum class allowed_updates
{
    update_id = (1 << 0),
    message = (1 << 1),
    edited_message = (1 << 2),
    channel_post = (1 << 3),
    edited_channel_post = (1 << 4),
    business_connection = (1 << 5),
    business_message = (1 << 6),
    edited_business_message = (1 << 7),
    deleted_business_messages = (1 << 8),
    message_reaction = (1 << 9),
    message_reaction_count = (1 << 10),
    inline_query = (1 << 11),
    chosen_inline_result = (1 << 12),
    callback_query = (1 << 13),
    shipping_query = (1 << 14),
    pre_checkout_query = (1 << 15),
    poll = (1 << 16),
    poll_answer = (1 << 17),
    my_chat_member = (1 << 18),
    chat_member = (1 << 19),
    chat_join_request = (1 << 20),
    chat_boost = (1 << 21),
    removed_chat_boost = (1 << 22),
};

boost::json::array make_allowed_updates(util::flag_set<allowed_updates> allowed);

} // namespace lw::telegram::update
