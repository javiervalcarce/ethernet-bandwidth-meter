########################################################################################################################
# CMakefile para compilar la librería thdanalyzer y los programas de ejemplo asociados.
#
# Para compilar hay que instalar previamente la herramienta cmake 2.8+, disponible para Linux, Windows y Mac, una vez
# instalada, hacemos:
#
# mkdir -p build
# cd build
# cmake ..
# make
########################################################################################################################

PROJECT(libbandwithmeter)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.6)
set (CMAKE_LEGACY_CYGWIN_WIN32 0)

set (LIBBANDWIDTHMETER_VERSION_MAJOR 0)
set (LIBBANDWIDTHMETER_VERSION_MINOR 1)
set (LIBBANDWIDTHMETER_VERSION_MICRO 0)
set (LIBBANDWIDTHMETER_VERSION_STRING ${LIBBANDWIDTHMETER_VERSION_MAJOR}.${LIBBANDWIDTHMETER_VERSION_MINOR}.${LIBBANDWIDTHMETER_VERSION_MICRO})

set (libbandwidthmeter_SRCS src/send.cpp src/recv.cpp)
set (test_bwmeter_SRCS src/test_bwmeter.cpp)

set (CMAKE_VERBOSE_MAKEFILE on)

# Directorios de ficheros cabecera y de bibliotecas (opciones -I y -L respectivamente)
include_directories (src/include)

# opciones de compilación (CFLAGS)
add_definitions ("-Wall -g")

add_library(bandwidthmeter SHARED ${libbandwidthmeter_SRCS})
set_target_properties(bandwidthmeter PROPERTIES VERSION ${LIBBANDWIDTHMETER_VERSION_STRING})

add_executable(test_bwmeter ${test_bwmeter_SRCS})
target_link_libraries(test_bwmeter bandwidthmeter m pthread)
