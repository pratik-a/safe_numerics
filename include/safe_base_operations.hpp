#ifndef BOOST_NUMERIC_SAFE_BASE_OPERATIONS_HPP
#define BOOST_NUMERIC_SAFE_BASE_OPERATIONS_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  Copyright (c) 2012 Robert Ramey
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "safe_base.hpp"
#include "policies.hpp"
#include "checked.hpp"

#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
//#include <boost/type_traits/is_convertible.hpp>
//#include <boost/utility/enable_if.hpp>
//#include <boost/mpl/print.hpp>
#include <type_traits> // is_convertable, enable_if

namespace boost {
namespace numeric {

template<class T, class U>
struct get_common_policies {
    static_assert(
        boost::mpl::or_<
            is_safe<T>,
            is_safe<U>
        >::type::value,
        "at least one type must be a safe type"
    );

    typedef typename boost::mpl::eval_if<
        is_safe<T>,
        get_policies<T>,
        boost::mpl::identity<boost::mpl::void_>
    >::type::type policies_t;

    typedef typename boost::mpl::eval_if<
        is_safe<U>,
        get_policies<U>,
        boost::mpl::identity<boost::mpl::void_>
    >::type::type policies_u;

    // if both types are safe, the policies have to be the same!
    static_assert(
        boost::mpl::if_<
            boost::mpl::and_<
                is_safe<T>,
                is_safe<U>
            >,
            typename boost::is_same<policies_t, policies_u>,
            boost::mpl::true_
        >::type::value,
        "if both types are safe, the policies have to be the same!"
    );

    // now we've verified that there is no conflict between policies
    // return the one from the first safe type
    typedef typename boost::mpl::if_<
        is_safe<T>,
        policies_t,
    typename boost::mpl::if_<
        is_safe<U>,
        policies_u,
    //
        boost::mpl::void_
    >::type
    >::type type;
};

/////////////////////////////////////////////////////////////////
// binary operators

// Note: the following global operators will be only found via
// argument dependent lookup.  So they won't conflict any
// other global operators for types in namespaces other than
// boost::numeric

// These should catch things like U < safe_base<...> and implement them
// as safe_base<...> >= U which should be handled above.

/////////////////////////////////////////////////////////////////
// addition

template<class T, class U>
struct addition_result {
    typedef typename get_common_policies<T, U>::type P;
    typedef typename get_promotion_policy<P>::type promotion_policy;
    typedef typename get_exception_policy<P>::type exception_policy;
    typedef typename promotion_policy::template addition_result<T, U, P>::type type;
};

template<class T, class U>
typename boost::enable_if<
    boost::mpl::or_<
        boost::numeric::is_safe<T>,
        boost::numeric::is_safe<U>
    >,
    typename addition_result<T, U>::type
>::type
inline constexpr operator+(const T & t, const U & u){
    // argument dependent lookup should guarentee that we only get here
    // only if one of the types is a safe type. Verify this here
    static_assert(
        boost::mpl::or_<
            boost::numeric::is_safe<T>,
            boost::numeric::is_safe<U>
        >::value,
        "Neither type is a safe type"
    );
    typedef addition_result<T, U> ar;
    typedef typename ar::type result_type;
    typedef typename base_type<result_type>::type result_base_type;
    typedef typename ar::exception_policy exception_policy;
    return
        checked::add<result_base_type, exception_policy>(
            static_cast<const typename base_type<T>::type &>(t),
            static_cast<const typename base_type<U>::type &>(u)
        );
}

/////////////////////////////////////////////////////////////////
// subtraction
template<class T, class U>
struct subtraction_result {
    typedef typename get_common_policies<T, U>::type P;
    typedef typename get_promotion_policy<P>::type promotion_policy;
    typedef typename get_exception_policy<P>::type exception_policy;
    typedef typename promotion_policy::template subtraction_result<T, U, P>::type type;
};

template<class T, class U>
typename boost::enable_if<
    boost::mpl::or_<
        boost::numeric::is_safe<T>,
        boost::numeric::is_safe<U>
    >,
    typename subtraction_result<T, U>::type
>::type
inline constexpr operator-(const T & t, const U & u){
    // argument dependent lookup should guarentee that we only get here
    // only if one of the types is a safe type. Verify this here
    static_assert(
        boost::mpl::or_<
            boost::numeric::is_safe<T>,
            boost::numeric::is_safe<U>
        >::value,
        "Neither type is a safe type"
    );
    typedef subtraction_result<T, U> sr;
    typedef typename sr::type result_type;
    typedef typename base_type<result_type>::type result_base_type;
    typedef typename sr::exception_policy exception_policy;
    return
        checked::subtract<result_base_type, exception_policy >(
            static_cast<const typename base_type<T>::type &>(t),
            static_cast<const typename base_type<U>::type &>(u)
        );
}

// comparison operators
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    bool
>::type
operator<(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs > lhs;
}
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    bool
>::type
inline operator>(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs < lhs;
}
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    bool
>::type
inline operator==(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs == lhs;
}
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    bool
>::type
inline operator!=(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs != rhs;
}
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    bool
>::type
inline operator>=(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs <= lhs;
}
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    bool
>::type
inline operator<=(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return  rhs >= lhs;
}

// multiplication
template<class T, class Stored, class Derived, class PromotionPolicy>
typename boost::enable_if<
    boost::is_integral<T>,
    decltype(T() * Stored())
>::type
inline operator*(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs * lhs;
}

// division
// special case - possible overflow
template<class T, class Stored, class Derived, class PromotionPolicy>
typename boost::enable_if<
    boost::is_integral<T>,
    decltype(T() / Stored())
>::type
inline operator/(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    if(safe_compare::equal(0, rhs))
        throw std::domain_error("Divide by zero");
    return static_cast<
        decltype(T() / Stored())
    >(lhs / static_cast<const Stored &>(rhs));
}

// modulus
template<class T, class Stored, class Derived, class PromotionPolicy>
typename boost::enable_if<
    boost::is_integral<T>,
    decltype(T() % Stored())
>::type
inline operator%(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    if(safe_compare::equal(0, rhs))
        throw std::domain_error("Divide by zero");
    return static_cast<
        decltype(T() % Stored())
    >(lhs % static_cast<const Stored &>(rhs));
}

// logical operators
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    typename multiply_result_type<T, Stored>::type
>::type
inline operator|(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs | lhs;
}
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    decltype(T() & Stored())
>::type
inline operator&(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs & lhs;
}
template<class T, class Stored, class Derived>
typename boost::enable_if<
    boost::is_integral<T>,
    decltype(T() ^ Stored())
>::type
inline operator^(const T & lhs, const safe_base<Stored, Derived> & rhs) {
    return rhs ^ lhs;
}

} // numeric
} // boost

#endif // BOOST_NUMERIC_SAFE_BASE_OPERATIONS_HPP