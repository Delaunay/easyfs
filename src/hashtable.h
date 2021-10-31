
#ifndef EASYFS_HASHTABLE_HEADER
#define EASYFS_HASHTABLE_HEADER

#include <tuple>
#include <vector>

#include "siphash.h"

const char* SALT = "12345678901234567890";

template<typename T>
uint64_t hash(T const& k) {
    uint64_t value = 0;
    siphash((void*)&k, sizeof(void*), SALT, (uint8_t*)&value, sizeof(uint64_t));
    return value;
}

template<>
uint64_t hash(std::string const& k) {
    uint64_t value = 0;
    siphash((void*)k[0], k.size(), SALT, (uint8_t*)&value, sizeof(uint64_t));
    return value;
}


template<typename Key, typename Value>
struct HashTable {
private:
    using Item = std::tuple<const Key, Value, bool>
    using Storage = std::vector<Item>;

public:
    HashTable(int reserve = 128):
        _storage(Storage(reserve))
    {}

    int load_factor() const {
        return used / _storage.size();
    }

    bool get(const Key& name, Value& v) {
        uint64_t i = hash(name) % _storage.size();
        auto& item = _storage[i];

        if (!std::get<2>(item)) {
            v = std::get<1>(item);
            return true;
        }

        return false;
    }

    // Rebuild the hash using a bigger storage
    void rehash(float multiplier = 2.f) {
        Storage storage(int(_storage.size() * multiplier));

        for(auto& item: _storage) {
            switch (insert(storage, item)) {
                case InsertStatus::Duplicate:
                case InsertStatus::FailureLinearProbing:
                    assert(false && "This should be unreachable in this context");
            }
        }

        _storage = storage;
    }

    // insert a key value pair
    // if the key already exist does not insert
    bool insert(const Key& name, const Value& value) {
        if (load_factor() > 1) {
            rehash();
        }

        auto item = std::make_tuple<Item>(name, value, true)
        auto status = insert(_storage, item);

        switch (status) {
            case InsertStatus::Insert: 
            case InsertStatus::LinearProbing:
            case InsertStatus::Success:
                return true;

            case InsertStatus::Failure: {
                rehash();
                return insert(_storage, item) == InsertStatus::Success;
            }
            case InsertStatus::Duplicate:
                return false;
        }

        return false;
    }

private:    
    enum class InsertStatus: uint8_t {
        Insert,
        LinearProbing,
        Success,
        FailureLinearProbing,
        Duplicate,
    }

    InsertStatus insert(Storage data, Item const& inserted_item) {
        uint64_t i = hash(name) % data.size();
        auto& item = data[i];

        // No collision
        if (!std::get<2>(item)) {
            item = inserted_item;
            used += 1;
            return InsertStatus::Insert;
        }

        // Item already there
        if (std::get<0>(item) == std::get<0>(inserted_item)) {
            return InsertStatus::Duplicate;
        } else {
            collision += 1;

            // Linear probing
            int offset = 1;
            while (offset < data.size()) {

                auto& item = data[(i + offset) % data.size()];

                if (!std::get<2>(item)) {
                    item = inserted_item;
                    used += 1;
                    return InsertStatus::LinearProbing;
                }

                offset += 1;
            }

            return InsertStatus::Failure;
        }
    }

    int used = 0;
    int collision = 0;
    Storage _storage;
};

#endif
