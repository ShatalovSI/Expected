#pragma once

#include "unexpected.h"
#include <iostream>

namespace nonstd
{
  struct unexpect_t {
    explicit unexpect_t() = default;
  };
  inline constexpr unexpect_t unexpect{};

  template <typename T>
  struct is_unexpected : std::false_type
  {
  };

  template <typename E>
  struct is_unexpected<unexpected<E>> : std::true_type
  {
  };

  template <typename T>
  inline constexpr bool is_unexpected_v = is_unexpected<T>::value;
}