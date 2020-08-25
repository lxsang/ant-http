# initialise autoconf and set up some basic information about the program we’re packaging
AC_INIT([antd], [1.0.5b], [xsang.le@gmail.com])

# We’re going to use automake for this project
AM_INIT_AUTOMAKE([subdir-objects])

# dependencies
# C compiler
AC_PROG_CC
# libtool for linking
AC_PROG_LIBTOOL
# check if sqlite3 header exists
use_db=no
AC_CHECK_HEADER([sqlite3.h],[
    AC_DEFINE([USE_DB], [1],[Use sqlite3])
    use_db=yes
    # check if the library exists
],[])
AC_CHECK_LIB([sqlite3],[sqlite3_open],[],[
    if test "$use_db" = "yes"; then
        AC_MSG_ERROR([Unable to find sqlite3 shared library])
    fi
])

use_ssl=no
# check if libssl header exists
AC_CHECK_HEADER([openssl/ssl.h],[
    # check if the library exists
    AC_DEFINE([USE_OPENSSL], [1],[Use sqlite3])
    use_ssl=yes
], [])
AC_CHECK_LIB([ssl],[SSL_read],[], [
    if test "$use_ssl" = "yes"; then
        AC_MSG_ERROR([Unable to find libssl shared library])
    fi
])

AC_CHECK_LIB([crypto],[ECDH_compute_key],[], [
    if test "$use_ssl" = "yes"; then
        AC_MSG_ERROR([Unable to find libcrypto shared library])
    fi
])

# check for pthread
AC_CHECK_LIB([pthread], [pthread_create], [],  [
    AC_MSG_ERROR([libpthread is not found])])

# check for dl
AC_CHECK_LIB([dl], [dlopen], [], [
  AC_MSG_ERROR([unable to find dlopen()])
])


# check for zlib
use_zlib=no
# check if libssl header exists
AC_CHECK_HEADER([zlib.h],[
    # check if the library exists
    AC_DEFINE([USE_ZLIB], [1],[Use zlib])
    use_zlib=yes
], [])
AC_CHECK_LIB([z],[deflate],[], [
    if test "$use_zlib" = "yes"; then
        AC_MSG_ERROR([Unable to find zlib shared library])
    fi
])


AC_DEFINE([_GNU_SOURCE], [1],[Use GNU source])
# AC_CANONICAL_HOST is needed to access the 'host_os' variable    

# debug option
AC_ARG_ENABLE([debug],
    [  --enable-debug    Turn on debugging],
    [case "${enableval}" in
        yes) AC_DEFINE([DEBUG], [1],[Enable debug]) ;;
        no) ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
    esac],[debug=false])

AC_CANONICAL_HOST
build_linux=no
build_windows=no
build_mac=no
# Detect the target system
case "${host_os}" in
    linux*)
        AC_DEFINE([LINUX], [1],[Linux system])
        build_linux=yes
        ;;
    darwin*)
        build_mac=yes
        AC_DEFINE([MACOS], [1],[MacOS system])
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac
# case for window:
# cygwin*|mingw*)
#    build_windows=yes
#    ;;
# Pass the conditionals to automake
AM_CONDITIONAL([DB], [test "$use_db" = "yes"])
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])

# find a file called Makefile.in, substitute placeholders
# like @PACKAGE_VERSION@ with values like 0.1.0a, 
# and write the results to Makefile.
AC_CONFIG_FILES([Makefile])

# output the script:
AC_OUTPUT
