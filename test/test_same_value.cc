#include <math.h>

#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include <es/parser/parser.h>
#include <es/types/conversion.h>
#include <es/types/property_descriptor_object_conversion.h>
#include <es/utils/helper.h>

using namespace es;

TEST(TestSameValue, Number) {
  {
    std::vector<std::pair<double, double>> pairs = {
      {0, 0}, {nan(""), nan("")}, {1.23, 1.23}
    };
    for (auto pair : pairs) {
      EXPECT_EQ(true, SameValue(new Number(pair.first), new Number(pair.second)));
    }
  }
  
  {
    std::vector<std::pair<double, double>> pairs = {
      {0.0, -0.0},
    };
    for (auto pair : pairs) {
      EXPECT_EQ(false, SameValue(new Number(pair.first), new Number(pair.second)));
    }
  }
}
