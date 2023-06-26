ROOT?=${.CURDIR}/../..
_ARCH!=${ROOT}/scripts/arch.sh
ARCH?=${_ARCH}
CONFIG?=Debug

PROG:=Game.${ARCH}
SRCS=${ROOT}/src/components.cpp \
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
WARNS?=-Wall -Wextra
CSTD=-std=c17
CFLAGS+=-xc -DUSE_SDL=1 -I${ROOT}/include -I${ROOT}/deps/include
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

.include <bsd.prog.mk>
