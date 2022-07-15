#pragma once

#include <iostream>
#include "unexpected.h"
#include "structs.h"

namespace nonstd
{
  template <typename T, typename E>
  class expected
  {
  private:
    bool has_val = true;
    union
    {
      T val;
      E unex;
    };

  public:

    using value_type = T;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    template<class U>
    using rebind = expected<U, error_type>;

    // Constructors
    template <typename T>
    requires std::is_default_constructible_v<T>
    constexpr expected()
    {
      new (&val) T();
      has_val = true;
    }

    constexpr expected(const expected& rhs) : has_val(rhs.has_val) // ??
    {
      if (has_val)
        new(&val) T(rhs.val);
      else
        new(&unex) E(rhs.unex);
    }

    template <typename T, typename E>
    requires std::is_move_constructible_v<T>
          && std::is_move_constructible_v<E>
    constexpr expected(expected&& rhs) noexcept(" ") : has_val(rhs.has_val)// Что в noexcept?
    {
      if (has_val)
        new(&val) T(std::move(rhs.val));
      else
        new(&unex) E(std::move(rhs.unex));
    }

    template<class U, class G>
    requires std::is_constructible_v<T, const U&>
          && std::is_constructible_v<E, const G&>
          && !std::is_constructible_v<T, expected<U, G>&>
          && !std::is_constructible_v<T, expected<U, G>>
          && !std::is_constructible_v<T, const expected<U, G>&>
          && !std::is_constructible_v<T, const expected<U, G>>
          && !std::is_convertible_v<expected<U, G>&, T>
          && !std::is_convertible_v<expected<U, G>&&, T>
          && !std::is_convertible_v<const expected<U, G>&, T>
          && !std::is_convertible_v<const expected<U, G>&&, T>
          && !std::is_constructible_v<unexpected<E>, expected<U, G>&>
          && !std::is_constructible_v<unexpected<E>, expected<U, G>>
          && !std::is_constructible_v<unexpected<E>, const expected<U, G>&>
          && !std::is_constructible_v<unexpected<E>, const expected<U, G>>
    constexpr explicit(!std::is_convertible_v<U, T> 
                    || !std::is_convertible_v<G, E>) expected(const expected<U, G>& rhs) : has_val(rhs.has_val)
    {
      if (has_val)
        new (&val) const U& (std::forward<const U&>(*rhs));
      else
        new (&unex) const G& (std::forward<const G&>(rhs.unex));
    }

    template<class U, class G>
    requires std::is_constructible_v<T, U>
          && std::is_constructible_v<E, G>
          && !std::is_constructible_v<T, expected<U, G>&>
          && !std::is_constructible_v<T, expected<U, G>>
          && !std::is_constructible_v<T, const expected<U, G>&>
          && !std::is_constructible_v<T, const expected<U, G>>
          && !std::is_convertible_v<expected<U, G>&, T>
          && !std::is_convertible_v<expected<U, G>&&, T>
          && !std::is_convertible_v<const expected<U, G>&, T>
          && !std::is_convertible_v<const expected<U, G>&&, T>
          && !std::is_constructible_v<unexpected<E>, expected<U, G>&>
          && !std::is_constructible_v<unexpected<E>, expected<U, G>>
          && !std::is_constructible_v<unexpected<E>, const expected<U, G>&>
          && !std::is_constructible_v<unexpected<E>, const expected<U, G>>
    constexpr explicit(!std::is_convertible_v<U, T> 
                    || !std::is_convertible_v<G, E>) expected(expected<U, G>&& rhs) : has_val(rhs.has_val)
    {
      if (has_val)
        new (&val) U(std::forward<U>(*rhs));
      else
        new (&unex) G(std::forward<G>(rhs.unex));
    }

    template <class U = T>
    requires !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t>
          && !std::is_same_v<expected<T, E>, std::remove_cvref_t<U>>
          && !is_unexpected_v<std::remove_cvref_t<U>>
          && std::is_constructible_v<T, U>
    constexpr explicit(!std::is_convertible_v<U, T>) expected(U&& v)
    {
      new (&val) T(std::forward<U>(v));
      has_val = true;
    }

    template<class G>
    requires std::is_constructible_v<E, const G&>
    constexpr expected(const unexpected<const G&>& e)
    {
      new (&unex) const G& (std::forward<const G&>(e.error()));
      has_val = false;
    }

    template<class G>
    requires std::is_constructible_v<E, G>
    constexpr expected(unexpected<G>&& e)
    {
      new (&unex) G(std::forward<G>(e.error()));
      has_val = false;
    }

    template<class... Args>
    requires std::is_constructible_v<T, Args...>
    constexpr explicit expected(std::in_place_t, Args&&...args)
    {
      new (&val) T(std::forward<Args>(args)...);
      has_val = true;
    }

    template<class U, class... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr explicit expected(std::in_place_t, std::initializer_list<U> il, Args&&... args)
    {
      new (&val) U(il, std::forward<Args>(args)...);
      has_val = true;
    }

    template<class... Args>
    requires std::is_constructible_v<E, Args...>
    constexpr explicit expected(unexpect_t, Args&&...args)
    {
      new (&unex) E(std::forward<Args>(args)...);
      has_val = false;
    }

    template<class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&...args)
    {
      new (&unex) U(il, std::forward<Args>(args)...);
      has_val = true;
    }

    // Destructor
    ~expected() {};

    // Assigment

    template<class T, class U, class... Args>
    constexpr void reinit_expected(T& newval, U& oldval, Args&&... args)
    {
      if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
        std::destroy_at(std::addressof(oldval));
        std::construct_at(std::addressof(newval), std::forward<Args>(args)...);
      }
      else if constexpr (std::is_nothrow_move_constructible_v<T>) {
        T tmp(std::forward<Args>(args)...);
        std::destroy_at(std::addressof(oldval));
        std::construct_at(std::addressof(newval), std::move(tmp));
      }
      else {
        U tmp(std::move(oldval));
        std::destroy_at(std::addressof(oldval));
        try {
          std::construct_at(std::addressof(newval), std::forward<Args>(args)...);
        }
        catch (...) {
          std::construct_at(std::addressof(oldval), std::move(tmp));
          throw;
        }
      }
    }

    template <typename T, typename E>
    requires std::is_copy_assignable_v<T>
          && std::is_copy_constructible_v<T>
          && std::is_copy_assignable_v<E>
          && std::is_copy_constructible_v<E>
          && std::is_nothrow_move_constructible_v<E> || std::is_nothrow_move_constructible_v<T>
    constexpr expected & operator=(const expected &rhs)
    {
      if (this->has_value() && rhs.has_value())
        val = *rhs;
      else
      {
        if (this->has_value())
          this->reinit_expected(unex, val, rhs.error());
        else
        {
          if (rhs.has_value())
            this->reinit_expected(val, unex, *rhs);
          else
          {
            unex = rhs.error();
          }
        }
      }
    }

    template <typename T, typename E>
    requires std::is_move_constructible_v<T>
          && std::is_move_assignable_v<T>
          && std::is_move_constructible_v<E>
          && std::is_move_assignable_v<E>
          && std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>
      constexpr expected & operator=(expected && rhs) noexcept(" ")
    {
      if (has_val && rhs.has_val)
        val = std::move(*rhs);
      else
      {
        if (has_val)
          this->reinit_expected(unex, val, std::move(rhs.error()));
        else
        {
          if (has_val)
            this->reinit_expected(val, unex, std::move(*rhs));
          else
            unex = std::move(rhs.error());
        }
      }
    }

    template<class U = T>
    requires !std::is_same_v<expected, remove_cvref_t<U>>
          && !is_unexpected_v<std::remove_cvref_t<U>>
          && std::is_constructible_v<T, U>
          && std::is_assignable_v<T&, U>
          && std::is_nothrow_constructible_v<T, U> || std::is_nothrow_move_constructible_v<E>
      constexpr expected & operator=(U && v)
    {
      if (has_val)
      {
        val = std::forward<U>(v);
        return *this;
      }
      else
      {
        this->reinit_expected(val, unex, std::forward<U>(v));
        has_val = true;
        return *this;
      }
    }


    template<class G>
    requires std::is_constructible_v < E, const G&>
          && std::is_assignable_v<E&, const G&>
          && std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T>
    constexpr expected & operator=(const unexpected<G>&e)
    {
      if (has_val)
      {
        this->reinit_expected(unex, val, std::forward<const G&>(e.value()));
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
    requires std::is_constructible_v < E, G>
          && std::is_assignable_v<E&, G>
          && std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T>
    constexpr expected & operator=(unexpected<G> && e)
    {
      if (has_val)
      {
        this->reinit_expected(unex, val, std::forward<G>(e.value()));
        has_val = false;
        return *this;
      }
      else
      {
        unex = std::forward<G>(e.value());
        return *this;
      }
    }

    // Modifires
    template<class... Args>
    requires std::is_nothrow_constructible_v<T, Args...>
    constexpr T& emplace(Args&&...args) noexcept
    {
      if (has_value())
        std::destroy_at(std::addressof(val));
      else {
        std::destroy_at(std::addressof(unex));
        has_val = true;
      }
      return *std::construct_at(std::addressof(val), std::forward<Args>(args)...);
    }

    template<class U, class... Args>
    requires std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr T& emplace(std::initializer_list<U> il, Args&&... args) noexcept
    {
      if (has_value())
        std::destroy_at(std::addressof(val));
      else {
        std::destroy_at(std::addressof(unex));
        has_val = true;
      }
      return *std::construct_at(std::addressof(val), il, std::forward<Args>(args)...);
    }

    // Observes
    constexpr const T* operator->() const noexcept
    {
      if (!has_val)
        throw (unex);
      else
        return std::addressof(val);
    }
    constexpr T* operator->() noexcept
    {
      if (!has_val)
        throw (unex);
      else
        return std::addressof(val);
    }
    constexpr const T& operator*() const& noexcept
    {
      if (!has_val)
        throw (unex);
      else
        return val;
    }
    constexpr T& operator*() & noexcept
    {
      if (!has_val)
        throw (unex);
      else
        return val;
    }
    constexpr const T&& operator*() const&& noexcept
    {
      if (!has_val)
        throw (unex);
      else
        return std::move(val);
    }
    constexpr T&& operator*() && noexcept
    {
      if (!has_val)
        throw (unex);
      else
        return std::move(val);
    }
    constexpr explicit operator bool() const noexcept
    {
      return has_val;
    }
    constexpr bool has_value() const noexcept
    {
      return has_val;
    }
    constexpr const T& value() const&
    {
      if (!has_val)
        throw (unex);
      else
      {
        return val;
        throw(bad_expected_access(error()));
      }
    }
    constexpr T& value()&
    {
      if (!has_val)
        throw (unex);
      else
      {
        return val;
        throw(bad_expected_access(error()));
      }
    }
    constexpr T&& value()&&
    {
      if (!has_val)
        throw (unex);
      else
      {
        return std::move(val);
        bad_expected_access(std::move(error()));
      }
    }
    constexpr const T&& value() const&&
    {
      if (!has_val)
        throw (unex);
      else
      {
        return std::move(val);
        bad_expected_access(std::move(error()));
      }
    }
    constexpr const E& error() const& noexcept
    {
      return unex;
    }
    constexpr E& error() & noexcept
    {
      return unex;
    }
    constexpr E&& error() && noexcept
    {
      return std::move(unex);
    }
    constexpr const E&& error() const&& noexcept
    {
      return std::move(unex);
    }
    template<class U>
    requires std::is_copy_constructible_v<T>
          && std::is_convertible<U, T>
    constexpr T value_or(U&& v) const&
    {
      return has_value() ? **this : static_cast<T>(std::forward<U>(v));
    }
    template<class U>
    requires std::is_move_constructible_v<T>
          && std::is_convertible<U, T>
    constexpr T value_or(U&& v)&&
    {
      has_value() ? std::move(**this) : static_cast<T>(std::forward<U>(v));
    }

    //Swap
    template <typename T, E>
    requires std::is_swappable_v<T>
          && std::is_swappable_v<E>
          && std::is_move_constructible_v<T>&& std::is_move_constructible_v<E>
          && std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>
    constexpr void swap(expected & rhs) noexcept(std::is_nothrow_move_constructible_v<T>
                                              && std::is_nothrow_swappable_v<T>
                                              && std::is_nothrow_move_constructible_v<E>
                                              && std::is_nothrow_swappable_v<E>)
    {
      if (has_val)
      {
        if (rhs.has_val)
        {
          using std::swap;
          swap(val, rhs.val);
        }
        else
        {
          if constexpr (std::is_nothrow_move_constructible_v<E>)
          {
            E tmp(std::move(rhs.unex));
            std::destroy_at(std::addressof(rhs.unex));
            try
            {
              std::construct_at(std::addressof(rhs.val), std::move(val));
              std::destroy_at(std::addressof(val));
              std::construct_at(std::addressof(unex), std::move(tmp));
            }
            catch (...)
            {
              std::construct_at(std::addressof(rhs.unex), std::move(tmp));
              throw;
            }
          }
          else
          {
            T tmp(std::move(val));
            std::destroy_at(std::addressof(val));
            try
            {
              std::construct_at(std::addressof(unex), std::move(rhs.unex));
              std::destroy_at(std::addressof(rhs.unex));
              std::construct_at(std::addressof(rhs.val), std::move(tmp));
            }
            catch (...)
            {
              std::construct_at(std::addressof(val), std::move(tmp));
              throw;
            }
          }
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

    //Expected equality operators
    template<class T2, class E2>
      requires (!is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y)
    {
      if (!std::equal(x.has_value(), y.has_value()))
        return false;
      else
      {
        if (x.has_value())
          return *x == *y;
        else
          return x.error() == y.error();
      }
    }
    template<class T2>
    friend constexpr bool operator==(const expected& x, const T2& v)
    {
      return x.has_value() && static_cast<bool>(*x == v);
    }
    template<class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e)
    {
      return !x.has_value() && static_cast<bool>(x.error() == e.value());
    }
  };

  template <typename T, typename E>
  constexpr void swap(expected<T, E>& x, expected<T, E>& y) noexcept(noexcept(x.swap(y)))
  {
    x.swap(y);
  }
}