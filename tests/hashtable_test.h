#ifndef PROJECT_TEST_TESTS_ADD_HEADER
#define PROJECT_TEST_TESTS_ADD_HEADER

#include <gtest/gtest.h>
#include <iostream>

#include "hashtable.h"

using namespace easyfs;

TEST(hastable, base)
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

    v.rehash();

    EXPECT_EQ(v.size(), 2);
    EXPECT_TRUE(v.get("b", value));
    EXPECT_EQ(value, 4);
    EXPECT_TRUE(v.get("a", value));
    EXPECT_EQ(value, 3);

    EXPECT_TRUE(v.remove("b"));
    EXPECT_EQ(v.size(), 1);

}

TEST(hastable, rehash)
{
    int value = 0;
    HashTable<std::string, int> v(6);

    std::vector<std::string> strings = {
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
    };

    for(int i = 0; i < 12; i++) {
        EXPECT_TRUE(v.insert(strings[i], i));
    }

    EXPECT_EQ(v.size(), 12);

    for(int i = 0; i < 12; i++) {
        int value = 0;
        EXPECT_TRUE(v.get(strings[i], value));
        EXPECT_EQ(value, i);
    }

    for(int i = 0; i < 12; i++) {
        EXPECT_TRUE(v.remove(strings[i]));
        EXPECT_FALSE(v.get(strings[i], value));
        EXPECT_EQ(value, 0);
    }
}


struct BadHash {
    static uint64_t hash(std::string const&) noexcept {
        return 0;
    }
};


TEST(hastable, collision)
{
    HashTable<std::string, int, BadHash> v(6);

    std::vector<std::string> strings = {
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
    };

    for(int i = 0; i < 12; i++) {
        EXPECT_TRUE(v.insert(strings[i], i));
    }
    
    EXPECT_EQ(v.size(), 12);
    EXPECT_FLOAT_EQ(v.load_factor(), 1.f);

    for(int i = 0; i < 12; i++) {
        int value = 0;
        EXPECT_TRUE(v.get(strings[i], value));
        EXPECT_EQ(value, i);
    }

    for(int i = 0; i < 12; i++) {
        int value = 0;
        EXPECT_TRUE(v.remove(strings[i]));
        EXPECT_FALSE(v.get(strings[i], value));
        EXPECT_EQ(value, 0);
    }
}

TEST(hastable, collision_deletion)
{
    HashTable<std::string, int, BadHash> v(6);

    std::vector<std::string> strings = {
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
    };

    for(int i = 0; i < 6; i++) {
        EXPECT_TRUE(v.insert(strings[i], i));
    }

    // remove in the middle
    int value = 0;
    EXPECT_TRUE(v.remove(strings[2]));
    EXPECT_FALSE(v.get(strings[2], value));
    EXPECT_EQ(value, 0);

    for(int i = 6; i < 12; i++) {
        EXPECT_TRUE(v.insert(strings[i], i));
    }

    EXPECT_LT(v.load_factor(), 1.f);
}
#endif
