#ifndef _SYS_AXSYSLOG_H
# error "Never include <bits/syslog-ldbl.h> directly; use <sys/syslog.h> instead."
#endif

__LDBL_REDIR_DECL (axsyslog)

#ifdef __USE_MISC
__LDBL_REDIR_DECL (axvsyslog)
#endif

#if __USE_FORTIFY_LEVEL > 0 && defined __fortify_function
__LDBL_REDIR_DECL (__axsyslog_chk)

# ifdef __USE_MISC
__LDBL_REDIR_DECL (__axvsyslog_chk)
# endif
#endif
