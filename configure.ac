#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libjason], [1.00], [max@arangodb.com])
${CFLAGS=""}
${CXXFLAGS=""}
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([JasonParser.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

m4_include([m4/configure.basics])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_AR

LT_INIT

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h stdint.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset pow])

# Checks for compiler options.
SSE42OPTION=""
TRY_CXX_OPTION([-msse4.2],
[
  detectedsse42=yes
],
[
  detectedsse42=no
])

AC_ARG_ENABLE(sse42,
  AS_HELP_STRING([--enable-sse42], [overwrite automatic recognition of SSE 4.2 for x86 platform (default: automatic)]),
  [usesse42="${enableval:-yes}"],
  [usesse42="auto"]
)

if test "x$usesse42" != "xauto"; then
  echo "Overwriting SSE4.2 detection with $usesse42"
  detectedsse42=$usesse42
fi

if test "x$detectedsse42" = "xyes"; then
  SSE42OPTION="-msse4.2"
  RAPIDJSONSSE42="-DRAPIDJSON_SSE42"
else
  SSE42OPTION=""
  RAPIDJSONSSE42=""
fi

AC_SUBST(SSE42OPTION, ${SSE42OPTION})
AC_SUBST(RAPIDJSONSSE42, ${RAPIDJSONSSE42})

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
