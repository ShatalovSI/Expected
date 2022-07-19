#pragma once
#include <iostream>
#include "expected.h"

namespace nonstd
{
  template <class T, class E> requires std::is_void_v<T> 
                                    && std::is_destructible_v<E>
  class expected<T, E>
  {
  private:
    bool has_val;
    union
    {
      E unex;
    };

  public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    template<class U>
    using rebind = expected<U, error_type>;

    // Constructors
    constexpr expected() noexcept
    {
      has_val = true;
    }
    constexpr explicit(" ") expected(const expected& rhs) : has_val(rhs.has_val)
    {
      if (!has_val)
        new(&unex) E(rhs.error());
    }

    template <typename E>
    requires std::is_move_constructible_v<E>
    constexpr explicit(" ") expected(expected&& rhs) noexcept(" ") : has_val(rhs.has_val)
    {
      if (!has_val)
        new(&unex) E(std::move(rhs.error()));
    }

    template<class U, class G>
    requires std::is_void_v<U>
          && std::is_constructible_v<E, const G&>
          && !std::is_constructible_v<unexpected<E>, expected<U, G>&>
          && !std::is_constructible_v<unexpected<E>, expected<U, G>>
          && !std::is_constructible_v<unexpected<E>, const expected<U, G>&>
          && !std::is_constructible_v<unexpected<E>, const expected<U, G>>
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const expected<U, G>& rhs) : has_val(rhs.has_val)
    {
      if (!has_val)
        new (&unex) const U& (std::forward<const U&>(*rhs.error()));
    }

    template<class U, class G>
    requires std::is_void_v<U>
          && std::is_constructible_v<E, G>
          && !std::is_constructible_v<unexpected<E>, expected<U, G>&>
          && !std::is_constructible_v<unexpected<E>, expected<U, G>>
          && !std::is_constructible_v<unexpected<E>, const expected<U, G>&>
          && !std::is_constructible_v<unexpected<E>, const expected<U, G>>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs) : has_val(rhs.has_val)
    {
      if (!has_val)
        new (&unex) U(std::forward<U>(*rhs.error()));
    }

    template<class G>
    requires std::is_constructible_v<E, const G&>
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>& e)
    {
      new (&unex) const G& (std::forward<const G&>(*e.value()));
      has_val = false;
    }

    template<class G>
    requires std::is_constructible_v<E, G>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&& e)
    {
      new (&unex) G(std::forward<G>(*e.value()));
      has_val = false;
    }

    constexpr explicit expected(std::in_place_t) noexcept
    {
      has_val = true;
    }

    template<class... Args>
    requires std::is_constructible_v<E, Args...>
    constexpr explicit expected(unexpect_t, Args&&... args)
    {
      new (&unex) E(std::forward<Args>(args)...);
      has_val = false;
    }

    template<class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
    {
      new (&unex) U(il, std::forward<Args>(args)...);
      has_val = false;
    }


    // Destructor
    constexpr ~expected();

    // Assignment
    constexpr expected& operator=(const expected& rhs)
    requires std::is_copy_assignable_v<E>
          && std::is_copy_constructible_v<E>
    {
      if (this->has_value() && rhs.has_value())
      {
        return *this;
      }
      else
      {
        if (this->has_value())
        {
          std::construct_at(std::addressof(unex), rhs.unex);
          has_val = false;
          return *this;
        }
        else
        {
          if (rhs.has_value())
          {
            std::destroy_at(std::addressof(unex));
            has_val = true;
            return *this;
          }
          else
          {
            unex = rhs.error();
            return *this;
          }
        }
      }
    }

    constexpr expected& operator=(expected&& rhs) noexcept(std::is_nothrow_move_constructible_v<E>
                                                        && std::is_nothrow_move_assignable_v<E>)
    requires std::is_move_constructible_v<E>
          && std::is_move_assignable_v<E>
    {
      if (this->has_value() && rhs.has_value())
      {
        return *this;
      }
      else
      {
        if (this->has_value())
        {
          std::construct_at(std::addressof(unex),
            std::move(rhs.unex));
          has_val = false;
        }
        else
        {
          if (rhs.has_value())
          {
            std::destroy_at(std::addressof(unex));
            has_val = true;
            return *this;
          }
          else
          {
            unex = rhs.error();
            return *this;
          }
        }
      }
    }

    template<class G>
    requires std::is_constructible_v<E, const G&>
          && std::is_assignable_v<E&, const G&>
    constexpr expected& operator=(const unexpected<G>& e)
    {
      if (has_value())
      {
        std::construct_at(std::addressof(unex), std::forward<const G&>(e.value()));
        has_val = false;
        return *this;
      }
      else
      {
        unex = std::forward<const G&>(e.value());
        return *this;
      }
    }

    template<class G>
      requires std::is_constructible_v<E, G>
    && std::is_assignable_v<E&, G>
      constexpr expected& operator=(unexpected<G>&& e)
    {
      if (has_value())
      {
        std::construct_at(std::addressof(unex), std::forward<G>(e.value()));
        has_val = false;
        return *this;
      }
      else
      {
        unex = std::forward<G>(e.value());
        return *this;
      }
    }

    // Modifiers
    constexpr void emplace() noexcept
    {
      if (!has_value())
      {
        std::destroy_at(std::addressof(unex));
        has_val = true;
      }
    }

    // Swap
    constexpr void swap(expected& rhs) noexcept(std::is_nothrow_move_constructible_v<E>
                                             && std::is_nothrow_swappable_v<E>)
    {
      if (has_val)
      {
        if (rhs.has_val)
        {
        }
        else
        {
          std::construct_at(std::addressof(unex), std::move(rhs.unex));
          std::destroy_at(std::addressof(rhs.unex));
          has_val = false;
          rhs.has_val = true;
        }
      }
      else
      {
        if (!rhs.has_val)
        {
          using std::swap;
          swap(unex, rhs.unex);
        }
        else
        {
          rhs.swap(*this);
        }
      }
    }

    // Observers
    constexpr explicit operator bool() const noexcept
    {
      return has_val;
    }
    constexpr bool has_value() const noexcept
    {
      return has_val;
    }
    constexpr void operator*() const noexcept
    {
      has_val = true;
    }
    constexpr void value() const&
    {
      if (!has_value())
        throw (bad_expected_access(error()));
    }
    constexpr void value()&&
    {
      if (!has_value())
        throw (bad_expected_access(std::move(error())));
    }
    constexpr const E& error() const&
    {
      has_val = false;
      return unex;
    }
    constexpr E& error()&
    {
      has_val = false;
      return unex;
    }
    constexpr const E&& error() const&&
    {
      has_val = false;
      return std::move(unex);
    }
    constexpr E&& error()&&
    {
      has_val = false;
      return std::move(unex);
    }

    //Expected equality operators
    template<class T2, class E2>
    requires std::is_void_v<T2>
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y)
    {
      if (!std::equal(x.has_value(), y.has_value()))
        return false;
      else
        return x.has_value() || static_cast<bool>(x.error() == y.error());
    }

    template<class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e)
    {
      return !x.has_value() && static_cast<bool>(x.error() == e.value());
    }
  };
}