cmake_minimum_required (VERSION 3.8)

project("Cinema Booking Client" VERSION 1.0.0)

# add the executable

add_executable (
	client WIN32
	"client.c"
	"client.h"
	"client.rc"
	"booking.c"
	"booking.h"
	"framework.h"
	"resource.h"
	"targetver.h"
	)

# add shared librareis

target_link_libraries(client PUBLIC connection)
target_link_libraries(client PUBLIC resources)
# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
