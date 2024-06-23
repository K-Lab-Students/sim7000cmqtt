#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(sample_test_case0, sample_test
)
{
EXPECT_EQ(1, 1);
}

TEST(sample_test_case1, sample_test
)
{
EXPECT_EQ(1, 1);
}

TEST(sample_test_case2, sample_test
)
{
EXPECT_EQ(1, 1);
}

TEST(sample_test_case3, sample_test
)
{
EXPECT_EQ(1, 1);
}
TEST(sample_test_case4, sample_test
)
{
EXPECT_EQ(1, 1);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	::testing::InitGoogleMock(&argc, argv);

	return RUN_ALL_TESTS();
}