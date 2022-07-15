#pragma once

namespace nonstd
{
  template<typename E>
  class unexpected
  {
  private:
    E val;
  public:
    // Constructors
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;

    template<class Err = E>
    requires !std::is_same_v<remove_cvref_t<Err>, unexpected>
          && !std::is_same_v<remove_cvref_t<Err>, std::in_place_t>
          && std::is_constructible_v<E, Err>
    constexpr explicit unexpected(Err&& e)
    {
      new (&val) E(std::forward<Err>(e));
    }

    template<class... Args>
    requires std::is_constructible_v<E, Args...>
    constexpr explicit unexpected(std::in_place_t, Args&&...args)
    {
      new (&val) E(std::forward<Args>(args)...);
    }

    template<class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il, Args&&...args)
    {
      new (&val) E(il, std::forward<Args>(args)...);
    }

    // Observers
    constexpr unexpected& operator=(const unexpected&) = default;
    constexpr unexpected& operator=(unexpected&&) = default;

    constexpr const E& value() const& noexcept
    {
      return val;
    }
    constexpr E& value() & noexcept
    {
      return val;
    }
    constexpr const E&& value() const&& noexcept
    {
      return std::move(val);
    }
    constexpr E&& value() && noexcept
    {
      return std::move(val);
    }

    // Swap
    template <typename E>
    requires std::is_swappable_v<E>
    constexpr void swap(unexpected& other) noexcept(std::is_nothrow_swappable_v<E>)
    {
      using std::swap;
      return swap(val, other.val);
    }

    // Equality operators
    template<class E2> // ??? The expression x.value()== y.value() is well-formed and its result is convertible to bool
    friend constexpr bool operator==(const unexpected& x, const unexpected<E2>& y)
    {
      return x.value() == y.value();
    }
  };

  template <typename E>
  constexpr void swap(unexpected<E>& x, unexpected<E>& y) noexcept(noexcept(x.swap(y)))
  {
    x.swap(y);
  }
  template<class E> unexpected(E)->unexpected<E>;
}