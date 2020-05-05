dnl config.m4 for extension Aspekt
dnl
dnl This file is part of the Aspekt.
dnl
dnl (c) airSlate Inc. <support@airslate.com>
dnl
dnl For the full copyright and license information, please view
dnl the LICENSE file that was distributed with this source code.

dnl Functions -----------------------------------------------------------------
AC_DEFUN([PHP_ASPEKT_ADD_SOURCES], [
  PHP_ASPEKT_SOURCES="$PHP_ASPEKT_SOURCES $1"
])

AC_DEFUN([PHP_ASPEKT_ADD_HEADERS], [
  PHP_ASPEKT_HEADERS="$PHP_ASPEKT_HEADERS $1"
])

AC_DEFUN([PHP_ASPEKT_ADD_FLAGS], [
  PHP_ASPEKT_FLAGS="$PHP_ASPEKT_FLAGS $1"
])

dnl Aspekt ---------------------------------------------------------------------
PHP_ARG_ENABLE(aspekt, whether to enable Aspekt support,
dnl Make sure that the comment is aligned:
[  --enable-aspekt         Enable Aspekt support], yes)

dnl Main ----------------------------------------------------------------------
if test "$PHP_ASPEKT" = "yes"; then
  AC_MSG_CHECKING([for PHP version])
  _found_version=`${PHP_CONFIG} --version`
  _found_vernum=`${PHP_CONFIG} --vernum`

  if test "$_found_vernum" -lt "70000"; then
    AC_MSG_ERROR(
      [not supported. Need a PHP version >= 7.0.0 (found $_found_version)]
    )
  else
    AC_MSG_RESULT([$_found_version (ok)])
  fi

  AC_DEFINE(HAVE_ASPEKT, 1, [Whether you have Aspekt])

  PHP_ASPEKT_ADD_FLAGS([-I@ext_srcdir@/aspekt])

  PHP_ASPEKT_ADD_SOURCES([
    aspekt/execute.c
    aspekt/interceptor.c
    aspekt/joinpoint.c
    aspekt/kind.c
    aspekt/lexer.c
    php_aspekt.c
  ])

  PHP_ASPEKT_ADD_HEADERS([
    aspekt/execute.h
    aspekt/interceptor.h
    aspekt/joinpoint.h
    aspekt/kind.h
    aspekt/lexer.h
    php_aspekt.h
  ])

  PHP_NEW_EXTENSION(aspekt, $PHP_ASPEKT_SOURCES, $ext_shared,, $PHP_ASPEKT_FLAGS)
  PHP_SUBST(ASPEKT_SHARED_LIBADD)

  dnl Compile and link against the bundled PCRE library
  PHP_ADD_EXTENSION_DEP([aspekt], [pcre])

  ifdef([PHP_INSTALL_HEADERS],
    [PHP_INSTALL_HEADERS([ext/aspekt], $PHP_ASPEKT_HEADERS)])

  PHP_ADD_MAKEFILE_FRAGMENT([mk/aspekt.mk])
fi

dnl Debug Mode ----------------------------------------------------------------
PHP_ARG_ENABLE(aspekt-debug, whether to enable debugging support in Aspekt,
dnl Make sure that the comment is aligned:
[  --enable-aspekt-debug   Enable debugging support in Aspekt], no, no)

if test "$PHP_ASPEKT_DEBUG" != "no"; then
  AC_DEFINE(USE_ASPEKT_DEBUG, 1, [Include debugging support in Aspekt])

  dnl Remove all optimization flags from CFLAGS
  changequote({,})
  CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
  CFLAGS=`echo "$CFLAGS" | $SED -e 's/-g[0-9a-z]*//g'`

  CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-O[0-9s]*//g'`
  CXXFLAGS=`echo "$CXXFLAGS" | $SED -e 's/-g[0-9a-z]*//g'`
  changequote([,])

  dnl Add the special flags
  CFLAGS="$CFLAGS -O0 -ggdb"
  CXXFLAGS="$CXXFLAGS -O0 -ggdb"
fi

dnl Code Coverage -------------------------------------------------------------
PHP_ARG_ENABLE(coverage, whether to include code coverage symbols,
[  --enable-coverage       Enable code coverage], no, no)

if test "$PHP_COVERAGE" != "no"; then
  dnl Check if ccache is being used
  case `$php_shtool path $CC` in
    *ccache*) ccache=yes ;;
    *) ccache=no ;;
  esac

  if test "$ccache" = "yes" && (test -z "$CCACHE_DISABLE" || test "$CCACHE_DISABLE" != "1"); then
    err_msg=$(cat | tr '\012' ' ' <<ΕΟF
ccache must be disabled when --enable-coverage option is used.
You can disable ccache by setting environment variable CCACHE_DISABLE=1.
ΕΟF
    )

    AC_MSG_ERROR([$err_msg])
  fi

  AC_CHECK_PROG(LCOV, lcov, lcov)
  PHP_SUBST(LCOV)

  if test -z "$LCOV"; then
    AC_MSG_ERROR([lcov testing requested but lcov not found])
  fi

  AC_CHECK_PROG(GENHTML, genhtml, genhtml)
  PHP_SUBST(GENHTML)

  if test -z "$GENHTML"; then
    AC_MSG_ERROR([Could not find genhtml from the LCOV package])
  fi

  dnl Add the special flags
  LDFLAGS="$LDFLAGS --coverage"
  CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
  CXXFLAGS="$CXXFLAGS -fprofile-arcs -ftest-coverage"

  PHP_ADD_MAKEFILE_FRAGMENT([mk/coverage.mk])

  AC_DEFINE(USE_COVERAGE, 1, [Whether coverage is enabled])
fi
