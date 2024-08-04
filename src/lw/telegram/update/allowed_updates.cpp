#include "lw/telegram/update/allowed_updates.hpp"

namespace lw::telegram::update {

boost::json::array make_allowed_updates(util::flag_set<allowed_updates> allowed)
{
    using enum allowed_updates;

    boost::json::array ret;

    // clang-format off
    if(allowed & update_id) ret.push_back("update_id");
    if(allowed & message) ret.push_back("message");
    if(allowed & edited_message) ret.push_back("edited_message");
    if(allowed & channel_post) ret.push_back("channel_post");
    if(allowed & edited_channel_post) ret.push_back("edited_channel_post");
    if(allowed & business_connection) ret.push_back("business_connection");
    if(allowed & business_message) ret.push_back("business_message");
    if(allowed & edited_business_message) ret.push_back("edited_business_message");
    if(allowed & deleted_business_messages) ret.push_back("deleted_business_messages");
    if(allowed & message_reaction) ret.push_back("message_reaction");
    if(allowed & message_reaction_count) ret.push_back("message_reaction_count");
    if(allowed & inline_query) ret.push_back("inline_query");
    if(allowed & chosen_inline_result) ret.push_back("chosen_inline_result");
    if(allowed & callback_query) ret.push_back("callback_query");
    if(allowed & shipping_query) ret.push_back("shipping_query");
    if(allowed & pre_checkout_query) ret.push_back("pre_checkout_query");
    if(allowed & poll) ret.push_back("poll");
    if(allowed & poll_answer) ret.push_back("poll_answer");
    if(allowed & my_chat_member) ret.push_back("my_chat_member");
    if(allowed & chat_member) ret.push_back("chat_member");
    if(allowed & chat_join_request) ret.push_back("chat_join_request");
    if(allowed & chat_boost) ret.push_back("chat_boost");
    if(allowed & removed_chat_boost) ret.push_back("removed_chat_boost");
    // clang-format on

    return ret;
}

} // namespace lw::telegram::update
