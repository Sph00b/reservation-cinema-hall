cmake_minimum_required (VERSION 3.8)

project("Cinema Booking Client" VERSION 1.0.0)

# add the executable

add_executable (
	cinema-client WIN32
	"cinema-client.c"
	"cinema-client.h"
	"cinema-client.rc"
	"booking.c"
	"booking.h"
	"framework.h"
	"resource.h"
	"targetver.h"
	)

# add shared librareis

target_link_libraries(cinema-client PUBLIC connection)
target_link_libraries(cinema-client PUBLIC resources)
# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
