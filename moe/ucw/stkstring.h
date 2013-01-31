/*
 *	UCW Library -- Strings Allocated on the Stack
 *
 *	(c) 2005--2007 Martin Mares <mj@ucw.cz>
 *	(c) 2005 Tomas Valla <tom@ucw.cz>
 *	(c) 2008 Pavel Charvat <pchar@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_STKSTRING_H
#define _UCW_STKSTRING_H

#include <alloca.h>
#include <string.h>
#include <stdio.h>

#define stk_strdup(s) ({ const char *_s=(s); uns _l=strlen(_s)+1; char *_x=alloca(_l); memcpy(_x, _s, _l); _x; })
#define stk_strndup(s,n) ({ const char *_s=(s); uns _l=strnlen(_s,(n)); char *_x=alloca(_l+1); memcpy(_x, _s, _l); _x[_l]=0; _x; })
#define stk_strcat(s1,s2) ({ const char *_s1=(s1); const char *_s2=(s2); uns _l1=strlen(_s1); uns _l2=strlen(_s2); char *_x=alloca(_l1+_l2+1); memcpy(_x,_s1,_l1); memcpy(_x+_l1,_s2,_l2+1); _x; })
#define stk_strmulticat(s...) ({ char *_s[]={s}; char *_x=alloca(stk_array_len(_s, ARRAY_SIZE(_s)-1)); stk_array_join(_x, _s, ARRAY_SIZE(_s)-1, 0); _x; })
#define stk_strarraycat(s,n) ({ char **_s=(s); int _n=(n); char *_x=alloca(stk_array_len(_s,_n)); stk_array_join(_x, _s, _n, 0); _x; })
#define stk_strjoin(s,n,sep) ({ char **_s=(s); int _n=(n); char *_x=alloca(stk_array_len(_s,_n)+_n-1); stk_array_join(_x, _s, _n, (sep)); _x; })
#define stk_printf(f...) ({ uns _l=stk_printf_internal(f); char *_x=alloca(_l); sprintf(_x, f); _x; })
#define stk_vprintf(f, args) ({ uns _l=stk_vprintf_internal(f, args); char *_x=alloca(_l); vsprintf(_x, f, args); _x; })
#define stk_hexdump(s,n) ({ uns _n=(n); char *_x=alloca(3*_n+1); stk_hexdump_internal(_x,(char*)(s),_n); _x; })
#define stk_str_unesc(s) ({ const char *_s=(s); char *_d=alloca(strlen(_s)+1); str_unesc(_d, _s); _d; })
#define stk_fsize(n) ({ char *_s=alloca(16); stk_fsize_internal(_s, n); _s; })

uns stk_array_len(char **s, uns cnt);
void stk_array_join(char *x, char **s, uns cnt, uns sep);
uns stk_printf_internal(const char *x, ...) FORMAT_CHECK(printf,1,2);
uns stk_vprintf_internal(const char *x, va_list args);
void stk_hexdump_internal(char *dst, const byte *src, uns n);
void stk_fsize_internal(char *dst, u64 size);

#endif
