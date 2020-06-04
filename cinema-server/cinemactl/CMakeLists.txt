cmake_minimum_required (VERSION 3.8)

project("Cinema Booking Control Manager" VERSION 1.0.0)


# add the executable
add_executable (cinemactl "cinemactl.c")

# add the librareis
target_link_libraries(cinemactl PUBLIC connection)
target_link_libraries(cinemactl PUBLIC resources)

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
