AC_DEFUN([GP_CHECK_FOLDBYLEVEL],
[
    GP_ARG_DISABLE([FoldByLevel], [auto])
    GP_CHECK_PLUGIN_GTK2_ONLY([FoldByLevel])
    GP_COMMIT_PLUGIN_STATUS([FoldByLevel])
    AC_CONFIG_FILES([
        foldbylevel/Makefile
        foldbylevel/src/Makefile
    ])
])
