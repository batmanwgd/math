#include <test/unit/math/test_ad.hpp>

TEST(mathMixCore, opratorMinusEqual) {
  auto f = [](const auto& x1, const auto& x2) {
    decltype(x1 + x2) y = x1;
    y -= x2;
    return y;
  };
  stan::test::expect_common_binary(f);
}
