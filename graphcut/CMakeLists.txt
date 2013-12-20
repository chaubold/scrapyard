PROJECT(graphcut)

cmake_minimum_required(VERSION 2.8)

# BUILDEM directory as parameter
SET(BUILDEM_DIR "None" CACHE TYPE FILEPATH) 

# Qt stuff
FIND_PACKAGE(Qt4)
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

SET(HEADERS_QT MainWindow.h)
SET(FORMS_QT MainWindow.ui)
QT4_WRAP_CPP(graphcut_HEADERS_MOC ${HEADERS_QT})
QT4_WRAP_UI(graphcut_FORMS ${FORMS_QT})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# compilation stuff
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

INCLUDE_DIRECTORIES(${BUILDEM_DIR}/include)
LINK_DIRECTORIES(${BUILDEM_DIR}/lib)

ADD_EXECUTABLE(graphcut main.cpp ImageGraph.cpp ImageGraph.h PixelMask.cpp PixelMask.h MainWindow.cpp ${graphcut_HEADERS_MOC} ${graphcut_FORMS})
TARGET_LINK_LIBRARIES(graphcut emon vigraimpex ${QT_LIBRARIES})
