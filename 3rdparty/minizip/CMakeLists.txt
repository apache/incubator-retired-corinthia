cmake_minimum_required(VERSION 2.8)
include_directories()
include_directories(.)
add_library(minizip ioapi.c unzip.c zip.c)
