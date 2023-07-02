OS!=uname
ROOT?=${.CURDIR}/../..
_ARCH!=${ROOT}/scripts/arch.sh
ARCH?=${_ARCH}
CONFIG?=Debug

PROG:=Game.${ARCH}
SRCS=${ROOT}/src/components.cpp \
     ${ROOT}/src/discord.cpp \
     ${ROOT}/src/fs.cpp \
     ${ROOT}/src/image.cpp \
     ${ROOT}/src/main.cpp \
     ${ROOT}/src/physics.cpp \
     ${ROOT}/src/sdl.cpp \
     ${ROOT}/src/sprite.cpp \
     ${ROOT}/src/systems.cpp \
     ${ROOT}/src/text.cpp \
     ${ROOT}/deps/src/flecs.c \
     ${ROOT}/deps/src/metrohash128.cpp \
     ${ROOT}/deps/src/metrohash64.cpp
.if "${OS}" == "Linux"
     SRCS+=${ROOT}/deps/src/discord/achievement_manager.cpp \
           ${ROOT}/deps/src/discord/activity_manager.cpp \
           ${ROOT}/deps/src/discord/application_manager.cpp \
           ${ROOT}/deps/src/discord/core.cpp \
           ${ROOT}/deps/src/discord/image_manager.cpp \
           ${ROOT}/deps/src/discord/lobby_manager.cpp \
           ${ROOT}/deps/src/discord/network_manager.cpp \
           ${ROOT}/deps/src/discord/overlay_manager.cpp \
           ${ROOT}/deps/src/discord/relationship_manager.cpp \
           ${ROOT}/deps/src/discord/storage_manager.cpp \
           ${ROOT}/deps/src/discord/store_manager.cpp \
           ${ROOT}/deps/src/discord/types.cpp \
           ${ROOT}/deps/src/discord/user_manager.cpp \
           ${ROOT}/deps/src/discord/voice_manager.cpp
.endif
WARNS?=-Wall -Wextra
CSTD=-std=c17
CFLAGS+=-xc -DUSE_SDL=1 -I${ROOT}/include -I${ROOT}/deps/include -I${.CURDIR}
CXXFLAGS+=-xc++ -std=c++17

.if "${CONFIG}" == "Debug"
     CFLAGS+=-D_DEBUG=1
.else
     CFLAGS+=-DNDEBUG=1
.endif

LIBDIR=${ROOT}/deps/lib
LDADD+=${LIBDIR}/${ARCH}/${CONFIG}/libPhysX_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXCharacterKinematic_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXCommon_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXExtensions_static_64${SLIBEXT} \
	  ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXFoundation_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXPvdSDK_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libSDL3${DLIBEXT} \
       ${LIBDIR}/${ARCH}/libzstd${SLIBEXT}

.if "${OS}" == "Linux"
     LDADD+=${LIBDIR}/${ARCH}/discord_game_sdk.so
.endif

.include <bsd.prog.mk>
