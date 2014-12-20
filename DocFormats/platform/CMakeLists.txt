include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(headers)
include_directories(../unittest)

set(GroupHeaders
    headers/DFPlatform.h)

set(GroupSrc
    src/Apple.c
    src/Linux.c
    src/Unix.c
    src/Win32.c)

set(GroupTests
    tests/PlatformTests.c)

add_library(platform OBJECT
    ${GroupHeaders}
    ${GroupSrc}
    ${GroupTests})

source_group(headers FILES ${GroupHeaders})
source_group(src FILES ${GroupSrc})
source_group(tests FILES ${GroupTests})
