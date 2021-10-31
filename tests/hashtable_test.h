#ifndef PROJECT_TEST_TESTS_ADD_HEADER
#define PROJECT_TEST_TESTS_ADD_HEADER

#include <gtest/gtest.h>
#include <iostream>

#include "hashtable.h"

using namespace easyfs;

TEST(hastable, insert_get)
{
    int value = 0;
    HashTable<std::string, int> v;

    EXPECT_TRUE(v.insert("a", 2));
    EXPECT_FALSE(v.insert("a", 3));

    EXPECT_TRUE(v.get("a", value));
    EXPECT_EQ(value, 2);

    EXPECT_EQ(v.size(), 1);

    EXPECT_TRUE(v.upsert("a", 3));
    EXPECT_TRUE(v.get("a", value));
    EXPECT_EQ(value, 3);

    EXPECT_EQ(v.size(), 1);

    EXPECT_TRUE(v.insert("b", 4));
    EXPECT_EQ(v.size(), 2);
    EXPECT_TRUE(v.get("b", value));
    EXPECT_EQ(value, 4);
    EXPECT_TRUE(v.get("a", value));
    EXPECT_EQ(value, 3);

    EXPECT_TRUE(v.remove("b"));
    EXPECT_EQ(v.size(), 1);
}


#endif
