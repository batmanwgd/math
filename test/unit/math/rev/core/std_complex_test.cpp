#include <stan/math/rev/scal.hpp>
#include <stan/math/rev/core/std_complex.hpp>
#include <gtest/gtest.h>

// For a definition of the spec:
// https://en.cppreference.com/w/cpp/numeric/complex
class MathRev : public testing::Test {
 public:
  void SetUp() { stan::math::recover_memory(); }
};

TEST_F(MathRev, complex_constructor) {
  std::complex<stan::math::var> x;
  EXPECT_EQ(0, stan::math::ChainableStack::instance().var_stack_.size());
  stan::math::recover_memory();

  std::complex<stan::math::var> y{0};
  EXPECT_EQ(1, stan::math::ChainableStack::instance().var_stack_.size());
  stan::math::recover_memory();

  std::complex<stan::math::var> z{0, 0};
  EXPECT_EQ(2, stan::math::ChainableStack::instance().var_stack_.size());
  stan::math::recover_memory();
}

TEST_F(MathRev, assignment) {  // operator=
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, real_component) { ADD_FAILURE() << "not yet implemented"; }

TEST_F(MathRev, imag_component) { ADD_FAILURE() << "not yet implemented"; }

TEST_F(MathRev, member_operators) {
  // operator+=
  // operator-=
  // operator/=
  // operator*=
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, unary_operators) {
  // operator+
  // operator-
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, arithmetic) {
  // operator+
  // operator-
  // operator*
  // operator/
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, comparison) {
  // complex and scalar; assume var and double as scalar?
  // operator==
  // operator!=
}

TEST_F(MathRev, serialize_deserialize) {
  // operator<<
  // operator>>
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, real) {
  // real
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, imag) {
  // imag
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, abs) {
  // abs
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, arg) {
  // arg
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, norm) {
  // norm
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, conj) {
  // conj
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, proj) {
  // projg
  ADD_FAILURE() << "not yet implemented";
}

TEST_F(MathRev, polar) {
  // polar
  ADD_FAILURE() << "not yet implemented";
}