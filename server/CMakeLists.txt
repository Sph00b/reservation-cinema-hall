cmake_minimum_required (VERSION 3.8)

project(
	"Cinema Booking Server"
	VERSION 1.0.0
	LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Includere i sottoprogetti.
add_subdirectory ("cinemad")
add_subdirectory ("cinemactl")

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
