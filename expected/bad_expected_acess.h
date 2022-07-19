#pragma once

#include <iostream>

namespace nonstd
{
  template<typename E>
  class bad_expected_access;

  template<>
  class bad_expected_access<void> : public std::exception {
  protected:
    bad_expected_access() noexcept = default;
    bad_expected_access(const bad_expected_access&) = default;
    bad_expected_access(bad_expected_access&&) = default;

    bad_expected_access& operator=(const bad_expected_access&) = default;
    bad_expected_access& operator=(bad_expected_access&&) = default;

    ~bad_expected_access() {}

  public:
    const char* what() const noexcept override
    {
      return "Bad_expected_access!\n";
    }
  };

  template<class E>
  class bad_expected_access : public bad_expected_access<void> {
  private:
    E val;
  public:
    explicit bad_expected_access(E e)
    {
      new(&val) E(std::move(e));
    }

    const char* what() const noexcept override
    {
      return "Bad_expected_access <void>!\n";
    }
    E& error() & noexcept
    {
      return val;
    }
    const E& error() const& noexcept
    {
      return val;
    }
    E&& error() && noexcept
    {
      return std::move(val);
    }
    const E&& error() const&& noexcept
    {
      return std::move(val);
    }
  };
}
