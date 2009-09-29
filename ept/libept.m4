# LIBEPT_DEFS([LIBEPT_REQS=libtagcoll2])
# ---------------------------------------
AC_DEFUN([LIBEPT_DEFS],
[
	dnl Import libtagcoll data
	PKG_CHECK_MODULES(LIBEPT,m4_default([$1], libept))
	AC_SUBST(LIBEPT_CFLAGS)
	AC_SUBST(LIBEPT_LIBS)
])
