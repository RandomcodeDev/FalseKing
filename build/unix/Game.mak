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
     ${ROOT}/src/player.cpp \
     ${ROOT}/src/sdl.cpp \
     ${ROOT}/src/sprite.cpp \
     ${ROOT}/src/sprites.cpp \
     ${ROOT}/src/systems.cpp \
     ${ROOT}/src/text.cpp \
     ${ROOT}/deps-public/src/flecs.c \
     ${ROOT}/deps-public/src/metrohash128.cpp \
     ${ROOT}/deps-public/src/metrohash64.cpp
.if "${OS}" == "Linux"
     SRCS+=${ROOT}/deps-public/src/discord/achievement_manager.cpp \
           ${ROOT}/deps-public/src/discord/activity_manager.cpp \
           ${ROOT}/deps-public/src/discord/application_manager.cpp \
           ${ROOT}/deps-public/src/discord/core.cpp \
           ${ROOT}/deps-public/src/discord/image_manager.cpp \
           ${ROOT}/deps-public/src/discord/lobby_manager.cpp \
           ${ROOT}/deps-public/src/discord/network_manager.cpp \
           ${ROOT}/deps-public/src/discord/overlay_manager.cpp \
           ${ROOT}/deps-public/src/discord/relationship_manager.cpp \
           ${ROOT}/deps-public/src/discord/storage_manager.cpp \
           ${ROOT}/deps-public/src/discord/store_manager.cpp \
           ${ROOT}/deps-public/src/discord/types.cpp \
           ${ROOT}/deps-public/src/discord/user_manager.cpp \
           ${ROOT}/deps-public/src/discord/voice_manager.cpp
.endif
CSTD=-std=c17
CFLAGS+=-xc -DUSE_SDL=1 -I${ROOT}/include -I${ROOT}/deps-public/include -I${.CURDIR} -Wall -Wextra
CXXFLAGS+=-xc++ -std=c++17

.if "${CONFIG}" == "Debug"
     CFLAGS+=-D_DEBUG=1
.else
     CFLAGS+=-DNDEBUG=1
.endif

LIBDIR=${ROOT}/deps-public/lib
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
