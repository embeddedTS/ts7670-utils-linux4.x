GITCOMMIT:= $(shell git describe --abbrev=12 --dirty --always)

tshwctl_SOURCES = tshwctl.c i2c-rtc.c pwmlib.c gpiod-helper.c
tshwctl_CFLAGS = -O0 -Wall -fno-tree-cselim
tshwctl_CPPFLAGS = -DGITCOMMIT="\"${GITCOMMIT}\""
tshwctl_LDADD = -lgpiod

tsmicroctl_SOURCES = tsmicroctl.c
tsmicroctl_CFLAGS = -O0 -Wall -fno-tree-cselim
tsmicroctl_CPPFLAGS = -DCTL -DGITCOMMIT="\"${GITCOMMIT}\""

bin_PROGRAMS = tshwctl tsmicroctl

