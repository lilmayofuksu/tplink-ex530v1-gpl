cmake_minimum_required(VERSION 2.6)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)

PROJECT(ubus C)
ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99  -Wmissing-declarations)

# OPTION(BUILD_LUA "build Lua plugin" ON)
# OPTION(BUILD_EXAMPLES "build examples" ON)
# OPTION(ENABLE_SYSTEMD "systemd support" ON)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(UBUS_UNIX_SOCKET "/var/run/ubus.sock")
SET(UBUS_MAX_MSGLEN 1048576)

ADD_DEFINITIONS( -DUBUS_UNIX_SOCKET="${UBUS_UNIX_SOCKET}")
ADD_DEFINITIONS( -DUBUS_MAX_MSGLEN=${UBUS_MAX_MSGLEN})

# IF(APPLE)
#   INCLUDE_DIRECTORIES(/opt/local/include)
#   LINK_DIRECTORIES(/opt/local/lib)
# ENDIF()

SET(WORKPATH $ENV{APP_PUBLIC_DIR})

INCLUDE_DIRECTORIES(${WORKPATH}/libubox-2015-06-14/install/include)
LINK_DIRECTORIES(${WORKPATH}/libubox-2015-06-14/install/lib)


ADD_LIBRARY(ubus SHARED libubus.c libubus-io.c libubus-obj.c libubus-sub.c libubus-req.c)
TARGET_LINK_LIBRARIES(ubus ubox)

ADD_EXECUTABLE(ubusd ubusd.c ubusd_id.c ubusd_obj.c ubusd_proto.c ubusd_event.c)
TARGET_LINK_LIBRARIES(ubusd ubox)


ADD_EXECUTABLE(msg msg.c)
TARGET_LINK_LIBRARIES(msg ubus ubox)

#find_library(json NAMES json-c json)
find_library(blobmsg_json NAMES blobmsg_json PATHS ${WORKPATH}/libubox-2015-06-14/install/lib)
IF(EXISTS ${blobmsg_json})
	ADD_EXECUTABLE(cli cli.c)
	SET_TARGET_PROPERTIES(cli PROPERTIES OUTPUT_NAME ubus)
	TARGET_LINK_LIBRARIES(cli ubus ubox blobmsg_json)
	#TARGET_LINK_LIBRARIES(cli ubus ubox blobmsg_json ${json})
	
	INSTALL(TARGETS cli
		RUNTIME DESTINATION bin
	)
ENDIF()

INSTALL(TARGETS ubus msg
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)
INSTALL(TARGETS ubusd
	RUNTIME DESTINATION bin
)

INSTALL(FILES ubusmsg.h ubus_common.h libubus.h DESTINATION include)

