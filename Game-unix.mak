_ARCH!=${.CURDIR}/scripts/arch.sh
ARCH?=${_ARCH}
CONFIG?=Debug

PROG:=Game.${ARCH}
SRCS=src/components.cpp \
     src/fs.cpp \
     src/image.cpp \
     src/main.cpp \
     src/physics.cpp \
     src/sdl.cpp \
     src/sprite.cpp \
     src/systems.cpp \
     src/text.cpp \
     deps/src/flecs.c \
     deps/src/metrohash128.cpp \
     deps/src/metrohash64.cpp
WARNS?=-Wall -Wextra
CSTD=-std=c17
CFLAGS+=-xc -DUSE_SDL=1 -I${.CURDIR}/include -I${.CURDIR}/deps/include
CXXFLAGS+=-xc++ -std=c++17

.if "${CONFIG}" == "Debug"
     CFLAGS+=-D_DEBUG=1
.else
     CFLAGS+=-DNDEBUG=1
.endif

LIBDIR=${.CURDIR}/deps/lib
LDADD+=${LIBDIR}/${ARCH}/${CONFIG}/libPhysX_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXCharacterKinematic_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXCommon_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXExtensions_static_64${SLIBEXT} \
	   ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXFoundation_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libPhysXPvdSDK_static_64${SLIBEXT} \
       ${LIBDIR}/${ARCH}/${CONFIG}/libSDL3${DLIBEXT} \
       ${LIBDIR}/${ARCH}/libzstd${SLIBEXT}

.include <bsd.prog.mk>
