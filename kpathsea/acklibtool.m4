dnl Autoconf support for Klibtool.
dnl $Id: acklibtool.m4,v 1.1 1997-05-23 03:09:14 jwe Exp $
dnl
dnl
dnl Find the script, check for subprogs, etc.
AC_DEFUN(kb_AC_PROG_LIBTOOL,
[AC_REQUIRE([AC_PROG_RANLIB])
AC_REQUIRE([AC_CANONICAL_HOST])
#
AC_MSG_CHECKING([for libtool object types])
#
## Check that the klibtool script is in ac_aux_dir.  Don't bother to
# scan PATH.  For one thing, if we found it somewhere there, we couldn't
# use that directory to put the config file ... and that's what we need
# to do, to avoid having to configure in every subdirectory.
LIBTOOL=$ac_aux_dir/klibtool
AC_SUBST(LIBTOOL)
if test ! -r $LIBTOOL; then
  AC_MSG_ERROR([klibtool not in $ac_aux_dir, goodbye])
  exit 1
fi
#
## Argument parsing: we support --enable-shared and --enable-static.
AC_ARG_ENABLE(shared,
[  --enable-shared              build shared libraries [default=no]],,
  enable_shared=no)
#
AC_ARG_ENABLE(static,
[  --enable-static              build shared libraries [default=yes]],,
  enable_static=yes)
#
# If they explicitly --enable-static, make that the link type.
# More commonly, they will just --enable-shared; make that the link type.
# In either case, prepend to any existing LIBTOOL_OBJTYPES.
# If they really want to build both and link statically,
# then they set LIBTOOL_OBJTYPES to SHARED and --enable-static.
test "$enable_static" = yes && LIBTOOL_OBJTYPES=STATIC:$LIBTOOL_OBJTYPES
test "$enable_shared" = yes && LIBTOOL_OBJTYPES=SHARED:$LIBTOOL_OBJTYPES
# Don't bother to remove the trailing :, it'll be ignored.
#
## Finally: Run the klibtool configure command.
LIBTOOL_OBJTYPES=$LIBTOOL_OBJTYPES RANLIB=$RANLIB \
  $LIBTOOL --config-dir $ac_aux_dir configure "$host"
AC_MSG_RESULT($LIBTOOL_OBJTYPES)
])dnl
dnl
dnl
dnl Like AC_REPLACE_FUNCS, but add to LTLIBOBJS instead of LIBOBJS.
AC_DEFUN(kb_AC_KLIBTOOL_REPLACE_FUNCS,
[ dnl cannot require this function, since it doesn't have a provide call.
AC_CHECK_FUNCS($1,, LTLIBOBJS="$LTLIBOBJS $ac_func.lo")
AC_SUBST(LTLIBOBJS)dnl
])dnl
