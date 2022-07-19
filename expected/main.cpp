#include <iostream>
#include <string_view>
#include <charconv>

#include "bad_expected_acess.h"
#include "expected.h"
#include "expected_void.h"
#include "unexpected.h"


template <typename T>
nonstd::expected<T, std::from_chars_result> StringToInteger(std::string_view str, int base = 10)
{
  T res;
  if (auto from_chars_res = std::from_chars(str.data(), str.data() + str.size(), res, base);
      from_chars_res.ec == std::errc{})
  {
    return res;
  }
  else
  {
    return nonstd::unexpected<std::from_chars_result>(from_chars_res);
  }
}

int main()
{

}

