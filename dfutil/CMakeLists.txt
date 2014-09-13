cmake_minimum_required(VERSION 2.8)

set(SOURCES
    BDTTest.c
    Commands.c
    DFChanges.c
    Plain.c
    Test.c
    TestCase.c
    TestFunctions.c
    TextPackage.c
    main.c
)

include_directories(SYSTEM ${INCLUDE_DIRS})
include_directories(.)
include_directories(../DocFormats)
include_directories(../DocFormats/css)
include_directories(../DocFormats/html)
include_directories(../DocFormats/latex)
include_directories(../DocFormats/lib)
include_directories(../DocFormats/names)
include_directories(../DocFormats/odf)
include_directories(../DocFormats/word)
include_directories(../DocFormats/word/formatting)
include_directories(../DocFormats/word/lenses)
include_directories(../DocFormats/xml)
include_directories(/usr/include/libxml2)
link_directories(${LIB_DIRS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    set(SOURCES ${SOURCES} FunctionTests.m StringTests.m)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 

add_executable(dfutil ${SOURCES})
target_link_libraries(dfutil DocFormats ${LIBS})
