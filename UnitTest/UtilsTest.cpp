

#include "Utils.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(UtilsTest, GenerateId)
{
    Utils::ID id = Utils::GenerateId();
    EXPECT_GT(id, 0);
}
