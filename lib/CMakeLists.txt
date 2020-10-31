add_library(
	resources
	STATIC
	"include/resources.h"
	"src/resources/resources.c"
)

target_include_directories(resources PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(resources PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_definitions(resources PRIVATE RESOURCES_VERSION=1)

add_library(
	connection
	STATIC
	"include/connection.h"
	"src/connection/connection.c"
)

target_include_directories(connection PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(connection PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_definitions(connection PRIVATE CONNECTION_VERSION=1)

add_library(
	try
	INTERFACE
)

target_include_directories(try INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(
	data-structure STATIC
	"include/data-structure/avl_tree.h"
	"include/data-structure/avl_tree_node.h"
	"include/data-structure/concurrent_flag.h"
	"include/data-structure/concurrent_queue.h"
	"include/data-structure/flag.h"
	"include/data-structure/queue.h"
	"include/data-structure/stack.h"
	"src/data-structure/avl_tree.c"
	"src/data-structure/avl_tree_node.c"
	"src/data-structure/concurrent_flag.c"
	"src/data-structure/concurrent_queue.c"
	"src/data-structure/flag.c"
	"src/data-structure/queue.c"
	"src/data-structure/stack.c"
)

target_include_directories(data-structure PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/data-structure")
target_include_directories(data-structure PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/data-structure")

target_compile_definitions(data-structure PRIVATE DATA-STRUCTURE_VERSION=1)

