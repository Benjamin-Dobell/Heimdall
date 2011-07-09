AC_INIT([Heimdall], [1.3], [bug-report@glassechidna.com.au], [heimdall], [http://www.glassechidna.com.au/])
AC_PREREQ([2.59])
PKG_CHECK_MODULES([DEPS], [libusb-1.0 >= 1.0.8])
AC_PROGRAM_CHECK(udevadminstalled, udevadm)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.10 -Wall no-define foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_CONFIG_FILES([Makefile])

AM_MAINTAINER_MODE

AC_MSG_CHECKING([operating system])
case $target in
*-linux*)
	AC_DEFINE(OS_LINUX, [], [Linux backend])
	AC_MSG_RESULT([Linux])
	linuxtarget=true
	;;
*-darwin*)
	AC_DEFINE(OS_DARWIN, [], [Darwin backend])
	AC_MSG_RESULT([Darwin/MacOS X])
	linuxtarget=false
	;;
*-mingw*)
	AC_DEFINE(OS_WINDOWS, [], [Windows backend])
	AC_MSG_RESULT([Windows])
	linuxtarget=false
	;;
*-cygwin*)
	AC_DEFINE(OS_WINDOWS, [], [Windows backend])
	AC_MSG_RESULT([Windows])
	linuxtarget=false
	;;
*)
	AC_MSG_ERROR([unsupported operating system])
esac

AM_CONDITIONAL([LINUXTARGET], [test x$linuxtarget = xtrue])
AM_CONDITIONAL([UDEVADM], [test x$udevadmninstalled = xtrue])

AC_C_BIGENDIAN

AC_OUTPUT
