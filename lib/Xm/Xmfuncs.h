/*
 * $TOG: Xmfuncs.h /main/1 1997/03/24 16:25:46 dbl $
 * 
 * 
Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 *
 */

#ifndef _XFUNCS_H_
#define _XFUNCS_H_

#include <X11/Xosdefs.h>

/* the old Xfuncs.h, for pre-R6 */

#ifndef __sun

#ifdef X_USEBFUNCS
void bcopy();
void bzero();
int bcmp();
#else
#if (__STDC__ && !defined(X_NOT_STDC_ENV) && !defined(sun) && !defined(macII) && !defined(apollo)) || defined(SVR4) || defined(hpux) || defined(_IBMR2) || defined(_SEQUENT_)
#include <string.h>
#define _XFUNCS_H_INCLUDED_STRING_H
#ifndef __CYGWIN__
#define bcopy(b1,b2,len) memmove(b2, b1, (size_t)(len))
#define bzero(b,len) memset(b, 0, (size_t)(len))
#define bcmp(b1,b2,len) memcmp(b1, b2, (size_t)(len))
#endif
#else
#ifdef sgi
#include <bstring.h>
#else
#ifdef SYSV
#include <memory.h>
void bcopy();
#define bzero(b,len) memset(b, 0, len)
#define bcmp(b1,b2,len) memcmp(b1, b2, len)
#else /* bsd */
void bcopy();
void bzero();
int bcmp();
#endif /* SYSV */
#endif /* sgi */
#endif /* __STDC__ and relatives */
#endif /* X_USEBFUNCS */

/* the new Xfuncs.h */

#if !defined(X_NOT_STDC_ENV) && (!defined(sun) || defined(SVR4))
/* the ANSI C way */
#ifndef _XFUNCS_H_INCLUDED_STRING_H
#include <string.h>
#endif
#ifndef __CYGWIN__
#undef bzero
#define bzero(b,len) memset(b,0,len)
#endif
#else /* else X_NOT_STDC_ENV or SunOS 4 */
#if defined(SYSV) || defined(luna) || defined(sun) || defined(__sxg__)
#include <memory.h>
#define memmove(dst,src,len) bcopy((char *)(src),(char *)(dst),(int)(len))
#if defined(SYSV) && defined(_XBCOPYFUNC)
#undef memmove
#define memmove(dst,src,len) _XBCOPYFUNC((char *)(src),(char *)(dst),(int)(len))
#define _XNEEDBCOPYFUNC
#endif
#else /* else vanilla BSD */
#define memmove(dst,src,len) bcopy((char *)(src),(char *)(dst),(int)(len))
#define memcpy(dst,src,len) bcopy((char *)(src),(char *)(dst),(int)(len))
#define memcmp(b1,b2,len) bcmp((char *)(b1),(char *)(b2),(int)(len))
#endif /* SYSV else */
#endif /* ! X_NOT_STDC_ENV else */

#endif /* __sun */

#endif /* _XFUNCS_H_ */
