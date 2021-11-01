
#ifndef EASYFS_HASHTABLE_HEADER
#define EASYFS_HASHTABLE_HEADER

#include <tuple>
#include <vector>
#include <sstream>
#include <functional>

#include "siphash.h"

namespace easyfs {

const char* SALT = "dW8(2!?GTfDFJ@Le";

template<typename T>
struct Hash {
    static uint64_t hash (T const& k) noexcept{
        uint64_t value = 0;
        siphash(
            (const void*)&k, 
            (size_t) sizeof(T), 
            SALT, 
            (uint8_t*)&value, 
            (size_t) sizeof(uint64_t)
        );
        return value;
    }
};

template<>
struct Hash<std::string> {
    static uint64_t hash (std::string const& k) noexcept{
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
};

// TODO: rewrite this with a Storage for Key-Values
// and a HashTable for Key->Index 
// the HasTable will be bigger than the number of values so it is useful
// to make it as small as possible
// but then it is not exactly linear probing
template<typename Key, typename Value, typename H = Hash<Key>>
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
        bool used: 1 = false;
        bool deleted: 1 = true;

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
    HashTable(int reserve = 128):
        _storage(Storage(reserve))
    {}

    float load_factor() const {
        return float(used) / float(_storage.size());
    }

    bool get(const Key& name, Value& v) const {
        Item const* item = _find(name);

        if (item == nullptr) {
            return false;
        }

        v = item->value;
        return true;
    }

    bool insert(const Key& name, const Value& value) {
        return track_insert(name, value, false);
    }

    bool upsert(const Key& name, const Value& value) {
        return track_insert(name, value, true);
    }

    bool remove(const Key& name) {
        Item* item = (Item*) _find(name);

        if (item == nullptr){
            return false;
        }

        item->deleted = true;
        used -= 1;
        return true;
    }

    int size() const {
        return used;
    }
    
    std::string __str__() const {
        std::stringstream ss;
        bool comma = false;
        bool all = false;
        ss << "{";
        for (auto i = 0; i < _storage.size(); i++) {

            auto& item = _storage[i];

            if (item.used && !item.deleted) {
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

    // Rebuild the hash using a bigger storage
    void rehash(float multiplier = 2.f) {
        Storage storage(int(_storage.size() * multiplier));

        for(auto& item: _storage) {
            if (!item.used || item.deleted) {
                continue;
            }

            switch(insert(storage, item, false)) {
                case InsertStatus::Duplicate:
                case InsertStatus::FailureLinearProbing:
                    assert(false && "Unreachalbe");
            }
        }

        _storage = storage;
    }

private:
    // this cannot be exposed because it returns a pointer
    // that could change once rehash is called
    Item const* _find(const Key& name) const {
        uint64_t i = H::hash(name) % _storage.size();

        // linear probing
        for(int offset = 0; offset < _storage.size(); offset++) {
            auto& item = _storage[(i + offset) % _storage.size()];

            // item was not used, so we know there was no collision
            // and no linear probe insert was done after this
            if (!item.used) {
                return nullptr;
            }

            // Item is not deleted and key matches
            if (!item.deleted && item.key == name) {
                return &item;
            }
        }
    
        return nullptr;
    }

    // insert a key value pair
    // if the key already exist does not insert
    bool track_insert(const Key& name, const Value& value, bool upsert) {
        if (load_factor() >= 1) {
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
                if (insert(_storage, item, upsert) <= InsertStatus::Success) {
                    used += 1;
                    return true;
                }
                return false;
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
        uint64_t i = H::hash(inserted_item.key) % data.size();
        
        for(int offset = 0; offset < data.size(); offset++) {
            Item& item = data[(i + offset) % data.size()];

            if (!item.used || item.deleted) {
                item = inserted_item;

                int p = offset > 0;
                return InsertStatus(int(InsertStatus::Insert) * (1 - p) + int(InsertStatus::LinearProbing) * p);
            }

            if (item.key == inserted_item.key) {
                if (upsert) {
                    item.value = inserted_item.value;
                    return InsertStatus::Update;
                }

                return InsertStatus::Duplicate;
            }
        }

        // unreachable
        return InsertStatus::FailureLinearProbing;
    }

    int used = 0;
    int collision = 0;
    Storage _storage;
};
}

#endif 
