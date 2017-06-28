#
# Include the TEA standard macro set
#

builtin(include,tclconfig/tcl.m4)

#
# Add here whatever m4 macros you want to define for your package
#

AC_DEFUN([FIND_LPSOLVE_INCLUDES], [
    dnl TEA specific: Make sure we are initialized
    AC_REQUIRE([TEA_INIT])
    #
    # Ok, lets find the lpsolve configuration
    # First, look for one uninstalled.
    # the alternative search directory is invoked by --with-lpsolve
    #

	AC_ARG_WITH(lpsolveinclude,
	    AC_HELP_STRING([--with-lpsolveinclude],
		[base directory for lpsolve headers and libs (/usr/lib/lpsolve)]),
	    with_lpsolve_include="${withval}")
	AC_MSG_CHECKING([for lpsolve includes])
	AC_CACHE_VAL(ac_cv_c_lpsolve_include,[

	    # First check to see if --with-lpsolve was specified.
	    if test x"${with_lpsolve_include}" != x ; then
		    if test -f "${with_lpsolve_include}/lp_lib.h" ; then
		        ac_cv_c_lpsolve_include="`(cd "${with_lpsolve_include}"; pwd)`"
		    else
		        AC_MSG_ERROR([${with_lpsolve_include} directory doesn't contain lp_lib.h])
		    fi
	    fi

	    # then check for a private Tcl installation
	    if test x"${ac_cv_c_lpsolve_include}" = x ; then
		    if test -f "/usr/include/lpsolve/lp_lib.h" ; then
                ac_cv_c_lpsolve_include=/usr/include/lpsolve
            fi
        fi

	])

	if test x"${ac_cv_c_lpsolve_include}" = x ; then
	    LPSOLVE_INCLUDES="# no lpsolve headers found"
	    AC_MSG_ERROR([Can't find lpsolve headers. Use --with-lpsolve to specify the directory containing lpsolve headers])
	else
	    LPSOLVE_INCLUDES="${ac_cv_c_lpsolve_include}"
	    AC_MSG_RESULT([found ${LPSOLVE_INCLUDES}/lp_lib.h])
        AC_SUBST(LPSOLVE_INCLUDES)
	fi
	
])

AC_DEFUN([FIND_LPSOLVE_LIBS], [
    dnl TEA specific: Make sure we are initialized
    AC_REQUIRE([TEA_INIT])
    #
    # Ok, lets find the lpsolve configuration
    # First, look for one uninstalled.
    # the alternative search directory is invoked by --with-lpsolve
    #

	AC_ARG_WITH(lpsolve-libs,
	    AC_HELP_STRING([--with-lpsolve-libs],
		[base directory for lpsolve libs (/usr/lib64)]),
	    with_lpsolve_libs="${withval}")
	AC_MSG_CHECKING([for lpsolve libs])
	AC_CACHE_VAL(ac_cv_c_lpsolve_libs,[

	    # First check to see if --with-lpsolve was specified.
	    if test x"${with_lpsolve_libs}" != x ; then
		    if test -f "${with_lpsolve_libs}/liblpsolve*.so" ; then
		        ac_cv_c_lpsolve_libs="`(cd "${with_lpsolve_libs}"; pwd)`"
		    else
		        AC_MSG_ERROR([${with_lpsolve_libs} directory doesn't contain liblpsolve.so])
		    fi
	    fi

	    # then check for a private Tcl installation
	    if test x"${ac_cv_c_lpsolve_libs}" = x ; then
            list="/usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64"
            for i in $list ; do
                if test -f $i/liblpsolve*.so ; then
                    ac_cv_c_lpsolve_libs=$i
                    break
                fi
            done
        fi

	])

	if test x"${ac_cv_c_lpsolve_libs}" = x ; then
	    LPSOLVE_LIBS_DIR="# no lpsolve headers found"
	    AC_MSG_ERROR([Can't find lpsolve libs. Use --with-lpsolve-libs to specify the directory containing liblpsolve<version>.so])
	else
	    LPSOLVE_LIB_DIR="${ac_cv_c_lpsolve_libs}"
        AC_SUBST(LPSOLVE_LIB_DIR)
        x="`(cd "${LPSOLVE_LIB_DIR}"; ls liblpsolve*)`"
        LPSOLVE_LIB=${x:3:9}
	    AC_MSG_RESULT([found ${LPSOLVE_LIB} in ${LPSOLVE_LIB_DIR}])
        AC_SUBST(LPSOLVE_LIB)
	fi
])
