#pragma once

namespace lw::util::asio {

template<typename T>
using coroutine = boost::asio::experimental::coro<T, T>;

} // namespace lw::util::asio
