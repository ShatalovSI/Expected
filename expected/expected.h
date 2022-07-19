#pragma once

#include <iostream>
#include <variant>
#include "unexpected.h"
#include "structs.h"

namespace details
{
  template <typename Invocable, typename T>
  concept and_then_invocable = requires
  {
    requires std::invocable<Invocable, T>;
    typename std::invoke_result_t<Invocable, T>::value_type;
    typename std::invoke_result_t<Invocable, T>::error_type;
  };
}

struct expect_t
{
  explicit expect_t() = default;
};

inline constexpr expect_t expect{};





namespace nonstd
{
  template <typename T, typename E>
  class expected
  {
  private:
    // [1] alignas(std::max(alignof(T), alignof(E))) m_storage[sizeof(std::max(sizeof(T), sizeof(E)))];
    std::variant<T, unexpected<E>> m_variant;

  public:

    using value_type = T;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    template <class U>
    using rebind = expected<U, error_type>;

    // Constructors
    template <typename T>
      requires std::is_default_constructible_v<T>
    constexpr expected() = default;

    constexpr expected(const expected& rhs) = default;
    
    template <typename T, typename E>
      requires std::is_move_constructible_v<T>
            && std::is_move_constructible_v<E>
    constexpr expected(expected &&rhs) noexcept = default;

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
    constexpr explicit(!std::is_convertible_v<U, T> || !std::is_convertible_v<G, E>)
    expected(const expected<U, G>& rhs) : m_variant { rhs.m_variant }
    {
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
    constexpr explicit(!std::is_convertible_v<U, T> || !std::is_convertible_v<G, E>)
    expected(expected<U, G> &&rhs) : m_variant { std::move(rhs.m_variant) }
    {
    }

    template <class U = T>
      requires !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> // ???
            && !traits::is_expected<std::remove_cvref_t<U>>
            && !is_unexpected_v<std::remove_cvref_t<U>>
            && std::is_constructible_v<T, U>
    constexpr explicit(!std::is_convertible_v<U, T>) expected(U&& v) : m_variant { std::in_place_type<T>, std::forward<U>(v) }
    {
    }

    template <class G>
      requires std::is_constructible_v<E, const G&>
    constexpr expected(const unexpected<const G&>& e) : m_variant { std::in_place_type<unexpected<E>>, e.value() }
    {
    }

    template<class G>
    requires std::is_constructible_v<E, G>
    constexpr expected(unexpected<G>&& e)
      : m_variant { std::in_place_type<unexpected<E>>, std::move(e).value() }
    {
    }

    template<class... Args>
    requires std::is_constructible_v<T, Args...>
    constexpr explicit expected(std::in_place_t, Args&&...args)
      : m_variant { std::in_place_type<T>, std::forward<Args>(args)... }
    {
    }

    template<class U, class... Args>
    requires std::is_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr explicit expected(std::in_place_t, std::initializer_list<U> il, Args&&... args)
      : m_variant { std::in_place_type<T>, il, std::forward<Args>(args)... }
    {
    }

    template<class... Args>
    requires std::is_constructible_v<E, Args...>
    constexpr explicit expected(unexpect_t, Args&&...args)
      : m_variant { std::in_place_type<unexpected<E>>, std::forward<Args>(args)... }
    {
    }

    template<class U, class... Args>
    requires std::is_constructible_v<E, std::initializer_list<U>&, Args...>
    constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&...args)
      : m_variant { std::in_place_type<unexpected<E>>, il, std::forward<Args>(args)... }
    {
    }

    // Destructor
    constexpr ~expected() = default;

    // Assigment


    template<class T, class U, class... Args> 
    static void reinit_expected(T &newval, U &oldval, Args &&...args)
    {
      if constexpr (std::is_nothrow_constructible_v<T, Args...>)
      {
        std::destroy_at(std::addressof(oldval));
        std::construct_at(std::addressof(newval), std::forward<Args>(args)...);
      }
      else if constexpr (std::is_nothrow_move_constructible_v<T>)
      {
        T tmp(std::forward<Args>(args)...);
        std::destroy_at(std::addressof(oldval));
        std::construct_at(std::addressof(newval), std::move(tmp));
      }
      else
      {
        U tmp(std::move(oldval));
        std::destroy_at(std::addressof(oldval));
        try
        {
          std::construct_at(std::addressof(newval), std::forward<Args>(args)...);
        }
        catch (...)
        {
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
    constexpr expected& operator=(const expected &rhs) = default;

    template <typename T, typename E>
      requires std::is_move_constructible_v<T>
            && std::is_move_assignable_v<T>
            && std::is_move_constructible_v<E>
            && std::is_move_assignable_v<E>
            && std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>
    constexpr expected & operator=(expected && rhs) noexcept = default;

    template<class U = T>
      requires !traits::is_expected<std::remove_cvref_t<U>>            
            && !is_unexpected_v<std::remove_cvref_t<U>>
            && std::is_constructible_v<T, U>
            && std::is_assignable_v<T&, U>
            && std::is_nothrow_constructible_v<T, U> || std::is_nothrow_move_constructible_v<E>
    constexpr expected& operator=(U && v)
    {
              //val = std::forward<U>(v); return *this;
      m_variant.emplace<T>(std::forward<U>(v));
    }


    template<class G>
      requires std::is_constructible_v < E, const G&>
            && std::is_assignable_v<E&, const G&>
            && std::is_nothrow_constructible_v<E, const G&> || std::is_nothrow_move_constructible_v<T>
    constexpr expected & operator=(const unexpected<G> &e)
    {
      m_variant.emplace<unexpected<E>>(e.value());
    }

    template<class G>
      requires std::is_constructible_v < E, G>
            && std::is_assignable_v<E&, G>
            && std::is_nothrow_constructible_v<E, G> || std::is_nothrow_move_constructible_v<T>
    constexpr expected & operator=(unexpected<G> &&e) noexcept
    {
      m_variant.emplace<unexpected<E>>(std::move(e).value());
    }

    // Modifires
    template<class... Args>
      requires std::is_nothrow_constructible_v<T, Args...>
    constexpr T& emplace(Args&&...args) noexcept
    {
      return m_variant.emplace<T>(std::forward<Args>(args)...);
    }

    template<class U, class... Args>
      requires std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>
    constexpr T& emplace(std::initializer_list<U> il, Args&&... args) noexcept
    {
      return m_variant.emplace<T>(il, std::forward<Args>(args)...);
    }

    // Observes
    constexpr const T* operator->() const noexcept
    {
      auto ptr = std::get_if<T>(m_variant);
      if (!ptr)
      {
        throw bad_expected_access { "Fucking animal..." };
      }
      return ptr;
    }

    constexpr T* operator->() noexcept
    {
      return const_cast<T*>(static_cast<const expected<T, E> &>(*this).operator->());
    }

    constexpr const T& operator*() const & noexcept
    {
      return std::get<T>(m_variant);
    }
    constexpr T& operator*() & noexcept
    {
      return const_cast<T&>(static_cast<const expected<T, E> &>(*this).operator*());
    }
    constexpr const T&& operator*() const&& noexcept
    {
      return std::get<T>(std::move(m_variant));
    }
    constexpr T&& operator*() && noexcept
    {
      return const_cast<T&&>(static_cast<const expected<T, E> &&>(*this).operator*());
    }
    constexpr explicit operator bool() const noexcept
    {
      return std::holds_alternative<T>(m_variant);
    }

    constexpr bool has_value() const noexcept
    {
      return std::holds_alternative<T>(m_variant);
    }

    constexpr const T& value() const&
    {
      if (has_value())
        return std::get<T>(m_variant);
      else
        throw bad_expected_access{ "Fucking animal..." };
    }
    constexpr T& value()&
    {
      if (has_value())
        return std::get<T>(m_variant);
      else
        throw bad_expected_access{ "Fucking animal..." }; /// ???
    }
    constexpr T&& value()&&
    {
      if (has_value())
        return std::move(std::get<T>(m_variant));
      else
        throw bad_expected_access(std::move(error()));
    }
    constexpr const T&& value() const&&
    {
      if (has_value())
        return std::move(std::get<T>(m_variant));
      else
        throw bad_expected_access(std::move(error()));
    }

    constexpr const E& error() const& noexcept
    {
      return std::get<unexpected<E>>(m_variant);
    }
    constexpr E& error() & noexcept
    {
      return std::get<unexpected<E>>(m_variant);
    }
    constexpr E&& error() && noexcept
    {
      return std::move(std::get<unexpected<E>>(m_variant));
    }
    constexpr const E&& error() const&& noexcept
    {
      return std::move(std::get<unexpected<E>>(m_variant));
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
      return has_value() ? std::move(**this) : static_cast<T>(std::forward<U>(v));
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
      using std::swap;
      swap(m_variant, rhs.m_variant);
    }

    //Expected equality operators
    template<class T2, class E2>
      requires (!is_void_v<T2>)
    friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y)
    {
      if (x.has_value() != y.has_value())
      {
        return false;
      }

      if (x.has_value())
      {
        return *x == *y;
      }
      
      return x.error() == y.error();
    }

    template<class T2> // The expression *x == v is well-formed and its result is convertible to bool
    friend constexpr bool operator==(const expected& x, const T2& v)
    {
      return x.has_value() && static_cast<bool>(*x == v);
    }

    template<class E2>
    friend constexpr bool operator==(const expected& x, const unexpected<E2>& e)
    {
      return !x.has_value() && static_cast<bool>(x.error() == e.value());
    }

    // Monadic optional

    template <details::and_then_invocable<T> Invocable>
    constexpr auto and_then(Invocable &&f) const & -> expected<typename std::invoke_result_t<Invocable, T>::value_type,
                                                               typename std::invoke_result_t<Invocable, T>::error_type>
    {
      if (has_value())
        return std::forward<Invocable>(f)(value());
      else
        return {};
    }

    template <details::and_then_invocable<T> Invocable>
    constexpr auto and_then(Invocable &&f) && -> expected<typename std::invoke_result_t<Invocable, T>::value_type,
                                                          typename std::invoke_result_t<Invocable, T>::error_type>
    {
      if (has_value())
        return std::forward<Invocable>(f)(std::move(value()));
      else
        return {};
    }
    
    template<std::invocable<T> Invocable>
    constexpr auto transform (Invocable&& f) const & -> expected<std::invoke_result_t<Invocable, T>, E> 
    {
      using ResultType = expected<std::invoke_result_t<Invocable, T>, E>;
      if (has_value())
        return ResultType{ std::forward<Invocable>(f)(value()) };
      else
        return {};
    }
    
    template<std::invocable<T> Invocable>
    constexpr auto transform(Invocable&& f) && -> expected<std::invoke_result_t<Invocable, T>, E>
    {
      using ResultType = expected<std::invoke_result_t<Invocable, T>, E>;
      if (has_value)
        return ResultType{ std::forward<Invocable>(f)(std::move(value())) };
      else
        return {};
    }

    template<std::invocable<E> Invocable>
    constexpr auto transform_error(Invocable&& f) const& -> expected<T, std::invoke_result_t<Invocable, E>>
    {
      using ResultType = expected<T, std::invoke_result_t<Invocable, E>>;
      if (!has_value())
        return ResultType{ std::forward<Invocable>(f)(value()) };
      else
        return value();
    }

    template<std::invocable<E> Invocable>
    constexpr auto transform_error(Invocable&& f) && -> expected<T, std::invoke_result_t<Invocable, E>>
    {
      using ResultType = expected<T, std::invoke_result_t<Invocable, E>>;
      if (!has_value())
        return ResultType{ std::forward<Invocable>(f)(std::move(error()))};
      else
        return std::move(value());
    }

    template <std::invocable Invocable>
    constexpr auto or_else(Invocable&& f) const & -> expected <T, E>
    {
      if (!has_value())
        return std::forward<Invocable>(f)(error());
      else
        return {};
    }

    template <std::invocable Invocable>
    constexpr auto or_else(Invocable&& f) && -> expected <T, E>
    {
      if (!has_value())
        return std::forward<Invocable>(f)(std::move(error()));
      else
        return {};
    }
  };

  template <typename T, typename E>
  constexpr void swap(expected<T, E>& x, expected<T, E>& y) noexcept(noexcept(x.swap(y)))
  {
    x.swap(y);
  }
}