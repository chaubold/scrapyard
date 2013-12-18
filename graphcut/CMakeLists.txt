PROJECT(graphcut)

cmake_minimum_required(VERSION 2.8)

# BUILDEM directory as parameter
SET(BUILDEM_DIR "None" CACHE TYPE FILEPATH) 

INCLUDE_DIRECTORIES(${BUILDEM_DIR}/include)
LINK_DIRECTORIES(${BUILDEM_DIR}/lib)

ADD_EXECUTABLE(graphcut main.cpp)
TARGET_LINK_LIBRARIES(graphcut emon vigraimpex)
