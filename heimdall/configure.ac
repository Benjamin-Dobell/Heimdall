AC_INIT([Heimdall], [0.1], [bug-report@glassechidna.com.au], [heimdall], [http://www.glassechidna.com.au/])
AC_PREREQ([2.59])
PKG_CHECK_MODULES([DEPS], [libusb-1.0 >= 1.0.8])
AM_INIT_AUTOMAKE([1.10 -Wall no-define foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_CONFIG_FILES([Makefile])

AM_MAINTAINER_MODE

AC_MSG_CHECKING([operating system])
case $host in
*-linux*)
	AC_DEFINE(OS_LINUX, [], [Linux backend])
	AC_MSG_RESULT([Linux])
	;;
*-darwin*)
	AC_DEFINE(OS_DARWIN, [], [Darwin backend])
	AC_MSG_RESULT([Darwin/MacOS X])
	;;
*-mingw*)
	AC_DEFINE(OS_WINDOWS, [], [Windows backend])
	AC_MSG_RESULT([Windows])
	;;
*-cygwin*)
	AC_DEFINE(OS_WINDOWS, [], [Windows backend])
	AC_MSG_RESULT([Windows])
	;;
*)
	AC_MSG_ERROR([unsupported operating system])
esac

AC_C_BIGENDIAN

AC_OUTPUT
