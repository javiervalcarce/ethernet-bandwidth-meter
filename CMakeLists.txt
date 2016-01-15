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

PROJECT(libteletraffic)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.6)
set (CMAKE_LEGACY_CYGWIN_WIN32 0)

set (LIBTELETRAFFIC_VERSION_MAJOR 0)
set (LIBTELETRAFFIC_VERSION_MINOR 1)
set (LIBTELETRAFFIC_VERSION_MICRO 0)
set (LIBTELETRAFFIC_VERSION_STRING ${LIBTELETRAFFIC_VERSION_MAJOR}.${LIBTELETRAFFIC_VERSION_MINOR}.${LIBTELETRAFFIC_VERSION_MICRO})

set (libteletraffic_SRCS src/test_teletraffic.cpp)
set (test_teletraffic_SRCS src/test_teletraffic.cpp)

set (CMAKE_VERBOSE_MAKEFILE on)

# Directorios de ficheros cabecera y de bibliotecas (opciones -I y -L respectivamente)
include_directories (src/include)

# opciones de compilación (CFLAGS)
add_definitions ("-Wall -g")

add_library(teletraffic SHARED ${libteletraffic_SRCS})
set_target_properties(bandwidthmeter PROPERTIES VERSION ${LIBTELETRAFFIC_VERSION_STRING})

add_executable(test_teletraffic ${test_bwmeter_SRCS})
target_link_libraries(test_teletraffic teletraffic pcap m pthread)
