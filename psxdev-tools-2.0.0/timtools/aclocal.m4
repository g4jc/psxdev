dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

# Configure paths for IMLIB
# Frank Belew     98-8-31
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_IMLIB([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for IMLIB, and define IMLIB_CFLAGS and IMLIB_LIBS
dnl
AC_DEFUN(AM_PATH_IMLIB,
[dnl 
dnl Get the cflags and libraries from the imlib-config script
dnl
AC_ARG_WITH(imlib-prefix,[  --with-imlib-prefix=PFX   Prefix where IMLIB is installed (optional)],
            imlib_prefix="$withval", imlib_prefix="")
AC_ARG_WITH(imlib-exec-prefix,[  --with-imlib-exec-prefix=PFX Exec prefix where IMLIB is installed (optional)],
            imlib_exec_prefix="$withval", imlib_exec_prefix="")
AC_ARG_ENABLE(imlibtest, [  --disable-imlibtest       Do not try to compile and run a test IMLIB program],
		    , enable_imlibtest=yes)

  if test x$imlib_exec_prefix != x ; then
     imlib_args="$imlib_args --exec-prefix=$imlib_exec_prefix"
     if test x${IMLIB_CONFIG+set} != xset ; then
        IMLIB_CONFIG=$imlib_exec_prefix/bin/imlib-config
     fi
  fi
  if test x$imlib_prefix != x ; then
     imlib_args="$imlib_args --prefix=$imlib_prefix"
     if test x${IMLIB_CONFIG+set} != xset ; then
        IMLIB_CONFIG=$imlib_prefix/bin/imlib-config
     fi
  fi

  AC_PATH_PROG(IMLIB_CONFIG, imlib-config, no)
  min_imlib_version=ifelse([$1], ,1.8.2,$1)
  AC_MSG_CHECKING(for IMLIB - version >= $min_imlib_version)
  no_imlib=""
  if test "$IMLIB_CONFIG" = "no" ; then
    no_imlib=yes
  else
    IMLIB_CFLAGS=`$IMLIB_CONFIG $imlibconf_args --cflags`
    IMLIB_LIBS=`$IMLIB_CONFIG $imlibconf_args --libs`

    imlib_major_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    imlib_minor_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    imlib_micro_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_imlibtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $IMLIB_CFLAGS"
      LIBS="$LIBS $IMLIB_LIBS"
dnl
dnl Now check if the installed IMLIB is sufficiently new. (Also sanity
dnl checks the results of imlib-config to some extent
dnl
      rm -f conf.imlibtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Imlib.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.imlibtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_imlib_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_imlib_version");
     exit(1);
   }

    if (($imlib_major_version > major) ||
        (($imlib_major_version == major) && ($imlib_minor_version > minor)) ||
	(($imlib_major_version == major) && ($imlib_minor_version == minor) &&
	($imlib_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'imlib-config --version' returned %d.%d, but the minimum version\n", $imlib_major_version, $imlib_minor_version);
      printf("*** of IMLIB required is %d.%d. If imlib-config is correct, then it is\n", major, minor);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If imlib-config was wrong, set the environment variable IMLIB_CONFIG\n");
      printf("*** to point to the correct copy of imlib-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_imlib=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_imlib" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$IMLIB_CONFIG" = "no" ; then
       echo "*** The imlib-config script installed by IMLIB could not be found"
       echo "*** If IMLIB was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the IMLIB_CONFIG environment variable to the"
       echo "*** full path to imlib-config."
     else
       if test -f conf.imlibtest ; then
        :
       else
          echo "*** Could not run IMLIB test program, checking why..."
          CFLAGS="$CFLAGS $IMLIB_CFLAGS"
          LIBS="$LIBS $IMLIB_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <Imlib.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding IMLIB or finding the wrong"
          echo "*** version of IMLIB. If it is not finding IMLIB, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means IMLIB was incorrectly installed"
          echo "*** or that you have moved IMLIB since it was installed. In the latter case, you"
          echo "*** may want to edit the imlib-config script: $IMLIB_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     IMLIB_CFLAGS=""
     IMLIB_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(IMLIB_CFLAGS)
  AC_SUBST(IMLIB_LIBS)
  rm -f conf.imlibtest
])

# Check for gdk-imlib
AC_DEFUN(AM_PATH_GDK_IMLIB,
[dnl 
dnl Get the cflags and libraries from the imlib-config script
dnl
AC_ARG_WITH(imlib-prefix,[  --with-imlib-prefix=PFX   Prefix where IMLIB is installed (optional)],
            imlib_prefix="$withval", imlib_prefix="")
AC_ARG_WITH(imlib-exec-prefix,[  --with-imlib-exec-prefix=PFX Exec prefix where IMLIB is installed (optional)],
            imlib_exec_prefix="$withval", imlib_exec_prefix="")
AC_ARG_ENABLE(imlibtest, [  --disable-imlibtest       Do not try to compile and run a test IMLIB program],
		    , enable_imlibtest=yes)

  if test x$imlib_exec_prefix != x ; then
     imlib_args="$imlib_args --exec-prefix=$imlib_exec_prefix"
     if test x${IMLIB_CONFIG+set} != xset ; then
        IMLIB_CONFIG=$imlib_exec_prefix/bin/imlib-config
     fi
  fi
  if test x$imlib_prefix != x ; then
     imlib_args="$imlib_args --prefix=$imlib_prefix"
     if test x${IMLIB_CONFIG+set} != xset ; then
        IMLIB_CONFIG=$imlib_prefix/bin/imlib-config
     fi
  fi

  AC_PATH_PROG(IMLIB_CONFIG, imlib-config, no)
  min_imlib_version=ifelse([$1], ,1.8.2,$1)
  AC_MSG_CHECKING(for IMLIB - version >= $min_imlib_version)
  no_imlib=""
  if test "$IMLIB_CONFIG" = "no" ; then
    no_imlib=yes
  else
    GDK_IMLIB_CFLAGS=`$IMLIB_CONFIG $imlibconf_args --cflags-gdk`
    GDK_IMLIB_LIBS=`$IMLIB_CONFIG $imlibconf_args --libs-gdk`

    imlib_major_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    imlib_minor_version=`$IMLIB_CONFIG $imlib_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    if test "x$enable_imlibtest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GDK_IMLIB_CFLAGS"
      LIBS="$LIBS $GDK_IMLIB_LIBS"
dnl
dnl Now check if the installed IMLIB is sufficiently new. (Also sanity
dnl checks the results of imlib-config to some extent
dnl
      rm -f conf.imlibtest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <gdk_imlib.h>

int main ()
{
  int major, minor;
  char *tmp_version;

  system ("touch conf.gdkimlibtest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_imlib_version");
  if (sscanf(tmp_version, "%d.%d", &major, &minor) != 2) {
     printf("%s, bad version string\n", "$min_imlib_version");
     exit(1);
   }

    if (($imlib_major_version > major) ||
        (($imlib_major_version == major) && ($imlib_minor_version >= minor)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'imlib-config --version' returned %d.%d, but the minimum version\n", $imlib_major_version, $imlib_minor_version);
      printf("*** of IMLIB required is %d.%d. If imlib-config is correct, then it is\n", major, minor);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If imlib-config was wrong, set the environment variable IMLIB_CONFIG\n");
      printf("*** to point to the correct copy of imlib-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_imlib=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_imlib" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$IMLIB_CONFIG" = "no" ; then
       echo "*** The imlib-config script installed by IMLIB could not be found"
       echo "*** If IMLIB was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the IMLIB_CONFIG environment variable to the"
       echo "*** full path to imlib-config."
     else
       if test -f conf.gdkimlibtest ; then
        :
       else
          echo "*** Could not run IMLIB test program, checking why..."
          CFLAGS="$CFLAGS $GDK_IMLIB_CFLAGS"
          LIBS="$LIBS $GDK_IMLIB_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <gdk_imlib.h>
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding IMLIB or finding the wrong"
          echo "*** version of IMLIB. If it is not finding IMLIB, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means IMLIB was incorrectly installed"
          echo "*** or that you have moved IMLIB since it was installed. In the latter case, you"
          echo "*** may want to edit the imlib-config script: $IMLIB_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     IMLIB_CFLAGS=""
     IMLIB_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GDK_IMLIB_CFLAGS)
  AC_SUBST(GDK_IMLIB_LIBS)
  rm -f conf.gdkimlibtest
])

# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 5

AC_DEFUN(AM_WITH_NLS,
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    USE_INCLUDED_LIBINTL=no

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS)
      AC_MSG_CHECKING([whether included gettext is requested])
      AC_ARG_WITH(included-gettext,
        [  --with-included-gettext use the GNU gettext library included here],
        nls_cv_force_use_gnu_gettext=$withval,
        nls_cv_force_use_gnu_gettext=no)
      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)

      nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
      if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
        dnl User does not insist on using GNU NLS library.  Figure out what
        dnl to use.  If gettext or catgets are available (in this order) we
        dnl use this.  Else we have to fall back to GNU NLS library.
	dnl catgets is only used if permitted by option --with-catgets.
	nls_cv_header_intl=
	nls_cv_header_libgt=
	CATOBJEXT=NONE

	AC_CHECK_HEADER(libintl.h,
	  [AC_CACHE_CHECK([for gettext in libc], gt_cv_func_gettext_libc,
	    [AC_TRY_LINK([#include <libintl.h>], [return (int) gettext ("")],
	       gt_cv_func_gettext_libc=yes, gt_cv_func_gettext_libc=no)])

	   if test "$gt_cv_func_gettext_libc" != "yes"; then
	     AC_CHECK_LIB(intl, bindtextdomain,
	       [AC_CACHE_CHECK([for gettext in libintl],
		 gt_cv_func_gettext_libintl,
		 [AC_CHECK_LIB(intl, gettext,
		  gt_cv_func_gettext_libintl=yes,
		  gt_cv_func_gettext_libintl=no)],
		 gt_cv_func_gettext_libintl=no)])
	   fi

	   if test "$gt_cv_func_gettext_libc" = "yes" \
	      || test "$gt_cv_func_gettext_libintl" = "yes"; then
	      AC_DEFINE(HAVE_GETTEXT)
	      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
		[test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)dnl
	      if test "$MSGFMT" != "no"; then
		AC_CHECK_FUNCS(dcgettext)
		AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
		AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		AC_TRY_LINK(, [extern int _nl_msg_cat_cntr;
			       return _nl_msg_cat_cntr],
		  [CATOBJEXT=.gmo
		   DATADIRNAME=share],
		  [CATOBJEXT=.mo
		   DATADIRNAME=lib])
		INSTOBJEXT=.mo
	      fi
	    fi
	])

        if test "$CATOBJEXT" = "NONE"; then
	  AC_MSG_CHECKING([whether catgets can be used])
	  AC_ARG_WITH(catgets,
	    [  --with-catgets          use catgets functions if available],
	    nls_cv_use_catgets=$withval, nls_cv_use_catgets=no)
	  AC_MSG_RESULT($nls_cv_use_catgets)

	  if test "$nls_cv_use_catgets" = "yes"; then
	    dnl No gettext in C library.  Try catgets next.
	    AC_CHECK_LIB(i, main)
	    AC_CHECK_FUNC(catgets,
	      [AC_DEFINE(HAVE_CATGETS)
	       INTLOBJS="\$(CATOBJS)"
	       AC_PATH_PROG(GENCAT, gencat, no)dnl
	       if test "$GENCAT" != "no"; then
		 AC_PATH_PROG(GMSGFMT, gmsgfmt, no)
		 if test "$GMSGFMT" = "no"; then
		   AM_PATH_PROG_WITH_TEST(GMSGFMT, msgfmt,
		    [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)
		 fi
		 AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		   [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		 USE_INCLUDED_LIBINTL=yes
		 CATOBJEXT=.cat
		 INSTOBJEXT=.cat
		 DATADIRNAME=lib
		 INTLDEPS='$(top_builddir)/intl/libintl.a'
		 INTLLIBS=$INTLDEPS
		 LIBS=`echo $LIBS | sed -e 's/-lintl//'`
		 nls_cv_header_intl=intl/libintl.h
		 nls_cv_header_libgt=intl/libgettext.h
	       fi])
	  fi
        fi

        if test "$CATOBJEXT" = "NONE"; then
	  dnl Neither gettext nor catgets in included in the C library.
	  dnl Fall back on GNU gettext library.
	  nls_cv_use_gnu_gettext=yes
        fi
      fi

      if test "$nls_cv_use_gnu_gettext" = "yes"; then
        dnl Mark actions used to generate GNU NLS library.
        INTLOBJS="\$(GETTOBJS)"
        AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
        AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
        AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
        AC_SUBST(MSGFMT)
	USE_INCLUDED_LIBINTL=yes
        CATOBJEXT=.gmo
        INSTOBJEXT=.mo
        DATADIRNAME=share
	INTLDEPS='$(top_builddir)/intl/libintl.a'
	INTLLIBS=$INTLDEPS
	LIBS=`echo $LIBS | sed -e 's/-lintl//'`
        nls_cv_header_intl=intl/libintl.h
        nls_cv_header_libgt=intl/libgettext.h
      fi

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi

      # We need to process the po/ directory.
      POSUB=po
    else
      DATADIRNAME=share
      nls_cv_header_intl=intl/libintl.h
      nls_cv_header_libgt=intl/libgettext.h
    fi
    AC_LINK_FILES($nls_cv_header_libgt, $nls_cv_header_intl)
    AC_OUTPUT_COMMANDS(
     [case "$CONFIG_FILES" in *po/Makefile.in*)
        sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile
      esac])


    # If this is used in GNU gettext we have to set USE_NLS to `yes'
    # because some of the sources are only built for this goal.
    if test "$PACKAGE" = gettext; then
      USE_NLS=yes
      USE_INCLUDED_LIBINTL=yes
    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done

    dnl Make all variables we use known to autoconf.
    AC_SUBST(USE_INCLUDED_LIBINTL)
    AC_SUBST(CATALOGS)
    AC_SUBST(CATOBJEXT)
    AC_SUBST(DATADIRNAME)
    AC_SUBST(GMOFILES)
    AC_SUBST(INSTOBJEXT)
    AC_SUBST(INTLDEPS)
    AC_SUBST(INTLLIBS)
    AC_SUBST(INTLOBJS)
    AC_SUBST(POFILES)
    AC_SUBST(POSUB)
  ])

AC_DEFUN(AM_GNU_GETTEXT,
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h sys/param.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
strdup __argz_count __argz_stringify __argz_next])

   if test "${ac_cv_func_stpcpy+set}" != "set"; then
     AC_CHECK_FUNCS(stpcpy)
   fi
   if test "${ac_cv_func_stpcpy}" = "yes"; then
     AC_DEFINE(HAVE_STPCPY)
   fi

   AM_LC_MESSAGES
   AM_WITH_NLS

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       for lang in ${LINGUAS=$ALL_LINGUAS}; do
         case "$ALL_LINGUAS" in
          *$lang*) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl The reference to <locale.h> in the installed <libintl.h> file
   dnl must be resolved because we cannot expect the users of this
   dnl to define HAVE_LOCALE_H.
   if test $ac_cv_header_locale_h = yes; then
     INCLUDE_LOCALE_H="#include <locale.h>"
   else
     INCLUDE_LOCALE_H="\
/* The system does not provide the header <locale.h>.  Take care yourself.  */"
   fi
   AC_SUBST(INCLUDE_LOCALE_H)

   dnl Determine which catalog format we have (if any is needed)
   dnl For now we know about two different formats:
   dnl   Linux libc-5 and the normal X/Open format
   test -d intl || mkdir intl
   if test "$CATOBJEXT" = ".cat"; then
     AC_CHECK_HEADER(linux/version.h, msgformat=linux, msgformat=xopen)

     dnl Transform the SED scripts while copying because some dumb SEDs
     dnl cannot handle comments.
     sed -e '/^#/d' $srcdir/intl/$msgformat-msg.sed > intl/po2msg.sed
   fi
   dnl po2tbl.sed is always needed.
   sed -e '/^#.*[^\\]$/d' -e '/^#$/d' \
     $srcdir/intl/po2tbl.sed.in > intl/po2tbl.sed

   dnl In the intl/Makefile.in we have a special dependency which makes
   dnl only sense for gettext.  We comment this out for non-gettext
   dnl packages.
   if test "$PACKAGE" = "gettext"; then
     GT_NO="#NO#"
     GT_YES=
   else
     GT_NO=
     GT_YES="#YES#"
   fi
   AC_SUBST(GT_NO)
   AC_SUBST(GT_YES)

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but ($top_srcdir).
   dnl Try to locate is.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl *** For now the libtool support in intl/Makefile is not for real.
   l=
   AC_SUBST(l)

   dnl Generate list of files to be processed by xgettext which will
   dnl be included in po/Makefile.
   test -d po || mkdir po
   if test "x$srcdir" != "x."; then
     if test "x`echo $srcdir | sed 's@/.*@@'`" = "x"; then
       posrcprefix="$srcdir/"
     else
       posrcprefix="../$srcdir/"
     fi
   else
     posrcprefix="../"
   fi
   rm -f po/POTFILES
   sed -e "/^#/d" -e "/^\$/d" -e "s,.*,	$posrcprefix& \\\\," -e "\$s/\(.*\) \\\\/\1/" \
	< $srcdir/po/POTFILES.in > po/POTFILES
  ])

# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN(AM_PATH_PROG_WITH_TEST,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])

# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

AC_DEFUN(AM_LC_MESSAGES,
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES)
    fi
  fi])

