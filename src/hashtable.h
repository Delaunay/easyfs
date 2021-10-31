
#ifndef EASYFS_HASHTABLE_HEADER
#define EASYFS_HASHTABLE_HEADER

#include <tuple>
#include <vector>
#include <sstream>

#include "siphash.h"


namespace easyfs {

const char* SALT = "dW8(2!?GTfDFJ@Le";

template<typename T>
uint64_t hash(T const& k) {
    uint64_t value = 0;
    siphash(
        (const void*)&k, 
        (size_t) sizeof(void*), 
        SALT, 
        (uint8_t*)&value, 
        (size_t) sizeof(uint64_t)
    );
    return value;
}

template<>
uint64_t hash(std::string const& k) {
    uint64_t value = 0;
        siphash(
        (const void*)&k[0], 
        (size_t) k.size(), 
        SALT, 
        (uint8_t*)&value, 
        (size_t) sizeof(uint64_t)
    );

    return value;
}


template<typename Key, typename Value>
struct HashTable {
private:
    struct Item {
        Item():
            key(Key())
        {}

        Item(Key const& k, Value const& v):
            key(k), value(v), used(true)
        {}

        Key const key;
        Value value;
        bool used = false;
        bool deleted = true;

        Item& operator= (Item const& i) {
            Key& mutkey = (Key&)key;
            mutkey = i.key;
            value = i.value;
            used = true;
            deleted = false;
            return *this;
        }
    };

    using Storage = std::vector<Item>;

public:
    std::string __str__() const {
        std::stringstream ss;
        bool comma = false;
        bool all = false;
        ss << "{";
        for (auto i = 0; i < _storage.size(); i++) {

            auto& item = _storage[i];

            if (item.used) {
                if (comma) {
                    ss << ", ";
                }

                ss << item.key << ": " << item.value;
                comma = true;
            }
        }
        ss << "}";
        return ss.str();
    }

    HashTable(int reserve = 128):
        _storage(Storage(reserve))
    {}

    int load_factor() const {
        return used / _storage.size();
    }

    bool get(const Key& name, Value& v) const {
        uint64_t i = hash(name) % _storage.size();
        auto item = _storage[i];

        // if the item was used or is deleted we need to do linear probing
        if (item.used || item.deleted) {
            // linear probing
            int offset = 1;
            while (offset < _storage.size()) {
                // skip deleted entries
                if (!item.deleted && item.key == name) {
                    v = item.value;
                    return true;
                }

                item = _storage[i + offset];
                offset += 1;
            }
        }

        return false;
    }

    // Rebuild the hash using a bigger storage
    void rehash(float multiplier = 2.f) {
        Storage storage(int(_storage.size() * multiplier));

        for(auto& item: _storage) {
            switch (insert(storage, item, false)) {
                case InsertStatus::Duplicate:
                case InsertStatus::FailureLinearProbing:
                    assert(false && "This should be unreachable in this context");
            }
        }

        _storage = storage;
    }

    bool insert(const Key& name, const Value& value) {
        return _insert(name, value, false);
    }

    bool upsert(const Key& name, const Value& value) {
        return _insert(name, value, true);
    }

    bool remove(const Key& name) {
        uint64_t i = hash(name) % _storage.size();
        auto& item = _storage[i];

        if (item.used || item.deleted) {
            // linear probing
            int offset = 1;
            while (offset < _storage.size()) {
                if (!item.deleted && item.key == name) {
                    item.deleted = true;
                    used -= 1;
                    return true;
                }

                item = _storage[i + offset];
                offset += 1;
            }
        }

        return false;
    }

    int size() const {
        return used;
    }

    private:
    // insert a key value pair
    // if the key already exist does not insert
    bool _insert(const Key& name, const Value& value, bool upsert) {
        if (load_factor() > 1) {
            rehash();
        }

        auto item = Item(name, value);
        auto status = insert(_storage, item, upsert);

        switch (status) {
            case InsertStatus::LinearProbing:
                collision += 1;
            case InsertStatus::Insert: 
                used += 1;
            case InsertStatus::Update:
            case InsertStatus::Success:
                return true;

            case InsertStatus::FailureLinearProbing: {
                rehash();
                return insert(_storage, item, upsert) <= InsertStatus::Success;
            }
            case InsertStatus::Duplicate:
                return false;
        }

        return false;
    }
   
    enum class InsertStatus: uint8_t {
        Insert,
        LinearProbing,
        Update,
        Success,
        FailureLinearProbing,
        Duplicate,
    };

    static InsertStatus insert(Storage& data, Item const& inserted_item, bool upsert) {
        uint64_t i = hash(inserted_item.key) % data.size();
        Item& item = data[i];

        // No collision
        if (!item.used) {
            item = inserted_item;
            return InsertStatus::Insert;
        }

        // Item already there
        if (item.key == inserted_item.key) {
            if (!upsert) {
                return InsertStatus::Duplicate;
            }
            item = inserted_item;
            return InsertStatus::Update;
        } else {
            // Linear probing
            int offset = 1;
            while (offset < data.size()) {

                auto& item = data[(i + offset) % data.size()];

                if (!item.used) {
                    item = inserted_item;
                    return InsertStatus::LinearProbing;
                }

                offset += 1;
            }

            return InsertStatus::FailureLinearProbing;
        }
    }

    int used = 0;
    int collision = 0;
    Storage _storage;
};
}

#endif 
