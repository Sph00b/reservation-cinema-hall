project(
	"Cinema Booking Daemon"
	VERSION 1.0.0
)

# add the executable
add_executable (
	cinemad
	"cinemad.c"
	"try.h"
	"utils.h"
	"utils.c"
	"database.c"
	"database.h"
	"storage.c"
	"storage.h"
	"index_table.c"
	"index_table.h"
	)

target_link_libraries(cinemad PUBLIC pthread)

# add shared librareis
target_link_libraries(cinemad PUBLIC connection)
target_link_libraries(cinemad PUBLIC resources)
target_link_libraries(cinemad PUBLIC data-structure)

# TODO: Aggiungere i test e, se necessario, installare le destinazioni.
