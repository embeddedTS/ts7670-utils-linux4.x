#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(ts7670-utils-linux4.x, 1.0, support@embeddedTS.com)
AC_CONFIG_SRCDIR([src/i2c-dev.h])

AM_INIT_AUTOMAKE([1.00 foreign no-define])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([gpiod], [gpiod_line_request_input], [], [AC_MSG_ERROR([libgpiod not found])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([bzero getpagesize memset munmap select strstr strtoul strtoull])

AC_CONFIG_FILES([
	Makefile 
	src/Makefile 
])
AC_OUTPUT
