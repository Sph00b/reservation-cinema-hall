# CMakeList.txt: file di progetto CMake di primo livello. Eseguire la configurazione globale
# e includere qui i sottoprogetti.
#
cmake_minimum_required (VERSION 3.8)

project (
	"Cinema Booking System"
	VERSION 1.0.0
)

# Guards

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
    FATAL_ERROR
    "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# Includere i sottoprogetti.
add_subdirectory ("lib")
add_subdirectory ("cinema-client")
add_subdirectory ("cinema-server")
