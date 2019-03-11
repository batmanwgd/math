#ifndef STAN_MATH_PRIM_META_IS_VAR_OR_ARITHMETIC_HPP
#define STAN_MATH_PRIM_META_IS_VAR_OR_ARITHMETIC_HPP

#include <stan/math/prim/meta/is_var.hpp>
#include <stan/math/prim/meta/scalar_type.hpp>
#include <type_traits>




namespace stan {

template <typename T1, typename T2 = double, typename T3 = double,
          typename T4 = double, typename T5 = double, typename T6 = double>
struct is_var_or_arithmetic {
  enum {
    value = (is_var<typename scalar_type<T1>::type>::value
             || std::is_arithmetic<typename scalar_type<T1>::type>::value)
            && (is_var<typename scalar_type<T2>::type>::value
                || std::is_arithmetic<typename scalar_type<T2>::type>::value)
            && (is_var<typename scalar_type<T3>::type>::value
                || std::is_arithmetic<typename scalar_type<T3>::type>::value)
            && (is_var<typename scalar_type<T4>::type>::value
                || std::is_arithmetic<typename scalar_type<T4>::type>::value)
            && (is_var<typename scalar_type<T5>::type>::value
                || std::is_arithmetic<typename scalar_type<T5>::type>::value)
            && (is_var<typename scalar_type<T6>::type>::value
                || std::is_arithmetic<typename scalar_type<T6>::type>::value)
  };
};

}  // namespace stan
#endif