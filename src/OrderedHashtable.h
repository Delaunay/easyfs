
#ifndef EASYFS_ORDERED_HASHTABLE_HEADER
#define EASYFS_ORDERED_HASHTABLE_HEADER

#include "hashtable.h"


namespace easyfs {

template <typename Key, typename Value, typename H = Hash<Key>>
struct OrderedHashTable {
    struct Item {
        Item(Key const &k, Value const &v) : key(k), value(v), used(true) {}

        Key   key;
        Value value;
        bool  used : 1    = false;
    }

    bool remove(const Key &name) {
        uint64_t n = 0;
        if (index.get(name, n)) {
            v = data[n].used = false;
            index.remove(name);
            return true;
        }

        return false;
    }

    bool get(const Key &name, Value &v) const {
        uint64_t n = 0;
        if (index.get(name, n)) {
            v = data[n].value;
        }
        return false;
    }

    bool upsert(const Key &name, const Value &value) {
        return _insert(name, value, true);
    }

    bool insert(const Key &name, const Value &value) {
        return _insert(name, value, false);
    }

    int size() const { return index.size(); }

private:
    bool _insert(const Key &name, const Value &value, bool update) {
        uint64_t n = 0;

        if (!index.get(name, n)) {
            n = data.size();
            auto Item& value = data.emplace_back(name, value);
            index.insert(Ref(&value), n);
            return true;
        }

        if (update) {
            data[n].value = value;
            return true;
        }

        return false;
    }

    struct Ref {
        bool operator== (Ref const& a) const {
            if (item == nullptr)
                return false;

            if (a.item == nullptr)
                return false;

            return item->key == a.item->key;
        }

        Item* item;
    };

    HashTable<Ref, uint64_t> index;
    std::vector<Item> data;
};

}

#endif