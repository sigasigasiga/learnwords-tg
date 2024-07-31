#pragma once

namespace lw::util {

template<typename T, template<typename...> typename Trait>
concept conceptify = Trait<T>::value;

} // namespace lw::util
