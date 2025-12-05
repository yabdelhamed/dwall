# dwall version
VERSION = 1.0

# paths
PREFIX = /usr/local

# includes and libs
INCS =
LIBS = -lm

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\"
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler
CC = cc
