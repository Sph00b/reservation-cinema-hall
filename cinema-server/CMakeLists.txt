cmake_minimum_required (VERSION 3.8)

project("Cinema Booking Server" VERSION 1.0.0)

# Includere i sottoprogetti.
add_subdirectory ("cinemad")
add_subdirectory ("cinemactl")

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
