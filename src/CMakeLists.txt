# better way ?
INCLUDE_DIRECTORIES(.)

# Source files
# ====================================
#           (Glob is not recommended)
#   file(GLOB APL_SRC *.cc)
#   file(GLOB_RECURSE APL_SRC *.cc)

SET(EASYFS_HDS
    logger.h
    siphash.h
)

SET(EASYFS_SRC
    logger.cpp
    siphash.cpp
)

# main library (prevent recompilation when building tests)
ADD_LIBRARY(easyfs ${EASYFS_SRC} ${EASYFS_HDS})
TARGET_LINK_LIBRARIES(easyfs spdlog::spdlog)

#  main executable
# ==========================

ADD_EXECUTABLE(main main.cpp)
TARGET_LINK_LIBRARIES(main spdlog::spdlog easyfs)
