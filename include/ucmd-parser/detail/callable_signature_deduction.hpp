// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <type_traits>

namespace ucmdp::detail
{


template< typename, typename = std::void_t<> >
struct is_functor : std::false_type
{
};

template< typename T >
struct is_functor<T, std::void_t<decltype(&T::operator())> > : std::true_type
{
};

template< typename T >
constexpr bool is_functor_v = is_functor<T>::value;


template< typename C >
struct functor_deduction_helper;

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...)>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) const>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) volatile>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) const volatile>
{
    using function_type = std::function<R(Args...)>;
};

#if __cpp_noexcept_function_type >= 201510 // || _MSC_VER >= xxx

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) noexcept>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) const noexcept>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) volatile noexcept>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) const volatile noexcept>
{
    using function_type = std::function<R(Args...)>;
};

#endif


template< typename T >
struct callable_signature_deducer
{
    static_assert(is_functor_v<T>, "cannot deduce the signature of non function types");
    using function_type = typename functor_deduction_helper<decltype(&T::operator())>::function_type;
};

template< typename R, typename... Args >
struct callable_signature_deducer<R (&)(Args...)>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename... Args >
struct callable_signature_deducer<R (*)(Args...)>
{
    using function_type = std::function<R(Args...)>;
};

#if __cpp_noexcept_function_type >= 201510 // || _MSC_VER >= xxx

template< typename R, typename... Args >
struct callable_signature_deducer<R(&)(Args...) noexcept>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename... Args >
struct callable_signature_deducer<R(*)(Args...) noexcept>
{
    using function_type = std::function<R(Args...)>;
};

#endif


template< typename C >
using function_type_of = typename callable_signature_deducer<C>::function_type;


}
