AC_INIT([libpit], [1.4], [bug-report@glassechidna.com.au], [libpit], [http://www.glassechidna.com.au/])
AC_PREREQ([2.59])
AC_CANONICAL_TARGET
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.10 -Wall no-define foreign])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX

LT_INIT()

AC_SUBST([LIBPIT_API_VERSION], [1.4])
AC_CONFIG_FILES([Makefile])

AM_MAINTAINER_MODE

AC_MSG_CHECKING([operating system])
case $target in
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
