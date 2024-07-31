#pragma once

#include "lw/util/meta.hpp"

namespace lw::util::asio {

namespace detail {

template<typename Tuple>
Tuple filter_executors(Tuple tuple)
{
    return tuple;
}

template<typename Tuple, typename Arg, typename... Args>
auto filter_executors(Tuple tuple, Arg &&arg, Args &&...args)
{
    constexpr bool is_executor = boost::asio::execution::is_executor<std::decay_t<Arg>>::value;

    constexpr bool is_io_object = requires {
        { arg.get_executor() } -> conceptify<boost::asio::execution::is_executor>;
    };

    if constexpr(is_executor || is_io_object) {
        auto updated = std::tuple_cat(std::move(tuple), std::make_tuple(std::forward<Arg>(arg)));
        return filter_executors(std::move(updated), std::forward<Args>(args)...);
    } else {
        return filter_executors(std::move(tuple), std::forward<Args>(args)...);
    }
}

template<
    boost::asio::completion_signature... Signatures,
    typename Coro,
    typename Tuple,
    std::size_t... Is>
auto make_composed_from_tuple(Coro &&coro, Tuple tuple, std::index_sequence<Is...>)
{
    return boost::asio::experimental::co_composed<Signatures...>(
        std::forward<Coro>(coro),
        get<Is>(std::move(tuple))... // `std::move` doesn't actually move the tuple
    );
}

template<boost::asio::completion_signature... Signatures, typename Coro, typename... Args>
auto make_composed(Coro &&coro, Args &&...args)
{
    auto io_tuple = filter_executors(std::tuple<>{}, std::forward<Args>(args)...);
    return make_composed_from_tuple<Signatures...>(
        std::forward<Coro>(coro),
        std::move(io_tuple),
        std::make_index_sequence<std::tuple_size_v<decltype(io_tuple)>>{}
    );
}

} // namespace detail

template<
    boost::asio::completion_signature... Signatures,
    typename CompletionToken,
    typename Coro,
    typename... Args>
auto co_initiate(CompletionToken &&handler, Coro &&coro, Args &&...args)
{
    return boost::asio::async_initiate<CompletionToken, Signatures...>(
        detail::make_composed<Signatures...>(std::forward<Coro>(coro), std::forward<Args>(args)...),
        handler,
        std::forward<Args>(args)...
    );
}

} // namespace lw::util::asio
