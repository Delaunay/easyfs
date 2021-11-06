#ifndef PROJECT_TEST_TESTS_ADD_HEADER
#define PROJECT_TEST_TESTS_ADD_HEADER

#include <chrono>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <unordered_map>

#include "hashtable.h"

using namespace easyfs;

TEST(fast_mod, valid) {
    int n = 1093;
    for(int i = 0; i < n * 2; i++) {
        EXPECT_EQ(fast_mod(i, n), i % n);
    }
}

TEST(fast_mod_p2, valid) {
    int n = 1024;
    for(int i = 0; i < n * 2; i++) {
        EXPECT_EQ(fast_mod_p2(i, n), i % n);
    }
}

#define REPEAT 10000
TEST(benchmark_mod, fast_mod) {
    int n = 1024;
    for(int j = 0; j < REPEAT; j++) {
        int volatile s = 0;
        for(int i = 0; i < n * 2; i++) {
            s += fast_mod(i, n);
        }
    }
}

TEST(benchmark_mod, reg_mod) {
    int n = 1024;
    for(int j = 0; j < REPEAT; j++) {
        int volatile s = 0;
        for(int i = 0; i < n * 2; i++) {
            s += reg_mod(i, n);
        }
    }
}

TEST(benchmark_mod, fast_mod_p2) {
    int n = 1024;
    for(int j = 0; j < REPEAT; j++) {
        int volatile s = 0;
        for(int i = 0; i < n * 2; i++) {
            s += fast_mod_p2(i, n);
        }
    }
}

TEST(hastable, base) {
    int                         value = 0;
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

TEST(hastable, rehash) {
    int                         value = 0;
    HashTable<std::string, int> v(6);

    std::vector<std::string> strings = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
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
    static uint64_t hash(std::string const &) noexcept { return 0; }
};

TEST(hastable, collision) {
    HashTable<std::string, int, BadHash> v(6);

    std::vector<std::string> strings = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
    };

    for(int i = 0; i < 12; i++) {
        EXPECT_TRUE(v.insert(strings[i], i));
    }

    EXPECT_EQ(v.size(), 12);
    // EXPECT_FLOAT_EQ(v.load_factor(), 1.f);

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

TEST(hastable, collision_deletion) {
    HashTable<std::string, int, BadHash> v(6);

    std::vector<std::string> strings = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
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

#define SIZE 10000000
inline std::vector<std::tuple<std::string, int>> const &pairs() {
    static std::vector<std::tuple<std::string, int>> data;

    if(data.size() > 0) {
        return data;
    }

    auto f = std::ifstream("../tests/Honor of Thieves.txt");

    std::default_random_engine         generator;
    std::uniform_int_distribution<int> distribution(0, SIZE);

    while(f) {
        std::string s;
        f >> s;
        data.push_back(std::make_tuple(s, data.size()));

        if(data.size() > SIZE) {
            break;
        }
    }

    return data;
}

template <typename T> struct StdHash {
    static uint64_t hash(T const &v) noexcept {
        std::hash<T> h;
        return h(v);
    }
};

TEST(benchmark, init) { pairs(); }

TEST(benchmark_insert, init) { pairs(); }

#define INIT 1024

#define START() auto start = std::chrono::system_clock::now()
#define DURATION() (std::chrono::duration<double>(end - start).count())
#define END(name)                                                                                  \
    auto end = std::chrono::system_clock::now();                                                   \
    std::cout << "[ TIME     ] " << name << " " << DURATION() * 1000.f << " ms\n";

TEST(benchmark_lookup_str, hashtable_siphash) {
    HashTable<std::string, int> v(INIT);
    int                         value;

    {
        START();
        for(int i = 0; i < 100; i++) {
            v = HashTable<std::string, int>(INIT);
            for(auto &p : pairs()) {
                v.insert(std::get<0>(p), std::get<1>(p));
            }
        }
        END("insert");
    }

    std::cout << "[ Collsion ] " << v.collided() << std::endl;

    {
        START();
        for(int i = 0; i < 100; i++) {
            for(auto &p : pairs()) {
                v.get(std::get<0>(p), value);
            }
        }
        END("lookup");
    }
}

TEST(benchmark_lookup_str, hashtable_stdhash) {
    HashTable<std::string, int, StdHash<std::string>> v(INIT);
    int                                               value;

    {
        START();
        for(int i = 0; i < 100; i++) {
            v = HashTable<std::string, int, StdHash<std::string>>(INIT);
            for(auto &p : pairs()) {
                v.insert(std::get<0>(p), std::get<1>(p));
            }
        }
        END("insert");
    }

    std::cout << "[ Collsion ] " << v.collided() << std::endl;

    {
        START();
        for(int i = 0; i < 100; i++) {
            for(auto &p : pairs()) {
                v.get(std::get<0>(p), value);
            }
        }
        END("lookup");
    }
}

TEST(benchmark_lookup_int, hashtable_siphash) {
    HashTable<int, std::string> v(INIT);
    std::string                 value;

    {
        START();
        for(int i = 0; i < 100; i++) {
            v = HashTable<int, std::string>(INIT);
            for(auto &p : pairs()) {
                v.insert(std::get<1>(p), std::get<0>(p));
            }
        }
        END("insert");
    }

    std::cout << "[ Collsion ] " << v.collided() << std::endl;

    {
        START();
        for(int i = 0; i < 100; i++) {
            for(auto &p : pairs()) {
                v.get(std::get<1>(p), value);
            }
        }
        END("lookup");
    }
}

TEST(benchmark_lookup_int, hashtable_stdhash) {
    HashTable<int, std::string, StdHash<int>> v(INIT);
    std::string                               value;

    {
        START();
        for(int i = 0; i < 100; i++) {
            v = HashTable<int, std::string, StdHash<int>>(INIT);
            for(auto &p : pairs()) {
                v.insert(std::get<1>(p), std::get<0>(p));
            }
        }
        END("insert");
    }

    std::cout << "[ Collsion ] " << v.collided() << std::endl;

    {
        START();
        for(int i = 0; i < 100; i++) {
            for(auto &p : pairs()) {
                v.get(std::get<1>(p), value);
            }
        }
        END("lookup");
    }
}

template <typename T> struct Siphash {
    std::size_t operator()(const T &k) const { return Hash<T>::hash(k); }
};

TEST(benchmark_lookup_str, unordered_map_siphash) {
    int                                                        value;
    std::unordered_map<std::string, int, Siphash<std::string>> v(INIT);
    {
        START();
        for(int i = 0; i < 100; i++) {
            v = std::unordered_map<std::string, int, Siphash<std::string>>(INIT);
            for(auto &p : pairs()) {
                v[std::get<0>(p)] = std::get<1>(p);
            }
        }
        END("insert");
    }

    {
        START();
        for(int i = 0; i < 100; i++) {
            for(auto &p : pairs()) {
                value = v[std::get<0>(p)];
            }
        }
        END("lookup");
    }
}

TEST(benchmark_lookup_str, unordered_map_stdhash) {
    int                                  value;
    std::unordered_map<std::string, int> v(INIT);
    {
        START();
        for(int i = 0; i < 100; i++) {
            v = std::unordered_map<std::string, int>(INIT);
            for(auto &p : pairs()) {
                v[std::get<0>(p)] = std::get<1>(p);
            }
        }
        END("insert");
    }

    {
        START();
        for(int i = 0; i < 100; i++) {
            for(auto &p : pairs()) {
                value = v[std::get<0>(p)];
            }
        }
        END("lookup");
    }
}

TEST(benchmark_lookup_int, unordered_map_stdhash) {
    std::unordered_map<int, std::string> v(INIT);
    std::string                          value;

    {
        START();
        for(int i = 0; i < 100; i++) {
            v = std::unordered_map<int, std::string>(INIT);
            for(auto &p : pairs()) {
                v[std::get<1>(p)] = std::get<0>(p);
            }
        }
        END("insert");
    }

    {
        START();
        for(int i = 0; i < 100; i++) {
            for(auto &p : pairs()) {
                value = v[std::get<1>(p)];
            }
        }
        END("lookup");
    }
}

TEST(benchmark_lookup_int, unordered_map_siphash) {
    std::unordered_map<int, std::string, Siphash<int>> v(INIT);
    std::string                                        value;

    {
        START();
        for(int i = 0; i < 100; i++) {
            v = std::unordered_map<int, std::string, Siphash<int>>(INIT);
            for(auto &p : pairs()) {
                v[std::get<1>(p)] = std::get<0>(p);
            }
        }
        END("insert");
    }

    {
        START();
        for(int i = 0; i < 100; i++) {
            for(auto &p : pairs()) {
                value = v[std::get<1>(p)];
            }
        }
        END("lookup");
    }
}

#endif
