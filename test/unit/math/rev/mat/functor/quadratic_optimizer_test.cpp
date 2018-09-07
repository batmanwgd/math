#include <stan/math/rev/core.hpp>
#include <stan/math/rev/mat/functor/quadratic_optimizer.hpp>
#include <stan/math/rev/mat/functor/eiquadprog.hpp>
#include <stan/math/rev/mat/functor/jacobian.hpp>
#include <test/unit/math/rev/mat/fun/util.hpp>
#include <test/unit/util.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <vector>

using stan::math::quadratic_optimizer;
using stan::math::quadratic_optimizer_analytical;
using stan::math::f_theta;
using stan::math::var;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::Matrix;
using Eigen::Dynamic;
using std::cout;

TEST(MathMatrix, eiquadprog) {
// implement example from 
// http://www.labri.fr/perso/guenneba/code/QuadProg/example.cpp.

  using Eigen::solve_quadprog;
  
  MatrixXd G(3, 3);
  G << 2.1, 0.0, 1.0,
       1.5, 2.2, 0.0,
       1.2, 1.3, 3.1;
  
  VectorXd g0(3);
  g0 << 6, 1, 1;
  
  MatrixXd CE(3, 1);
  CE << 1, 2, -1;
  
  VectorXd ce0(1);
  ce0(0) = -4;
  
  MatrixXd CI(3, 4);
  CI << 1, 0, 0, -1,
        0, 1, 0, -1,
        0, 0, 1,  0;

  VectorXd ci0(4);
  ci0 << 0, 0, 0, 10;

  VectorXd x;
  double f = solve_quadprog(G, g0, CE, ce0, CI, ci0, x);
  
  EXPECT_FLOAT_EQ(6.4, f);
  EXPECT_NEAR(0, x(0), 1e-12);  // Need to deal with floating point precision
  EXPECT_FLOAT_EQ(2, x(1));
  EXPECT_FLOAT_EQ(0, x(2));
}


struct fh {
  template <typename T0>
  inline Eigen::Matrix<T0, Eigen::Dynamic, Eigen::Dynamic> 
    operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
               const std::vector<double>& delta,
               const std::vector<int>& delta_int) const {
      int n = 2;
      return Eigen::MatrixXd::Identity(n, n);
    }
};

struct fv {
  template <typename T0>
  inline Eigen::Matrix<T0, Eigen::Dynamic, 1>
    operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
               const std::vector<double>& delta,
               const std::vector<int>& delta_int) const {
      int n = 2;
      Eigen::Matrix<T0, Eigen::Dynamic, 1> linear_term(n);
      linear_term(0) = 0;
      linear_term(1) = theta(1);
      return linear_term;
    }
};

struct fa_0 {
  template <typename T0>
  Eigen::Matrix<T0, Eigen::Dynamic, 1>
  inline operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
                    const std::vector<double>& delta,
                    const std::vector<int>& delta_int) const {
    int n = 2;
    Eigen::Matrix<T0, Eigen::Dynamic, 1> linear_constraint(n);
    linear_constraint(0) = 0;
    linear_constraint(1) = 0;
    return linear_constraint;
  }
};

struct fa {
  template <typename T0>
  Eigen::Matrix<T0, Eigen::Dynamic, 1>
  inline operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
                    const std::vector<double>& delta,
                    const std::vector<int>& delta_int) const {
    int n = 2;
    Eigen::Matrix<T0, Eigen::Dynamic, 1> linear_constraint(n);
    linear_constraint(0) = 1;
    linear_constraint(1) = 0;
    return linear_constraint;
  }
};

struct fb {
  template <typename T0>
  T0
  inline operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
                    const std::vector<double>& delta,
                    const std::vector<int>& delta_int) const {
    return theta(0);
  }
};

// template <typename F1, typename F2, typename F3, typename F4>
// struct f_theta {
//   VectorXd delta_;
//   VectorXd x_;
// 
//   // structure to be passed to the stan::math::jacobian function
//   f_theta(const F1& fh,
//           const F2& fv,
//           const F3& fa,
//           const F4& fb,
//           const VectorXd& delta,
//           const VectorXd& x) : delta_(delta), x_(x) {}
// 
//   template<typename T>
//   Eigen::Matrix<T, Eigen::Dynamic, 1>
//   inline operator()(const Eigen::Matrix<T, Eigen::Dynamic, 1>& theta) const {
//     return quadratic_optimizer_analytical(fh(), fv(), fa(), fb(),
//                                           theta, delta_, x_);
//   }
// };

TEST(MathMatrix, quadratic_optimizer_double) {
  VectorXd theta(2);
  theta << 0, -1;
  std::vector<double> delta;
  std::vector<int> delta_int;
  double tol = 1e-10;
  int n = 2;

  VectorXd x = quadratic_optimizer(fh(), fv(), fa_0(), fb(), theta, 
                                   delta, delta_int, n, 0, tol);
  EXPECT_EQ(x(0), 0);
  EXPECT_EQ(x(1), 1);

  theta << -5, -3;
  x = quadratic_optimizer(fh(), fv(), fa(), fb(), theta,
                          delta, delta_int, n);
  EXPECT_EQ(x(0), -theta(0));
  EXPECT_EQ(x(1), -theta(1));
  
  // Test analytical solution
  VectorXd x_an
    = quadratic_optimizer_analytical(fh(), fv(), fa(), fb(),
                                     theta, delta, delta_int, x);
  EXPECT_EQ(x_an(0), -theta(0));
  EXPECT_EQ(x_an(1), -theta(1));

  MatrixXd J_solution(2, 2);
  J_solution << -1, 0,
                0, -1;
  
  // Test gradient computation with analytical solution
  f_theta<fh, fv, fa, fb> f(fh(), fv(), fa(), fb(), delta, delta_int, x);
  VectorXd f_eval;
  MatrixXd J;
  stan::math::jacobian(f, theta, f_eval, J);
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++) EXPECT_EQ(J_solution(i, j), J(i, j));

  // cout << J << "\n";
  
  // theta << -1, 0;
  // std::cout << "Finite diff test \n"
  //           << quadratic_optimizer(fh(), fv(), fa(), fb(), theta, delta, n)
  //           << "\n \n";
  // 
  // double finite_diff = 1e-10;  // , finite_diff_2 = 1e-10;
  // VectorXd theta_lb = theta, theta_ub = theta;
  // theta_lb(1) = theta(1) - finite_diff;
  // theta_ub(1) = theta(1) + finite_diff;
  // 
  // VectorXd
  //   x_lb = quadratic_optimizer(fh(), fv(), fa(), fb(), theta_lb, delta, n),
  //   x_ub = quadratic_optimizer(fh(), fv(), fa(), fb(), theta_ub, delta, n);
  // 
  // std::cout << x_lb << "\n \n";
  // std::cout << x_ub << "\n \n";
  // 
  // std::cout << "dx_2 / d_theta_2 = "
  //           << (x_lb(1) - x_ub(1)) / (2 * finite_diff) << "\n \n";

  // note: if constraint forces theta = -1, then inequality constraint
  // is overritten. Not sure if this should be concerning.
  // This can be seen by writing the test with theta << 1, -1,
  // which returns x = {-1, 1}.

  // test the inequality constraint.
  // theta << 0, 1;
  // x = quadratic_optimizer(fh(), fv(), fa_0(), fb(), theta, delta, n);
  // std::cout << "Inequality constraint test:\n" << x << "\n \n";
}

TEST(MathMatrix, quadratic_optimizer_var) {
  std::vector<double> delta;
  std::vector<int> delta_int;
  int n_x = 2;
  int n_theta = 2;
  
  MatrixXd J(2, 2);
  J << -1, 0, 
       0, -1;

  for (int k = 0; k < n_x; k++) {
    Matrix<var, Dynamic, 1> theta(n_theta);
    theta << -5, -3;
    Matrix<var, Dynamic, 1> x = quadratic_optimizer(fh(), fv(), fa(), fb(),
                                                    theta, delta, delta_int,
                                                    n_x);
    EXPECT_EQ(-theta(0), x(0).val());
    EXPECT_EQ(-theta(1), x(1).val());
  
    AVEC parameter_vec = createAVEC(theta(0), theta(1));
    VEC g;
    x(k).grad(parameter_vec, g);
    EXPECT_EQ(J(k, 0), g[0]);
    EXPECT_EQ(J(k, 1), g[1]);
  }
}

// Shosh unit tests start here
struct fh_s1 {
  template <typename T0>
  inline Eigen::Matrix<T0, Eigen::Dynamic, Eigen::Dynamic>
    operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
               const std::vector<double>& delta,
               const std::vector<int>& delta_int) const {
      // declare "data"
      int n = 2;
      // VectorXd qa(2);
      // qa << 1.5,2;
      // VectorXd qe(2);
      // qe << 1,1;
      // VectorXd co(2);
      // co << 5,10;
      // double s = 82.5;


      // theta signiature: (gamma, sigma1sq,sigma2sq, alpha)
      // Eigen::Matrix<T0, Eigen::Dynamic, 2> H_mat(n,n) = MatrixXf::Zero();
      // H_mat = Identity(n, n);
      Eigen::Matrix<T0, Eigen::Dynamic, 2> H_mat(n,n);
      H_mat(0,0) = pow(theta(0), 2) * theta(1);
      H_mat(1,1) = pow(theta(0), 2) * theta(2);
      H_mat(0,1) = 0;
      H_mat(1,0) = 0;
      return H_mat;
    }
};

struct fv_s1 {
  template <typename T0>
  inline Eigen::Matrix<T0, Eigen::Dynamic, 1>
    operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
               const std::vector<double>& delta,
               const std::vector<int>& delta_int) const {

     // declare "data"
     int n = 2;
     VectorXd qa(2);
     qa << 1.5, 2;
     // VectorXd qe(2);
     // qe << 1,1;
     VectorXd co(2);
     co << 5, 10;
     // double s = 82.5;

     // theta signiature: (gamma, sigma1sq,sigma2sq, alpha)
      Eigen::Matrix<T0, Eigen::Dynamic, 1> linear_term(n);
      linear_term(0) = -( theta(0)*qa(0) + pow(theta(0),2)*theta(1)*theta(3)*co(0) );
      linear_term(1) = -( theta(0)*qa(1) + pow(theta(0),2)*theta(2)*theta(3)*co(1) );
      return linear_term;
    }
};

struct fa_s1 {
  template <typename T0>
  Eigen::Matrix<T0, Eigen::Dynamic, 1>
  inline operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
                    const std::vector<double>& delta,
                    const std::vector<int>& delta_int) const {
    // declare "data"
    int n = 2;
    VectorXd qe(2);
    qe << 1,1;

    // theta signiature: (gamma, sigma1sq,sigma2sq, alpha)
    Eigen::Matrix<T0, Eigen::Dynamic, 1> linear_constraint(n);
    linear_constraint(0) = qe(0);
    linear_constraint(1) = qe(1);
    return linear_constraint;
  }
};

struct fb_s1 {
  template <typename T0>
  T0
  inline operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
                    const std::vector<double>& delta,
                    const std::vector<int>& delta_int) const {
    // double s = 82.5;
    // double minus_s = -s;
    return -82.5;
  }
};

TEST(MathMatrix, quadratic_optimizer_s1) {
  VectorXd theta(4);
  theta << 1.5, 0.2, 0.8, 1.3;
  std::vector<double> delta;
  std::vector<int> delta_int;
  double tol = 1e-10;
  int n = 2;

  VectorXd x = quadratic_optimizer(fh_s1(), fv_s1(), fa_s1(), fb_s1(), theta,
                                   delta, delta_int, n, 0, tol);

  // std::cout << "x opt: " << x;
  EXPECT_NEAR(x(0), 56.5667, 0.0001);
  EXPECT_NEAR(x(1), 25.9333,0.0001);
}

TEST(MathMatrix, quadratic_optimizer_var_s1) {
  std::vector<double> delta;
  std::vector<int> delta_int;
  int n_x = 2;
  int n_theta = 4;
  // theta signiature: (gamma, sigma1sq,sigma2sq, alpha)

  MatrixXd J(2, 4);
  // Jacobian worked out by Shosh
//   J << 0.222222, -0.222222,
//   		-50.0667, 50.0667,
// 		12.9333, -12.9333,
// 	 	 -7.0, 7.0;

  // Jacobian worked with finite-diff (which is in agreement with autodiff)
  J << 0.222222, -50.0667, 12.9333, -7,
       -0.222222, 50.0667, -12.9333, 7;

  // to get results with finite differentiation
  // var diff = 1e-8;

  for (int k = 0; k < n_x; k++) {
    Matrix<var, Dynamic, 1> theta(n_theta);
    theta << 1.5, 0.2, 0.8, 1.3;

    Matrix<var, Dynamic, 1> x = quadratic_optimizer(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
                                                    theta, delta, delta_int, n_x);

	  EXPECT_NEAR(x(0).val(), 56.5667, 0.0001);
	  EXPECT_NEAR(x(1).val(), 25.9333, 0.0001);

    AVEC parameter_vec = createAVEC(theta(0), theta(1), theta(2), theta(3));
    VEC g;
    x(k).grad(parameter_vec, g);

    EXPECT_NEAR(J(k, 0), g[0], 0.0001);
    EXPECT_NEAR(J(k, 1), g[1], 0.0001);
    EXPECT_NEAR(J(k, 2), g[2], 0.0001);
    EXPECT_NEAR(J(k, 3), g[3], 0.0001);

    // Comparision with finite differentiation
    // for (int l = 0; l < n_theta; l++) {
    //   Matrix<var, Dynamic, 1> theta_lb = theta;
    //   Matrix<var, Dynamic, 1> theta_ub = theta;
    //   theta_lb(l) = theta(l) - diff;
    //   theta_ub(l) = theta(l) + diff;
    // 
    //   Matrix<var,Dynamic, 1> x_lb, x_ub;
    //   x_lb = quadratic_optimizer(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
    //                              theta_lb, delta, delta_int, n_x);
    //   x_ub = quadratic_optimizer(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
    //                              theta_ub, delta, delta_int, n_x);
    //   
    //   // compute the derivative of the first solution with respect
    //   // to the first parameter (i.e. element (1, 1) of the Jacobian).
    //   double derivative = (x_ub(k).val() - x_lb(k).val()) / (2 * diff.val());
    //   cout << "finite diff derivative: " << derivative << "\n";

      // << "with x_ub = " << x_ub << "\n"
      // << "and x_lb = " << x_lb << "\n";
    // }
  }

  // Check analytical solution (need to redeclare variables which
  // were declared in the FOR loop scope)
  // VectorXd theta(n_theta);
  // theta << 1.5, 0.2, 0.8, 1.3;
  // Matrix<double, Dynamic, 1> 
  //   x = quadratic_optimizer(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
  //                           theta, delta, delta_int, n_x);
  // 
  // VectorXd
  //   x_an = quadratic_optimizer_analytical(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
  //                                         theta, delta, delta_int,
  //                                         x);
  // std::cout << "\n" << "analytical form: " << x_an << "\n \n";
  // 
  // // finite diff test
  // for (int k = 0; k < n_theta; k++) {
  //   double diff_dbl = 1e-8;
  //   VectorXd theta_lb = theta, theta_ub = theta;
  //   theta_lb(k) = theta(k) - diff_dbl;
  //   theta_ub(k) = theta(k) + diff_dbl;
  // 
  //   VectorXd
  //     x_an_lb = quadratic_optimizer_analytical(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
  //                                              theta_lb, delta, delta_int,
  //                                              x);
  //   VectorXd
  //      x_an_ub = quadratic_optimizer_analytical(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
  //                                               theta_ub, delta, delta_int,
  //                                               x);
  // 
  //   double finite_diff = (x_an_ub(0) - x_an_lb(0)) / (2 * diff_dbl);
  //   std::cout << "finite diff: " << finite_diff << "\n";
  // }

  // Check autodiff on analytical function
  // {  // declare variables inside its own scope.
  //   stan::math::start_nested();
  //   std::cout << "\n" << "ANALYTICAL SOLUTIONS \n";
  //   Matrix<var, Dynamic, 1> theta_v(n_theta);
  //   theta_v << 1.5, 0.2, 0.8, 1.3;
  // 
  //   Matrix<var, Dynamic, 1>
  //     x_an_v = quadratic_optimizer_analytical(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
  //                                             theta_v, delta, delta_int,
  //                                             x);
  //   std::cout << "x_an_v: " << x_an_v(0).val() << " " << x_an_v(1).val() << "\n";
  // 
  //   AVEC parameter_vec = createAVEC(theta_v(0), theta_v(1), theta_v(2),
  //                                   theta_v(3));
  //   VEC g;
  //   x_an_v(0).grad(parameter_vec, g);
  //   std::cout << "g: " << g[0] << " " << g[1] << " " << g[2] << " " << g[3] << "\n";
  // 
  //   stan::math::recover_memory_nested();
  //   // This clearly shows autodiff and finite diff are in agreement.
  //   // Manually tested for both outputs, indexed 0 and 1.
  // }
}
 
// TEST(Math_matrix, quadratic_optimizer_cm1) { 
//   // Check autodiff for original function
//   std::vector<double> delta;
//   std::vector<int> delta_int;
//   int n_x = 2;
//   int n_theta = 4;
// 
//   {  // declare variables inside its own scope.
//     stan::math::start_nested();
//     std::cout << "\n" << "NUMERICAL SOLUTIONS \n";
//     Matrix<var, Dynamic, 1> theta_v(n_theta);
//     theta_v << 1.5, 0.2, 0.8, 1.3;
// 
//     Matrix<var, Dynamic, 1>
//       x_an_v = quadratic_optimizer(fh_s1(), fv_s1(), fa_s1(), fb_s1(),
//                                    theta_v, delta, delta_int,
//                                    n_x);
//     std::cout << "x_an_v: " << x_an_v(0).val() << " " << x_an_v(1).val() << "\n";
// 
//     AVEC parameter_vec = createAVEC(theta_v(0), theta_v(1), theta_v(2),
//                                     theta_v(3));
// 
//     // std::cout << "x before grad: " << x_an_v(0).adj() 
//     //           << " " << x_an_v(1).adj() << "\n";
// 
//     VEC g;
//     x_an_v(0).grad(parameter_vec, g);
//     std::cout << "g: " << g[0] << " " << g[1] << " " << g[2] << " " << g[3] << "\n";
//     // Does NOT return the same gradient as above !!!
// 
//     stan::math::recover_memory_nested();
//     // This clearly shows autodiff and finite diff are in agreement.
//     // Manually tested for both outputs, indexed 0 and 1.
//   }
// }


// Shosh unit test 2 start here
struct fh_s2 {
  template <typename T0>
  inline Eigen::Matrix<T0, Eigen::Dynamic, Eigen::Dynamic>
    operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
               const std::vector<double>& delta,
               const std::vector<int>& delta_int) const {
      // declare "data"
      int n = 3;
      // VectorXd qa(3);
      // qa << 1.5,2,4;
      // VectorXd qe(3);
      // qe << 1,1,0.8;
      // VectorXd co(3);
      // co << 5,10,7;
      // double s = 130;


      // theta signiature: (gamma, sigma1sq,sigma2sq,sigma3sq, alpha)
      // Eigen::Matrix<T0, Eigen::Dynamic, 2> H_mat(n,n) = MatrixXf::Zero();
      // H_mat = Identity(n, n);
      Eigen::Matrix<T0, Eigen::Dynamic, 2> H_mat(n,n);
      H_mat(0,0) = pow(theta(0), 2) * theta(1);
      H_mat(1,1) = pow(theta(0), 2) * theta(2);
      H_mat(2,2) = pow(theta(0), 2) * theta(3);
      H_mat(0,1) = 0;
      H_mat(1,0) = 0;
	  H_mat(0,2) = 0;
	  H_mat(1,2) = 0;
	  H_mat(2,0) = 0;
	  H_mat(2,1) = 0;
      return H_mat;
    }
};

struct fv_s2 {
  template <typename T0>
  inline Eigen::Matrix<T0, Eigen::Dynamic, 1>
    operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
               const std::vector<double>& delta,
               const std::vector<int>& delta_int) const {

     // declare "data"
     int n = 3;
     VectorXd qa(3);
     qa << 1.5,2,4;
     // VectorXd qe(3);
     // qe << 1,1,0.8;
     VectorXd co(3);
     co << 5,10,7;
     // double s = 130;

     // theta signiature: (gamma, sigma1sq,sigma2sq, alpha)
      Eigen::Matrix<T0, Eigen::Dynamic, 1> linear_term(n);
      linear_term(0) = -( theta(0)*qa(0) + pow(theta(0),2)*theta(1)*theta(3)*co(0) );
      linear_term(1) = -( theta(0)*qa(1) + pow(theta(0),2)*theta(2)*theta(3)*co(1) );
      linear_term(2) = -( theta(0)*qa(2) + pow(theta(0),2)*theta(3)*theta(3)*co(2) );
	  
      return linear_term;
    }
};

struct fa_s2 {
  template <typename T0>
  Eigen::Matrix<T0, Eigen::Dynamic, 1>
  inline operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
                    const std::vector<double>& delta,
                    const std::vector<int>& delta_int) const {
    // declare "data"
    int n = 3;
    // VectorXd qa(3);
    // qa << 1.5,2,4;
    VectorXd qe(3);
    qe << 1,1,0.8;
    // VectorXd co(3);
    // co << 5,10,7;
    // double s = 130;

    // theta signiature: (gamma, sigma1sq,sigma2sq, alpha)
    Eigen::Matrix<T0, Eigen::Dynamic, 1> linear_constraint(n);
    linear_constraint(0) = qe(0);
    linear_constraint(1) = qe(1);
    linear_constraint(2) = qe(2);
	
    return linear_constraint;
  }
};

struct fb_s2 {
  template <typename T0>
  T0
  inline operator()(const Eigen::Matrix<T0, Eigen::Dynamic, 1>& theta,
                    const std::vector<double>& delta,
                    const std::vector<int>& delta_int) const {

    return -130;
  }
};

TEST(MathMatrix, quadratic_optimizer_s2) {
  VectorXd theta(5);
  theta << 1.5, 0.2, 0.8, 1, 1.3;
  std::vector<double> delta;
  std::vector<int> delta_int;
  double tol = 1e-10;
  int n = 3;

  VectorXd x = quadratic_optimizer(fh_s2(), fv_s2(), fa_s2(), fb_s2(), theta,
                                   delta, delta_int, n, 0, tol);

 std::cout << "x opt: " << x;
 // EXPECT_NEAR(x(0).val(), 80.0196, 0.0001);
 // EXPECT_NEAR(x(1).val(), 31.7966, 0.0001);
 // EXPECT_NEAR(x(2).val(), 22.7298, 0.0001);

}

// TEST(MathMatrix, quadratic_optimizer_var_s2) {
//   std::vector<double> delta;
//   std::vector<int> delta_int;
//   int n_x = 3;
//   int n_theta = 5;
//   // theta signiature: (gamma, sigma1sq,sigma2sq,sigma3, alpha)
//
//   MatrixXd J(3, 5);
//   // Jacobian worked out by Shosh
//   J << 0.924045, -100.836, 17.0506, 7.9128, -9.9492,
//        -0.0467667, 66.6905, -19.2331, 1.9782, 6.2627,
// 	   -1.0966, 42.6819, 2.72809, -12.3638, 4.60813;
//
//   // to get results with finite differentiation
//   // var diff = 1e-8;
//
//   for (int k = 0; k < n_x; k++) {
//     Matrix<var, Dynamic, 1> theta(n_theta);
//     theta << 1.5, 0.2, 0.8, 1, 1.3;
//
//     Matrix<var, Dynamic, 1> x = quadratic_optimizer(fh_s2(), fv_s2(), fa_s2(), fb_s2(),
//                                                     theta, delta, delta_int, n_x);
//
// 	  EXPECT_NEAR(x(0).val(), 80.0196, 0.0001);
// 	  EXPECT_NEAR(x(1).val(), 31.7966, 0.0001);
// 	  EXPECT_NEAR(x(2).val(), 22.7298, 0.0001);
//
//     AVEC parameter_vec = createAVEC(theta(0), theta(1), theta(2), theta(3), theta(4));
//     VEC g;
//     x(k).grad(parameter_vec, g);
//
//     EXPECT_NEAR(J(k, 0), g[0], 0.0001);
//     EXPECT_NEAR(J(k, 1), g[1], 0.0001);
//     EXPECT_NEAR(J(k, 2), g[2], 0.0001);
//     EXPECT_NEAR(J(k, 3), g[3], 0.0001);
//     EXPECT_NEAR(J(k, 4), g[4], 0.0001);
//
//
// 	}
// }
//