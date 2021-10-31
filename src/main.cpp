#include "logger.h"
#include "version.h"
#include "easyfs.h"

#include "hashtable.h"

NEW_EXCEPTION(HugeError);



namespace easyfs 
{

uint64_t size(Entry* e) {
    switch (e->kind)
    {
        #define KIND(name)\
            case EntryKind::name: {\
                auto n = static_cast<name*>(e);\
                return reinterpret_cast<uint64_t>(n->size);\
            }

        ENTRY_KIND(KIND)
        #undef KIND
    }

    // unreachable
    return 0;
}

uint64_t size(EntryKind k) {
    switch (k)
    {
        #define KIND(name)\
            case EntryKind::name: {\
                return sizeof(name::size);\
            }

        ENTRY_KIND(KIND)
        #undef KIND
    }

    // unreachable
    return 0;
}


}


int main(){
    info("Testing throwing function");
    info("version hash  : {}", _HASH);
    info("version date  : {}", _DATE);
    info("version branch: {}", _BRANCH);

    easyfs::HashTable<int, int> b;

    return 0;
}
