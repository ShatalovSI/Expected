#pragma once
#include <concepts>

namespace nonstd
{
  struct unexpect_t
  {
    explicit unexpect_t() = default;
  };

  inline constexpr unexpect_t unexpect {};

  template <typename E>
  class unexpected;

  template <typename T, typename E>
  class expected;

  namespace traits
  {
    template <typename E>
    struct is_unexpected : std::false_type
    {
    };

    template <typename E>
    struct is_unexpected<unexpected<E>> : std::true_type
    {
    };

    template <typename E>
    struct is_unexpected<const unexpected<E>> : std::true_type
    {
    };

    template <typename E>
    struct is_unexpected<volatile unexpected<E>> : std::true_type
    {
    };

    template <typename E>
    struct is_unexpected<const volatile unexpected<E>> : std::true_type
    {
    };

    template <typename E>
    inline constexpr bool is_unexpected_v = is_unexpected<E>::value;



    template <typename T>
    struct is_expected : std::false_type
    {
    };

    template <typename T, typename E>
    struct is_expected<expected<T, E>> : std::true_type
    {
    };

    template <typename T, typename E>
    struct is_expected<const expected<T, E>> : std::true_type
    {
    };

    template <typename T, typename E>
    struct is_expected<volatile expected<T, E>> : std::true_type
    {
    };

    template <typename T, typename E>
    struct is_expected<const volatile expected<T, E>> : std::true_type
    {
    };

    template <typename T, typename E>
    inline constexpr bool is_expected_v = is_expected<T >> ::value;
  }

  template <typename E>
  class unexpected
  {
  private:
    E val;
  public:
    // Constructors
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;

    template<class Err = E>
      requires !traits::is_unexpected_v<Err>
            && !std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t>
            && std::is_constructible_v<E, Err>
    constexpr explicit unexpected(Err &&e) : val { std::forward<Err>(e) }
    {
    }

    template <class... Args>
      requires std::is_constructible_v<E, Args...>
    constexpr explicit unexpected(std::in_place_t, Args&&...args) : val { std::forward<Args>(args)... }
    {
    }

    template<class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il, Args&&...args)
      : val { il, std::forward<Args>(args)... }
    {
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
    template<class E2> 
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

  template<class E>
  unexpected(E) -> unexpected<E>;
}

//transform, and_