cmake_minimum_required (VERSION 3.8)

project(
	"Cinema Booking Server"
	VERSION 1.0.0
	LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Guards

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
    FATAL_ERROR
    "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# Includere i sottoprogetti.
add_subdirectory ("cinemad")
add_subdirectory ("cinemactl")

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
