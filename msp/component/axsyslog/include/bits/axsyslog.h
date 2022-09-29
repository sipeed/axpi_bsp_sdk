#ifndef _SYS_AXSYSLOG_H
# error "Never include <bits/axsyslog.h> directly; use <sys/axsyslog.h> instead."
#endif

void axsyslog (int __pri, const char* __tag, int __id, const char *__fmt, ...);

extern void __axsyslog_chk (int __pri, const char* __tag, int __id, int __flag, const char *__fmt, ...)
     __attribute__ ((__format__ (__printf__, 5, 6)));
#ifdef __va_arg_pack
__fortify_function void
axsyslog (int __pri, const char* __tag, int __id, const char *__fmt, ...)
{
  __axsyslog_chk (__pri, __tag, __id, __USE_FORTIFY_LEVEL - 1, __fmt, __va_arg_pack ());
}
#elif !defined __cplusplus
# define axsyslog(tag, pri, ...) \
  __axsyslog_chk (tag, pri, __USE_FORTIFY_LEVEL - 1, __VA_ARGS__)
#endif
extern void __axvsyslog_chk (int __pri, const char* __tag, int __id, int __flag, const char *__fmt,
			   __gnuc_va_list __ap)
     __attribute__ ((__format__ (__printf__, 5, 0)));

__fortify_function void
axvsyslog (int __pri, const char* __tag, int __id, const char *__fmt, __gnuc_va_list __ap)
{
  __axvsyslog_chk (__pri, __tag, __id,  __USE_FORTIFY_LEVEL - 1, __fmt, __ap);
}
