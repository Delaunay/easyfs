
#include <cassert>

namespace easyfs {

enum class EntryKind: uint8_t {
    Invalid,

    #define ENTRY_KIND(KIND)\
        KIND(Meta)\
        KIND(File)\
        KIND(Directory)\
        KIND(Name)

    #define KIND(name) name,
        ENTRY_KIND(KIND)
    #undef KIND

    Size,
};

struct Entry {
    Entry(EntryKind k): kind(k) {}

    const EntryKind kind;
};

struct Name: Entry {
    // uint8_t kind;
    uint64_t size;
    const char* name;

    Name(): Entry(EntryKind::Name) {}
};

struct Meta: public Entry {
    // uint8_t kind;
    uint64_t size;
    uint64_t inode;

    Meta(): Entry(EntryKind::Meta) {}
};

struct File: public Entry {
    // uint8_t kind;
    uint64_t size;
    Name     name;
    uint64_t inode;
    uint64_t parent;

    uint64_t start;
    uint64_t size;

    File(): Entry(EntryKind::File) {}
};

/* 
 *
 */
struct Directory: public Entry {
    // uint8_t kind;
    uint64_t size;
    Name     name;
    uint64_t inode;
    uint64_t parent;

    Directory(): Entry(EntryKind::Directory) {}

    std::vector<Entry> entries;
};

//! return the size on disk of the given entry
uint64_t size(Entry* e);

//! return the number of bytes used to storage the size of a given entry
uint64_t size(EntryKind kind);

int write(FILE*);

int read(FILE* f) {
    EntryKind kind;
    assert(fread(&kind, sizeof(EntryKind), 1, f) == 1);

    uint64_t size;
    assert(fread(&size, size(kind), 1, f) == 1);

}



}