/*****************************************************************************
 *       INCLUDE FILES
 *****************************************************************************/

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <stdio.h>
#include <ctype.h>

/*
 * Include stdlib.h and malloc.h if code is C++, ANSI, or Extended ANSI.
 */
#if defined(__cplusplus) || defined(__STDC__) || defined(__EXTENSIONS__)
#  include <stdlib.h>
#  if defined(HAVE_MALLOC_H)
#  include <malloc.h>
#  elif defined(HAVE_SYS_MALLOC_H)
#  include <sys/malloc.h>
#  endif
#endif


/*****************************************************************************
 *       TYPDEFS AND DEFINES
 *****************************************************************************/

/*
 * Undefine this if you want to use native strcasecmp.
 */
#define LOCAL_STRCASECMP

#ifdef _NO_PROTO
#ifdef NeedFunctionPrototypes
#undef NeedFunctionPrototypes
#endif
#endif

/*
 * Define SUPPORTS_WCHARS if the system supports wide character sets
 */
#if !defined(VAX) && !defined(__CENTERLINE__)
#define SUPPORTS_WCHARS
#endif

/*
 * Handy definition used in SET_BACKGROUND_COLOR
*/
#define UNSET		(-1)

/*
 * Set state of inclusion of prototypes properly
 */
#ifdef NeedFunctionPrototypes
#define ARGLIST(p)	(
#define ARG(a, b)	a b,
#define GRA(a, b)	a b)
#else
#define ARGLIST(p)	p
#define ARG(a, b)	a b;
#define GRA(a, b)	a b;
#endif

#ifdef NeedFunctionPrototypes
#ifdef __cplusplus
#define UARG(a, b)	a,
#define GRAU(a, b)	a)
#else
#define UARG(a, b)	a b,
#define GRAU(a, b)	a b)
#endif
#else
#define UARG(a, b)	a b;
#define GRAU(a, b)	a b;
#endif

/*
 * Set up strcasecmp function
 */
#if defined(LOCAL_STRCASECMP)
#define STRCASECMP	StrCasecmp
#ifndef NeedFunctionPrototypes
static int StrCasecmp();
#else
static int StrCasecmp(char*, char*);
#endif
#else
#define STRCASECMP	strcasecmp
#endif

/*
 * Define XTPOINTER so it works with all releases of
 * Xt and c++.
 */
#ifdef __cplusplus
#if XtSpecificationRelease < 5
#define XTPOINTER	char *
#else
#define XTPOINTER	XPointer
#endif
#else
#define XTPOINTER	XtPointer
#endif

/*
 * The following enum is used to support wide character sets.
 * Use this enum for references into the Common Wide Characters array.
 * If you add to the array, ALWAYS keep NUM_COMMON_WCHARS as the last
 * entry in the enum.  This will maintain correct memory usage, etc.
 */
enum { WNull, WTab, WNewLine, WCarriageReturn, WFormFeed, WVerticalTab,
       WBackSlash, WQuote, WHash, WColon, WideF, WideL, WideN, WideR,
       WideT, WideV, WideUF, WideUL, WideUR, WideUT, WideZero, WideOne,
       NUM_COMMON_WCHARS };

/*****************************************************************************
 *       GLOBAL DECLARATIONS
 *****************************************************************************/

/*****************************************************************************
 *       EXTERNAL DECLARATIONS
 *****************************************************************************/

/*****************************************************************************
 *	STATIC DECLARATION
 *****************************************************************************/

#ifndef NeedFunctionPrototypes

#ifndef SUPPORTS_WCHARS
static int	 mblen			();
#endif
static int	strlenWc		();
static size_t	doMbstowcs		();
static size_t	doWcstombs 		();
static void	copyWcsToMbs		();
static int 	dombtowc		();
static Boolean	extractSegment		();
static XmString	StringToXmString	();
static char*	getNextCStrDelim	();
static int	getCStrCount		();
static wchar_t *CStrCommonWideCharsGet	();

#else

#ifndef SUPPORTS_WCHARS
static int 	mblen			(char*, size_t);
#endif
static int	strlenWc		(wchar_t*);
static size_t	doMbstowcs		(wchar_t*, char*, size_t);
static size_t	doWcstombs 		(char*, wchar_t*, size_t);
static void	copyWcsToMbs		(char*, wchar_t*, int, Boolean);
static int 	dombtowc		(wchar_t*, char*, size_t);
static Boolean	extractSegment		(wchar_t**, wchar_t**, int *,
					 wchar_t**, int*, int*,	Boolean*);
static XmString	StringToXmString	(char*);
static char*	getNextCStrDelim	(char*);
static int	getCStrCount		(char*);
static wchar_t *CStrCommonWideCharsGet	();

#endif

/*****************************************************************************
 *	STATIC CODE
 *****************************************************************************/

#if defined(LOCAL_STRCASECMP) 

/*
 * Function:
 *      cmp = StrCasecmp(s1, s2);
 * Description:
 *      Compare two strings ignoring case
 * Input:
 *      s1 - char * : string 1 to compare
 *      s2 - char * : string 2 to compare
 * Output:
 *      int :  0; s1 == s2
 *             1; s1 != s2
 */
static int StrCasecmp
    ARGLIST((s1, s2))
        ARG(register char *, s1)
        GRA(register char *, s2)
{
    register int        c1, c2;
    
    while (*s1 && *s2)
    {
        c1 = isupper(*s1) ? tolower(*s1) : *s1;
        c2 = isupper(*s2) ? tolower(*s2) : *s2;
        if (c1 != c2)
        {
            return(1);
        }
        s1++;
        s2++;
    }
    if (*s1 || *s2)
    {
        return(1);
    }
    return(0);
}
#endif

#ifndef SUPPORTS_WCHARS
/*
 * Function:
 *      len = mblen(s, n);
 * Description:
 *      The mblen function for platforms that don't have one. This
 * 	function simply returns a length of 1 since no wide character
 *	support exists for this platform.
 * Input:
 *      s - char * : the character string to get the length from
 *	n - size_t : the size of the string
 * Output:
 *      int : always 1
 */
static int mblen
    ARGLIST((s, n))
        ARG(char *, s)
        GRA(size_t, n)
{
    return(1);
}
#endif

/*
 * Function:
 *      len = strlenWc(ptr);
 * Description:
 *      Return the number of characters in a wide character string (not
 *	the characters in the resultant mbs).
 * Input:
 *      ptr - wchar_t* : pointer to the wcs to count
 * Output:
 *      int : the number of characters found
 */
static int strlenWc
    ARGLIST((ptr))
        GRA(wchar_t *,ptr)
{
    register wchar_t	*p = ptr;
    register int	x = 0;
    
    if (!ptr) return(0);
    
    while (*p++) x++;
    return (x);
}

/*
 * Function:
 *      bytesConv = doMbstowcs(wcs, mbs, n);
 * Description:
 *      Create a wcs string from an input mbs. 
 * Input:
 *	wcs - wchar_t* : pointer to result buffer of wcs
 *      mbs - char* : pointer to the source mbs
 *	n - size_t : the number of characters to convert
 * Output:
 *      bytesConv - size_t : number of bytes converted
 */
static size_t doMbstowcs
    ARGLIST((wcs, mbs, n))
        ARG(wchar_t *,wcs)
        ARG(char *, mbs)
        GRA(size_t, n)
{
#ifndef SUPPORTS_WCHARS
    int i;
    
    for (i = 0; i < n && mbs[i] != 0; ++i)
    {
	wcs[i] = mbs[i];
    }
    wcs[i++] = 0;
    return(i);
#else
    return(mbstowcs(wcs, mbs, n));
#endif
}

/*
 * Function:
 *      bytesConv = doWcstombs(wcs, mbs, n);
 * Description:
 *      Create a mbs string from an input wcs.
 * Input:
 *	wcs - wchar_t* : pointer to the source wcs
 *      mbs - char* : pointer to result mbs buffer 
 *	n - size_t : the number of characters to convert
 * Output:
 *      bytesConv - size_t : number of bytes converted
 */
static size_t doWcstombs
    ARGLIST((mbs, wcs, n))
        ARG(char *, mbs)
        ARG(wchar_t *, wcs)
        GRA(size_t, n)
{
#ifndef SUPPORTS_WCHARS
    int i;
    
    for (i = 0; i < n && wcs[i] != 0; ++i)
    {
	mbs[i] = wcs[i];
    }
    mbs[i] = 0;
    return(i);
#else
    return(wcstombs(mbs, wcs, n));
#endif
}

/*
 * Function:
 *      copyWcsToMbs(mbs, wcs, len);
 * Description:
 *      Create a mbs string from an input wcs. This function allocates
 *	a buffer if necessary.
 * Input:
 *	mbs - char* : destination for the converted/copied output
 *	wcs - wchar_t* : pointer to wcs to copy/convert
 *	len - int : the number of wchar_t' to convert
 *	process_it - Boolean : True if processing of quoted charcters,
 *			False if blind.
 * Output:
 *      None
 */
static void copyWcsToMbs
    ARGLIST((mbs, wcs, len, process_it))
        ARG(char *, mbs)
        ARG(wchar_t *, wcs)
        ARG(int, len)
        GRA(Boolean, process_it)
{
    static	wchar_t	*tbuf;
    static	int	tbufSize = 0;
    
    int		numCvt;
    int		lenToConvert;
    wchar_t	*fromP = wcs;
    wchar_t	*x = &fromP[len];
    wchar_t	*toP;
    wchar_t	*commonWChars = CStrCommonWideCharsGet();
    wchar_t	tmp;
    
    /*
     * Make sure there's room in the buffer
     */
    if (tbufSize < len)
    {
	tbuf = (wchar_t*)XtRealloc((char*)tbuf, (len + 1) * sizeof(wchar_t));
	tbufSize = len;
    }
    
    /*
     * Now copy and process
     */
    toP = tbuf;
    lenToConvert = 0;
    while (fromP < x)
    {
	/*
	 * Check for quoted characters
	 */
	if ((*fromP == commonWChars[WBackSlash]) && process_it)
	{
	    fromP++;		/* Skip quote */
	    if (fromP == x)	/* Hanging quote? */
	    {
		*toP++ = commonWChars[WBackSlash];
		lenToConvert++;
		break;
	    }
	    tmp = *fromP++;
	    if (tmp == commonWChars[WideN])
	    {
		*toP++ = commonWChars[WNewLine];
	    }
	    else if (tmp == commonWChars[WideT])
	    {
		*toP++ = commonWChars[WTab];
	    }
	    else if (tmp == commonWChars[WideR])
	    {
		*toP++ = commonWChars[WCarriageReturn];
	    }
	    else if (tmp == commonWChars[WideF])
	    {
		*toP++ = commonWChars[WFormFeed];
	    }
	    else if (tmp == commonWChars[WideV])
	    {
		*toP++ = commonWChars[WVerticalTab];
	    }
	    else if (tmp == commonWChars[WBackSlash])
	    {
		*toP++ = commonWChars[WBackSlash];
	    }
	    else
	    {
                /*
		 * No special translation needed
		 */
		*toP++ = tmp;
	    }
	}
	else
	{
	    *toP++ = *fromP++;
	}
	lenToConvert++;
    }
    
    numCvt = doWcstombs(mbs, tbuf, lenToConvert);
    mbs[numCvt] = '\0';
}

/*
 * Function:
 *      status = dombtowc(wide, multi, size);
 * Description:
 *      Convert a multibyte character to a wide character.
 * Input:
 *      wide	- wchar_t *	: where to put the wide character
 *	multi	- char *	: the multibyte character to convert
 *	size	- size_t	: the number of characters to convert
 * Output:
 *      0	- if multi is a NULL pointer or points to a NULL character
 *	#bytes	- number of bytes in the multibyte character
 *	-1	- multi is an invalid multibyte character.
 *
 *	NOTE:  if wide is NULL, then this returns the number of bytes in
 *	       the multibyte character.
 */
static int dombtowc
    ARGLIST((wide, multi, size))
        ARG(wchar_t *, wide)
        ARG(char *, multi)
        GRA(size_t, size)
{
    int		retVal = 0;
    
#ifndef SUPPORTS_WCHARS
    if ((multi == NULL) || (*multi == '\000'))
    {
	if (wide) wide[0] = '\0';
	return (0);
    }
    
    for (retVal = 0; retVal < size && multi[retVal] != '\000'; retVal++)
    {
	if (wide != NULL)
	{
	    wide[retVal] = multi[retVal];
	}
    }
#else
    retVal = mbtowc(wide, multi, size);
#endif
    return(retVal);
}

/*
 * Function:
 *	ptr = getNextSepartor(str);
 * Description:
 *	Parse through a string looking for the next compound string
 *	field separator
 * Inputs:
 *	str - wchar_t* : the address of address of the string to parse
 * Outputs:
 *	ptr - wchar_t* : pointer to character, if found, points to end
 *			of string otherwise ('\0').
 */
static wchar_t* getNextSeparator
    ARGLIST((str))
        GRA(wchar_t *, str)	
{
    wchar_t	*ptr = str;
    wchar_t	*commonWChars = CStrCommonWideCharsGet();
    
    while (*ptr)
    {
	/*
	 * Check for separator
	 */
	if ((*ptr == commonWChars[WHash]) ||
	    (*ptr == commonWChars[WQuote]) ||
	    (*ptr == commonWChars[WColon]))
	{
	    return(ptr);
	}
	else if (*ptr == commonWChars[WBackSlash])
	{
	    ptr++;
	    if (*ptr) ptr++;	/* Skip quoted character */
	}
	else
	{
	    ptr++;
	}
    }
    return(ptr);
}

/*
 * Function:
 *	more =
 *        extractSegment(str, tagStart, tagLen, txtStart, txtLen, 
 *			pDir, pSep);
 * Description:
 *	Parse through a string version of a compound string and extract
 *	the first compound string segment from the string.
 * Inputs:
 *	str - char** : the address of address of the string to parse
 *	tagStart - char** : address to return pointer to tag start into 
 *	tagLen - int* : address where to return the tag length into
 *	txtStart - char** : address to return the text start into
 *	txtLen - int* : address where to return the text length
 *	pDir - int* : address to return the string direction into
 *	pSep - Boolean * : address to return the separtor into
 * Outputs:
 *	more - Boolean : True if more of the string to parse.
 *			False means done.
 */
static Boolean extractSegment
    ARGLIST((str, tagStart, tagLen, txtStart, txtLen, pDir, pSep))
        ARG(wchar_t **, str)
        ARG(wchar_t **, tagStart)
        ARG(int *, tagLen)
        ARG(wchar_t **, txtStart)
        ARG(int *, txtLen)
        ARG(int *, pDir)
        GRA(Boolean *, pSep)
{
    wchar_t		*start;
    wchar_t		*text;
    int			textL;
    Boolean		tagSeen;
    wchar_t		*tag;
    int			tagL;
    Boolean		modsSeen;
    Boolean		sep;
    int			dir;
    Boolean		done;
    int			*lenUp;
    Boolean		checkDir;
    wchar_t		*commonWChars;
    wchar_t		emptyStrWcs[1];

    /*
     * Initialize variables
     */
    text = NULL;
    textL = 0;
    tagSeen = False;
    tag = NULL;
    tagL = 0;
    modsSeen = False;
    dir = XmSTRING_DIRECTION_L_TO_R;
    sep = False;
    done = False;
    lenUp = NULL;
    commonWChars = CStrCommonWideCharsGet();

    /*
     * Guard against nulls
     */
    if (!(start = *str))
    {
	start = emptyStrWcs;
	emptyStrWcs[0] = commonWChars[WNull];
    }

    /*
     * If the first character of the string isn't a # or a ", then we
     * just have a regular old simple string. Do the same the thing for
     * the empty string.
     */
    if ((*start == '\0') || (start != getNextSeparator(start)))
    {
	text = start;
	if (!(textL = strlenWc(start)))
	{
	    text = NULL;
	}
	start += textL;
    }
    else
    {
	done = False;
	while (!done)
	{
	    if (*start == commonWChars[WHash])
	    {
		if (tagSeen)
		{
		    done = True;
		    break;
		}
		else
		{
		    tagSeen = True;
		    tag = ++start;
		    start = getNextSeparator(tag);
		    if ((tagL = start - tag) == 0)
		    {
			tag = NULL;		/* Null tag specified */
		    }
		}
	    }
	    else if (*start == commonWChars[WQuote])
	    {
		text = ++start;
		start = getNextSeparator(start);
		while (!((*start == commonWChars[WQuote]) ||
			 (*start == commonWChars[WNull])))
		{
		    start = getNextSeparator(++start);
		}
		
		if ((textL = start - text) == 0)
		{
		    text = NULL;	/* Null text specified  */
		}
                /*
		 * if a quote, skip over it
		 */
		if (*start == commonWChars[WQuote])
		{
		    start++;
		}
		done = True;
	    }
	    else if (*start == commonWChars[WColon])
	    {
		if (modsSeen)
		{
		    done = True;
		    break;
		}
		
		/*
		 * If the next character is a t or f, the we've got 
		 * a separator.
		 */
		modsSeen = True;
		checkDir = False;
		start++;
		if ((*start == commonWChars[WideT]) ||
		    (*start == commonWChars[WideUT]) ||
		    (*start == commonWChars[WideOne]))
		{
		    sep = True;
		    start++;
		    checkDir = True;
		}
		else if ((*start == commonWChars[WideF]) ||
			 (*start == commonWChars[WideUF]) ||
			 (*start == commonWChars[WideZero]))
		{
		    sep = False;
		    start++;
		    checkDir = True;
		}
		else if ((*start == commonWChars[WideR]) ||
			 (*start == commonWChars[WideUR]))
		{
		    start++;
		    dir = XmSTRING_DIRECTION_R_TO_L;
		}
		else if ((*start == commonWChars[WideL]) ||
			 (*start == commonWChars[WideUL]))
		{
		    start++;
		    dir = XmSTRING_DIRECTION_L_TO_R;
		}
		/*
		 * Look for direction if necessary. This requires a bit of
		 * look ahead.
		 */
		if (checkDir && (*start == commonWChars[WColon]))
		{
		    if ((*(start + 1) == commonWChars[WideL]) ||
			(*(start + 1) == commonWChars[WideUL]))
		    {
			dir = XmSTRING_DIRECTION_L_TO_R;
			start += 2;
		    }
		    else if ((*(start + 1) == commonWChars[WideR]) ||
			     (*(start + 1) == commonWChars[WideUR]))
		    {
			dir = XmSTRING_DIRECTION_R_TO_L;
			start+=2;
		    }
		}
	    }
            else
	    {
		/*
		 * A bad string format! We'll just skip the character.
		 */
		start++;
	    }
	}
    }

    /*
     * Now fill in return values
     */
    if (*str)		*str = start;
    if (tagStart)	*tagStart = tag;
    if (tagLen)		*tagLen = tagL;
    if (txtStart)	*txtStart = text;
    if (txtLen)		*txtLen = textL;
    if (pDir)		*pDir = dir;
    if (pSep)		*pSep = sep;

    return ((*start == commonWChars[WNull]) ? False : True);
}

/*
 * Function:
 *	xstr = StringToXmString(str);
 * Description:
 *	Parse a string into an XmString.
 * Inputs:
 *	str - char * : the string to parse
 * Outputs:
 *	xstr - XmString : the allocated return structure
 */
static XmString StringToXmString
    ARGLIST((str))
        GRA(char *,str)
{
    static char*	tagBuf = NULL;
    static int		tagBufLen = 0;
    static char*	textBuf = NULL;
    static int		textBufLen = 0;

    wchar_t		*ctx;
    wchar_t		*tag;
    int			tagLen;
    wchar_t		*text;
    int			textLen;
    Boolean		sep;
    int			dir;
    
    Boolean		more;
    wchar_t		*wcStr;
    int			curDir;
    XmString		xmStr;
    XmString		s1;
    XmString		s2;

    if (!str) return(NULL);

    /*
     * For expediencies sake, we'll overallocate this buffer so that
     * the wcs is guaranteed to fit (1 wc per byte in original string).
     */
    wcStr = (wchar_t*)XtMalloc((strlen(str) + 1) * sizeof(wchar_t));
    doMbstowcs(wcStr, str, strlen(str) + 1);

    /*
     * Create the beginning segment
     */
    curDir = XmSTRING_DIRECTION_L_TO_R;
    xmStr = XmStringDirectionCreate(curDir);

    /*
     * Convert the string.
     */
    more = True;
    ctx = wcStr;
    while (more)
    {
	more = extractSegment(&ctx, &tag, &tagLen,
			      &text, &textLen, &dir, &sep);
	/*
	 * Pick up a direction change
	 */
	if (dir != curDir)
	{
#if defined(VMS) || (defined(__osf__) && defined(__alpha))
	    /*
	     * This is required on DEC Windows systems because they've
	     * added the REVERT direction.
	     */
	    s1 = XmStringDirectionCreate(XmSTRING_DIRECTION_REVERT);
	    s2 = xmStr;
	    xmStr = XmStringConcat(s2, s1);
	    XmStringFree(s1);
	    XmStringFree(s2);
#endif
	    curDir = dir;
	    s1 = XmStringDirectionCreate(curDir);
	    s2 = xmStr;
	    xmStr = XmStringConcat(s2, s1);
	    XmStringFree(s1);
	    XmStringFree(s2);

	}

	/*
	 * Create the segment. Text and tag first.
	 */
	if (textLen)
	{
	    if (textBufLen <= (textLen * sizeof(wchar_t)))
	    {
		textBufLen = (textLen + 1) * sizeof(wchar_t);
		textBuf = (char*)XtRealloc(textBuf, textBufLen);
	    }
	    copyWcsToMbs(textBuf, text, textLen, True);

	    if (tagLen)
	    {
		if (tagBufLen <= (tagLen * sizeof(wchar_t)))
		{
		    tagBufLen = (tagLen + 1) * sizeof(wchar_t);
		    tagBuf = (char*)XtRealloc(tagBuf, tagBufLen);
		}
		copyWcsToMbs(tagBuf, tag, tagLen, False);
	    }
	    else
	    {
		if (!tagBuf)
		{
		    tagBufLen = strlen(XmSTRING_DEFAULT_CHARSET) + 1;
		    tagBuf = (char*)XtMalloc(tagBufLen);
		}
		strcpy(tagBuf, XmSTRING_DEFAULT_CHARSET);
	    }

	    s1 = XmStringCreate(textBuf, tagBuf); 
	    s2 = xmStr;
	    xmStr = XmStringConcat(s2, s1);
	    XmStringFree(s1);
	    XmStringFree(s2);
	}

	/*
	 * Add in the separators.
	 */
	if (sep)
	{
	    s1 = XmStringSeparatorCreate();
	    s2 = xmStr;
	    xmStr = XmStringConcat(s2, s1);
	    XmStringFree(s1);
	    XmStringFree(s2);
	}
    }
    
    /*
     * Free up memory and return
     */
    XtFree((char*)wcStr);
    return(xmStr);
}

/*
 * Function:
 *      nextCStr = getNextCStrDelim(str);
 * Description:
 *      Find the next unquoted , or \n in the string
 * Input:
 *	str - char * : the input string
 * Output:
 *      nextCStr - char* : pointer to the next delimiter. Returns NULL if no
 *			delimiter found.
 */
static char* getNextCStrDelim
    ARGLIST((str))
        GRA(char *,str)
{
    char	*comma = str;
    Boolean	inQuotes = False;
    int		len;

    if (!str) return(NULL);
    if (!*str) return(NULL);	/* At end */

#ifdef __CENTERLINE__
    mblen((char *)NULL, sizeof(wchar_t));
#else
    mblen(NULL, sizeof(wchar_t));
#endif
    while (*comma)
    {
	if ((len = mblen(comma, sizeof(wchar_t))) > 1)
	{
	    comma += len;
	    continue;
	}
	
	if (*comma == '\\')
	{
	    comma++;	/* Over quote */
	    comma += mblen(comma, sizeof(wchar_t));
	    continue;
	}

	/*
	 * See if we have a delimiter
	 */
	if (!inQuotes)
	{
	    if ((*comma == ',') || (*comma == '\012'))
	    {
		return(comma);
	    }
	}

	/*
	 * Deal with quotes
	 */
	if (*comma == '\"')
	{
	    inQuotes = ~inQuotes;
	}

	comma++;
    }

    return(NULL);		/* None found */
}

/*
 * Function:
 *	cnt = getCStrCount(str);
 * Description:
 *      Get the count of cstrings in a compound string table ascii
 *	format.
 * Input:
 *      str - char * : string to parse
 * Output:
 *      cnt - int : the number of XmStrings found
 */
static int getCStrCount
    ARGLIST((str))
        GRA(char *, str)
{
    int		x = 1;
    char	*newStr;

    if (!str) return(0);
    if (!*str) return(0);

    while ((newStr = getNextCStrDelim(str)))
    {
	x++;
	str = ++newStr;
    }
    return(x);
}

/*
 * Function:
 *      cwc = CStrCommonWideCharsGet();
 * Description:
 *      Return the array of common wide characters.
 * Input:
 *      None.
 * Output:
 *     	cwc - wchar_t * : this array should never be written to or FREEd.
 */
static wchar_t *CStrCommonWideCharsGet()
{
    static wchar_t	*CommonWideChars = NULL;
    /*
     * If you add to this array, don't forget to change the enum in
     * the TYPEDEFS and DEFINES section above to correspond to this
     * array.
     */
    static char	*characters[] = { "\000", "\t", "\n", "\r", "\f", "\v",
				  "\\", "\"", "#", ":", "f", "l", "n", "r",
				  "t", "v", "F", "L", "R", "T", "0", "1" };
	

    if (CommonWideChars == NULL)
    {
	int	i;

	/*
	 * Allocate and create the array.
	 */
	CommonWideChars = (wchar_t*)XtMalloc(NUM_COMMON_WCHARS * sizeof(wchar_t));
	
	for (i = 0; i < NUM_COMMON_WCHARS; i++)
	{
	    (void)dombtowc(&(CommonWideChars[i]), characters[i], 1);
	}
    }
    return(CommonWideChars);
}

/*
 * Function:
 *	CONVERTER CvtStringToXmString
 *           and
 *	XmStringCvtDestroy
 * Description:
 *	Convert a string to an XmString. This allows a string contained in
 *	resource file to contain multiple fonts. The syntax for the string
 *	is:
 *		::[#[font-tag]]"string"[#[font-tag]"string"] ...
 *
 *	note that the # can be escaped (\#).
 *
 * Input:
 * Output:
 *	Standard.
 */
static Boolean CvtStringToXmString
    ARGLIST((d, args, num_args, fromVal, toVal, data))
        ARG(Display *, d)
        UARG(XrmValue *, args)
        ARG(Cardinal *, num_args)
        ARG(XrmValue *, fromVal)
        ARG(XrmValue *, toVal)
        GRAU(XtPointer, data)
{
    static XmString	resStr;
    char		*str;

    /*
     * This converter takes no parameters
     */
    if (*num_args != 0)
    {
	XtAppWarningMsg(XtDisplayToApplicationContext(d), 
			"cvtStringToXmString",
			"wrongParameters",
			"XtToolkitError",
			"String to XmString converter needs no extra arguments",
			(String *)NULL,
			(Cardinal *)NULL);
    }

    /*
     * See if this is a simple string
     */
    str = (char*)fromVal->addr;
    if (strncmp(str, "::", 2))
    {
	resStr = XmStringCreateLtoR(fromVal->addr, XmSTRING_DEFAULT_CHARSET);
    }
    else
    {
	/*
	 * Convert into internal format
	 */
	resStr = StringToXmString(fromVal->addr + 2);	/* skip :: */
    }

    /*
     * Done, return result
     */
    if (toVal->addr == NULL)
    {
	toVal->addr = (XTPOINTER)&resStr;
	toVal->size = sizeof(XmString);
    }
    else if (toVal->size < sizeof(XmString))
    {
	toVal->size = sizeof(XmString);
	XtDisplayStringConversionWarning(d, fromVal->addr, "XmString");
	XmStringFree(resStr);
	return(False);
    }
    else 
    {
	*(XmString *)toVal->addr = resStr;
	toVal->size = sizeof(XmString);
    }
    return(True);
}
static void XmStringCvtDestroy
    ARGLIST((app, to, data, args, num_args))
        UARG(XtAppContext, app)
        ARG(XrmValue *, to)
        UARG(XtPointer, data)
        UARG(XrmValue *, args)
        GRAU(Cardinal *, num_args)
{
    XmStringFree(*(XmString*)(to->addr));
}

/*
 * Function:
 *      CONVERTER CvtStringToXmStringTable
 *          and
 *      XmStringTableCvtDestroy
 *
 * Description:
 *	Convert a string to an XmString table. This allows a string contained in
 *	resource file to contain multiple fonts. The syntax for the string
 *	is:
 *	   compound_string = [#[font-tag]]"string"[#[font-tag]"string"] ...
 *	   compound_string_table = [compound_string][,compound_string] ...
 *
 *	note that the # can be escaped (\#).
 *
 * Input:
 * Output:
 *	Standard.
 */
static Boolean CvtStringToXmStringTable
    ARGLIST((d, args, num_args, fromVal, toVal, data))
        ARG(Display *, d)
        ARG(XrmValue *, args)
        ARG(Cardinal *, num_args)
        ARG(XrmValue *, fromVal)
        ARG(XrmValue *, toVal)
        GRAU(XtPointer, data)
{
    static XmString	*CStrTable;
    XmString		*tblPtr;
    char		*str;
    char		*tmpBuf;
    char		*nextDelim;
    XrmValue		fVal;
    XrmValue		tVal;

    /*
     * This converter takes no parameters
     */
    if (*num_args != 0)
    {
	XtAppWarningMsg
	    (XtDisplayToApplicationContext(d), 
	     "cvtStringToXmStringTable",
	     "wrongParameters",
	     "XtToolkitError",
	     "String to XmStringTable converter needs no extra arguments",
	     (String *)NULL,
	     (Cardinal *)NULL);
    }

    /*
     * Set str and make sure there's somethin' there
     */
    if (!(str = (char*)fromVal->addr))
    {
	str = "";
    }

    /*
     * Allocate the XmStrings + 1 for NULL termination
     */
    CStrTable = (XmString*)XtMalloc((getCStrCount(str) + 1) * sizeof(XmString*));

    /*
     * Use the string converter for the strings
     */
    tmpBuf = (char*)XtMalloc(strlen(str) + 1);
    strcpy(tmpBuf, str);
    str = tmpBuf;

    /*
     * Create strings
     */
    tblPtr = CStrTable;
    if (*str)
    {
	while (str)
	{
	    nextDelim = getNextCStrDelim(str);
	    
	    /*
	     * Overwrite nextDelim
	     */
	    if (nextDelim)
	    {
		*nextDelim = '\0';
		nextDelim++;
	    }
	    
	    /*
	     * Convert it
	     */
	    fVal.size = strlen(str) + 1;
	    fVal.addr = str;
	    tVal.size = sizeof(XTPOINTER);
	    tVal.addr = (XTPOINTER)tblPtr;
	    
	    /*
	     * Call converter ourselves since this is used to create
	     * the strings in the table we create. We need to do this
	     * since we don't have a widget to send to the XtConvertAndStore
	     * function. Side effects are that we can never get these
	     * compound strings cached and that no destructor function is
	     * called when the strings leave existance, but we nuke 'em
	     * in the XmStringTable destuctor.
	     */
	    CvtStringToXmString(d, args, num_args, &fVal, &tVal, NULL);
	    tblPtr++;
	    str = nextDelim;
	}
    }
    XtFree(tmpBuf);

    /*
     * Null terminate
     */
    *tblPtr = NULL;

    /*
     * Done, return result
     */
    if (toVal->addr == NULL)
    {
	toVal->addr = (XTPOINTER)&CStrTable;
	toVal->size = sizeof(XmString);
    }
    else if (toVal->size < sizeof(XmString*))
    {
	toVal->size = sizeof(XmString*);
	XtDisplayStringConversionWarning(d, fromVal->addr, "XmStringTable");

	tblPtr = CStrTable;
	while (*tblPtr)
	{
	    XmStringFree(*tblPtr);
	}
	XtFree((char*)CStrTable);
	return(False);
    }
    else 
    {
	*(XmString **)toVal->addr = CStrTable;
	toVal->size = sizeof(XmString*);
    }
    return(True);
}
static void XmStringTableCvtDestroy
    ARGLIST((app, to, data, args, num_args))
        UARG(XtAppContext, app)
        ARG(XrmValue *, to)
        UARG(XtPointer, data)
        UARG(XrmValue *, args)
        GRAU(Cardinal *, num_args)
{
    XmString	*tblPtr = *(XmString**)(to->addr);

    while (*tblPtr)
    {
	XmStringFree(*tblPtr);
    }
    XtFree((char*)(*(XmString**)(to->addr)));
}    

/*****************************************************************************
 *	GLOBAL CODE
 *****************************************************************************/

/*
 * Function:
 *      RegisterBxConverters(appContext);
 * Description:
 *      This globally available function installs all the converters necessary
 *	to run BuilderXcessory generated interfaces that use compound
 *	strings. This is necessary since Motif has not supplied very smart
 *	converters.
 * Input:
 *      appContext - XtAppContext : the application context
 * Output:
 *      None
 */
void RegisterBxConverters
    ARGLIST((appContext))
        GRA(XtAppContext, appContext)
{
    XtAppSetTypeConverter(appContext, XmRString, XmRXmString,
			  (XtTypeConverter)CvtStringToXmString,
			  NULL, 0, XtCacheNone, XmStringCvtDestroy);

    XtAppSetTypeConverter(appContext, XmRString, XmRXmStringTable,
			  (XtTypeConverter)CvtStringToXmStringTable,
			  NULL, 0, XtCacheNone, XmStringTableCvtDestroy);
}

/*
 * Function:
 *      CONVERT(w, from_string, to_type, to_size, success);
 * Description:
 *      A converter wrapper for convenience from BuilderXcessory.
 * Input:
 *      w - Widget : the widget to use for conversion
 *	from_string - char * : the string to convert from
 *	to_type - char * : the type to convert to
 *	to_size - int : the size of the conversion result
 *	success - Boolean* : Set to the result value of the conversion
 * Output:
 *      None
 */
#ifndef IGNORE_CONVERT
XtPointer CONVERT
    ARGLIST((w, from_string, to_type, to_size, success))
        ARG(Widget, w)
        ARG(char *, from_string)
        ARG(char *, to_type)
        ARG(int, to_size)
        GRA(Boolean *, success)
{
    XrmValue		fromVal, toVal;	/* resource holders		*/
    Boolean		convResult;	/* return value			*/
    XtPointer		val;		/* Pointer size return value    */

    to_size = 0;

    /*
     * We will assume that the conversion is going to fail and change this
     * value later if the conversion is a success.
     */
    *success = False;

    /*
     * Since we are converting from a string to some type we need to
     * set the fromVal structure up with the string information that
     * the caller passed in.
     */
    fromVal.size = strlen(from_string) + 1;
    fromVal.addr = from_string;

    /*
     * Since we are not sure what type and size of data we are going to
     * get back we will set this up so that the converter will point us
     * at a block of valid data.
     */
    toVal.size = 0;
    toVal.addr = NULL;

    /*
     * Now lets try to convert this data by calling this handy-dandy Xt
     * routine.
     */
    convResult = XtConvertAndStore(w, XmRString, &fromVal, to_type, &toVal);
    

    /*
     * Now we have two conditions here.  One the conversion was a success
     * and two the conversion failed.
     */
    if(!convResult)
    {
	/*
	 * If this conversion failed that we can pretty much return right
	 * here because there is nothing else we can do.
	 */
	return((XtPointer) NULL);
    }

    /*
     * If we get this far that means we did the conversion and all is
     * well.  Now we have to handle the special cases for type and
     * size constraints.
     */
    if(!strcmp(to_type, "String"))
    {
	/*
	 * Since strings are handled different in Xt we have to deal with
	 * the conversion from a string to a string.  When this happens the
	 * toVal.size will hold the strlen of the string so generic
	 * conversion code can't handle it.  It is possible for a string to
	 * string conversion to happen so we do have to watch for it.
	 */
	val = (XTPOINTER)toVal.addr;
    }
    else
    {
	/*
	 * Here is the generic conversion return value handler.  This 
	 * just does some size specific casting so that value that we
	 * return is in the correct bytes of the XtPointer that we
	 * return.  Here we check all sizes from 1 to 8 bytes.
	 */
	switch(toVal.size)
	{
	case 1:
	    val = (XTPOINTER)(*(char*)toVal.addr);
	    break;
	case 2:
	    val = (XTPOINTER)(*(short*)toVal.addr);
	    break;
	case 4:
	    val = (XTPOINTER)(*(int*)toVal.addr);
	    break;
	case 8:
	default:
            val = (XTPOINTER)(*(long*)toVal.addr);
	    break;
	}
    }

    /*
     * Well everything is done and the conversion was a success so lets
     * set the success flag to True.
     */
    *success = convResult;

    /*
     * Finally lets return the converted value.
     */
    /*SUPPRESS 80*/
    return(val);
}
#endif

/*
 * Function:
 *      MENU_POST(p, mw, ev, dispatch);
 * Description:
 *      A converter wrapper for convenience from BuilderXcessory.
 * Input:
 *      p - Widget : the widget to post
 *	mw - XtPointer : the menu widget
 *	ev - XEvent* : the event that caused the menu post
 *	dispatch - Boolean* : not used
 * Output:
 *      None
 */

#ifndef IGNORE_MENU_POST

void MENU_POST
    ARGLIST((p, mw, ev, dispatch))
        UARG(Widget, p)
        ARG(XtPointer, mw)
        ARG(XEvent *, ev)
        GRAU(Boolean *, dispatch)
{
    Arg	args[2];
    int	argcnt;
    int	button;
    Widget m = (Widget)mw;
    XButtonEvent *e = (XButtonEvent *)ev;

    argcnt = 0;
    XtSetArg(args[argcnt], XmNwhichButton, &button);
    argcnt++;
    XtGetValues(m, args, argcnt);
    if(e->button != button) return;
    XmMenuPosition(m, e);
    XtManageChild(m);
}
#endif

/*
 * Function:
 *      SET_BACKGROUND_COLOR(w, args, argcnt, bg_color);
 * Description:
 *      Sets the background color and shadows of a widget.
 * Input:
 *      w - The widget to set the background color on.
 *      args, argcnt - The argument list so far.
 *      bg_color - The new background color as a pixel.
 * Output:
 *      none
 *
 *  NOTES:  This assumes that args later in the argument list
 *          override those already in the list.  Therfore i f
 *          there are shadow colors later in the list they will win.
 *        
 *          There is no need to use this function when creating a widget
 *          only when doing a set values, shadow colors are automatically
 *          calculated at creation time.
 */

void SET_BACKGROUND_COLOR
    ARGLIST((w, args, argcnt, bg_color))
        ARG(Widget, w)
        ARG(ArgList, args)
        ARG(Cardinal *, argcnt)
        GRA(Pixel, bg_color)
{
    int		i;
    int		topShadowLoc;
    int		bottomShadowLoc;
    int		selectLoc;
    int		fgLoc;
    Boolean	argok = False;

#if ((XmVERSION == 1) && (XmREVISION > 0))

    /*
     * Walk through the arglist to see if the user set the top or
     * bottom shadow colors.
     */
    selectLoc = topShadowLoc =  bottomShadowLoc = UNSET;
    for (i = 0; i < *argcnt; i++)
    {
	if ((strcmp(args[i].name, XmNtopShadowColor) == 0) ||
	    (strcmp(args[i].name, XmNtopShadowPixmap) == 0))
	{
	    topShadowLoc = i;
	}
	else if ((strcmp(args[i].name, XmNbottomShadowColor) == 0) ||
		 (strcmp(args[i].name, XmNbottomShadowPixmap) == 0))
	{
	    bottomShadowLoc = i;
	}
	else if (strcmp(args[i].name, XmNarmColor) == 0)
	{
	    selectLoc = i;
	}
	else if (strcmp(args[i].name, XmNforeground) == 0)
	{
	    fgLoc = i;
	}
    }

    /*
     * If either the top or bottom shadow are not set then we
     * need to use XmGetColors to get the shadow colors from the backgound
     * color and add those that are not already in the arglist to the
     * arglist.
     * 
     */
    if ((bottomShadowLoc == UNSET) ||
	(topShadowLoc == UNSET) ||
	(selectLoc == UNSET) ||
	(fgLoc == UNSET))
    {
	Arg		larg[1];
	Colormap	cmap;
	Pixel		topShadow;
	Pixel		bottomShadow;
	Pixel		select;
	Pixel		fgColor;

	XtSetArg(larg[0], XmNcolormap, &cmap);
	XtGetValues(w, larg, 1);
	XmGetColors(XtScreen(w), cmap, bg_color, 
		    &fgColor, &topShadow, &bottomShadow, &select);

	if (topShadowLoc == UNSET)
	{
	    XtSetArg(args[*argcnt], XmNtopShadowColor, topShadow); 
	    (*argcnt)++;
	}
	
	if (bottomShadowLoc == UNSET)
	{
	    XtSetArg(args[*argcnt], XmNbottomShadowColor, bottomShadow); 
	    (*argcnt)++;
	}

	if (selectLoc == UNSET)
	{
	    XtSetArg(args[*argcnt], XmNarmColor, select); 
	    (*argcnt)++;
	}

	if (fgLoc == UNSET)
	{
	    XtSetArg(args[*argcnt], XmNforeground, fgColor); 
	    (*argcnt)++;
	}
    }
#endif

    XtSetArg(args[*argcnt], XmNbackground, bg_color); (*argcnt)++;
}

/*
 * Function:
 *	w = BxFindTopShell(start);
 * Description:
 *	Go up the hierarhcy until we find a shell widget.
 * Input:
 *      start - Widget : the widget to start with.
 * Output:
 *	w - Widget : the shell widget.
 */
#ifndef _BX_FIND_TOP_SHELL
#define _BX_FIND_TOP_SHELL

Widget BxFindTopShell
    ARGLIST((start))
        GRA(Widget, start)
{
    Widget	p;
    
    while ((p = XtParent(start)))
    {
	start = p;
    }
    return(start);
}
#endif /* _BX_FIND_TOP_SHELL */

/*
 * Function:
 *	BxWidgetIdsFromNames(ref, cbName, stringList)
 * Description:
 *	Return an array of widget ids from a list of widget names.
 * Input:
 *	ref - Widget : reference widget.
 *	cbName - char* : callback name.
 *	stringList - char*: list of widget names.
 * Output:
 *	WidgetList : array of widget IDs.
 */

#ifndef _BX_WIDGETIDS_FROM_NAMES
#define _BX_WIDGETIDS_FROM_NAMES

WidgetList BxWidgetIdsFromNames
    ARGLIST((ref, cbName, stringList))
        ARG(Widget, ref)
        ARG(char, *cbName)
        GRA(char, *stringList)
{
    WidgetList	wgtIds = NULL;
    int		wgtCount = 0;
    Widget	inst;
    Widget	current;
    String	tmp;
    String	start;
    String	widget;
    char       *ptr;
    
    /*
     * For backward compatibility, remove [ and ] from the list.
     */
    tmp = start = XtNewString(stringList);
    if((start = strchr(start, '[')) != NULL) start++;
    else start = tmp;
    
    while((start && *start) && isspace(*start))
    {
	start++;
    }
    ptr = strrchr(start, ']');
    if (ptr)
    {
	*ptr = '\0';
    }
    
    ptr = start + strlen(start) - 1;
    while(ptr && *ptr)
    {
	if (isspace(*ptr))
	{
	    ptr--;
	}
	else
	{
	    ptr++;
	    break;
	}
    }
    if (ptr && *ptr)
    {
	*ptr = '\0';
    }
    
    /*
     * start now points to the first character after the [.
     * the list is now either empty, one, or more widget
     * instance names.
     */
    start = strtok(start, ",");
    while(start)
    {
        while((start && *start) && isspace(*start))
        {
            start++;
        }
        ptr = start + strlen(start) - 1;
        while(ptr && *ptr)
        {
            if (isspace(*ptr))
            {
                ptr--;
            }
            else
            {
                ptr++;
                break;
            }
        }
        if (ptr && *ptr)
        {
            *ptr = '\0';
        }

	/*
	 * Form a string to use with XtNameToWidget().
	 */
        widget = (char *)XtMalloc((strlen(start) + 2) * sizeof(char));
        sprintf(widget, "*%s", start);
	
	/*
	 * Start at this level and continue up until the widget is found 
	 * or until the top of the hierarchy is reached.
	 */
	current = ref;
	while (current != NULL)
	{
	    inst = XtNameToWidget(current, widget);
	    if (inst != NULL )
	    {
		wgtCount++;
		wgtIds = (WidgetList)XtRealloc((char *)wgtIds, 
					       wgtCount * sizeof(Widget));
		wgtIds[wgtCount - 1] = inst;
		break;
	    }
	    current = XtParent(current);
	}

	if (current == NULL)
        {
            printf("Callback Error (%s):\n\t\
Cannot find widget %s\n", cbName, widget);
        }
        XtFree(widget);
        start = strtok(NULL, ",");
    }

    /*
     * NULL terminate the list.
     */
    wgtIds = (WidgetList)XtRealloc((char *)wgtIds, 
				   (wgtCount + 1) * sizeof(Widget));
    wgtIds[wgtCount] = NULL;

    XtFree((char *)tmp);
    return(wgtIds);
}
#endif /* _BX_WIDGETIDS_FROM_NAMES */
/****************************************************************************
 *
 * Big chunk of code inserted from Bull
 *
 ****************************************************************************/

#ifndef IGNORE_XPM_PIXMAP
#ifdef SYSV
#include <memory.h>
#endif

/*
 * Copyright 1990, 1991 GROUPE BULL
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of GROUPE BULL not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  GROUPE BULL makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * GROUPE BULL disclaims all warranties with regard to this software,
 * including all implied warranties of merchantability and fitness,
 * in no event shall GROUPE BULL be liable for any special,
 * indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious
 * action, arising out of or in connection with the use 
 * or performance of this software.
 *
 */

/* Return ErrorStatus codes:
 * null     if full success
 * positive if partial success
 * negative if failure
 */

#define XpmColorError    1
#define XpmSuccess       0
#define XpmOpenFailed   -1
#define XpmFileInvalid  -2
#define XpmNoMemory     -3
#define XpmColorFailed  -4

typedef struct {
    char *name;                         /* Symbolic color name */
    char *value;                        /* Color value */
    Pixel pixel;                        /* Color pixel */
} 	XpmColorSymbol;

typedef struct {
    unsigned long valuemask;            /* Specifies which attributes are
                                         * defined */

    Visual *visual;                     /* Specifies the visual to use */
    Colormap colormap;                  /* Specifies the colormap to use */
    unsigned int depth;                 /* Specifies the depth */
    unsigned int width;                 /* Returns the width of the created
                                         * pixmap */
    unsigned int height;                /* Returns the height of the created
                                         * pixmap */
    unsigned int x_hotspot;             /* Returns the x hotspot's
                                         * coordinate */
    unsigned int y_hotspot;             /* Returns the y hotspot's
                                         * coordinate */
    unsigned int cpp;                   /* Specifies the number of char per
                                         * pixel */
    Pixel *pixels;                      /* List of used color pixels */
    unsigned int npixels;               /* Number of pixels */
    XpmColorSymbol *colorsymbols;       /* Array of color symbols to
                                         * override */
    unsigned int numsymbols;            /* Number of symbols */
    char *rgb_fname;                    /* RGB text file name */

    /* Infos */
    unsigned int ncolors;               /* Number of colors */
    char ***colorTable;                 /* Color table pointer */
    char *hints_cmt;                    /* Comment of the hints section */
    char *colors_cmt;                   /* Comment of the colors section */
    char *pixels_cmt;                   /* Comment of the pixels section */
    unsigned int mask_pixel;            /* Transparent pixel's color table
                                         * index */
}      XpmAttributes;

/* Xpm attribute value masks bits */
#define XpmVisual          (1L<<0)
#define XpmColormap        (1L<<1)
#define XpmDepth           (1L<<2)
#define XpmSize            (1L<<3)      /* width & height */
#define XpmHotspot         (1L<<4)      /* x_hotspot & y_hotspot */
#define XpmCharsPerPixel   (1L<<5)
#define XpmColorSymbols    (1L<<6)
#define XpmRgbFilename     (1L<<7)
#define XpmInfos           (1L<<8)      /* all infos members */

#define XpmReturnPixels    (1L<<9)
#define XpmReturnInfos     XpmInfos

/*
 * minimal portability layer between ansi and KR C
 */

/* forward declaration of functions with prototypes */

#ifdef NeedFunctionPrototypes
#define FUNC(f, t, p) extern t f p
#define LFUNC(f, t, p) static t f p
#else
#define FUNC(f, t, p) extern t f()
#define LFUNC(f, t, p) static t f()
#endif

/*
 * functions declarations
 */

#ifdef __cplusplus
extern "C" {
#endif

    LFUNC(XpmCreatePixmapFromData, int, (Display * display,
					 Drawable d,
					 char **data,
					 Pixmap * pixmap_return,
					 Pixmap * shapemask_return,
					 XpmAttributes * attributes));

    LFUNC(XpmCreateImageFromData, int, (Display * display,
                                       char **data,
                                       XImage ** image_return,
                                       XImage ** shapemask_return,
                                       XpmAttributes * attributes));

    LFUNC(XpmFreeAttributes, void, (XpmAttributes * attributes));

#ifdef __cplusplus
}                                       /* for C++ V2.0 */
#endif


#ifdef SYSV
#define bcopy(source, dest, count) memcpy(dest, source, count)
#endif

typedef struct {
    unsigned int type;
    union {
        FILE *file;
        char **data;
    }     stream;
    char *cptr;
    unsigned int line;
    int CommentLength;
    char Comment[BUFSIZ];
    char *Bcmt, *Ecmt, Bos, Eos;
    unsigned int InsideString;          /* used during parsing: 0 or 1
                                         * whether we are inside or not */
}      xpmData;

#define XPMARRAY 0
#define XPMFILE  1
#define XPMPIPE  2

typedef unsigned char byte;

extern char *xpmColorKeys[];

#define TRANSPARENT_COLOR "None"        /* this must be a string! */

/* number of xpmColorKeys */
#define NKEYS 5

/*
 * key numbers for visual type, they must fit along with the number key of
 * each corresponding element in xpmColorKeys[] defined in xpm.h
 */
#define MONO    2
#define GRAY4   3
#define GRAY    4
#define COLOR   5

/* structure containing data related to an Xpm pixmap */
typedef struct {
    char *name;
    unsigned int width;
    unsigned int height;
    unsigned int cpp;
    unsigned int ncolors;
    char ***colorTable;
    unsigned int *pixelindex;
    XColor *xcolors;
    char **colorStrings;
    unsigned int mask_pixel;            /* mask pixel's colorTable index */
}      xpmInternAttrib;

#define UNDEF_PIXEL 0x80000000

char *xpmColorKeys[] =
{
 "s",					/* key #1: symbol */
 "m",					/* key #2: mono visual */
 "g4",					/* key #3: 4 grays visual */
 "g",					/* key #4: gray visual */
 "c",					/* key #5: color visual */
};

/* XPM private routines */

LFUNC(xpmCreateImage, int, (Display * display,
                           xpmInternAttrib * attrib,
                           XImage ** image_return,
                           XImage ** shapeimage_return,
                           XpmAttributes * attributes));

LFUNC(xpmParseData, int, (xpmData * data,
                         xpmInternAttrib * attrib_return,
                         XpmAttributes * attributes));

LFUNC(xpmVisualType, int, (Visual * visual));
LFUNC(xpmFreeColorTable, void, (char ***colorTable, int ncolors));

LFUNC(xpmInitInternAttrib, void, (xpmInternAttrib * xmpdata));

LFUNC(xpmFreeInternAttrib, void, (xpmInternAttrib * xmpdata));

LFUNC(xpmSetAttributes, void, (xpmInternAttrib * attrib,
                             XpmAttributes * attributes));

/* I/O utility */

LFUNC(xpmNextString, void, (xpmData * mdata));
LFUNC(xpmNextUI, int, (xpmData * mdata, unsigned int *ui_return));
LFUNC(xpmGetC, int, (xpmData * mdata));
LFUNC(xpmUngetC, int, (int c, xpmData * mdata));
LFUNC(xpmNextWord, unsigned int, (xpmData * mdata, char *buf));
LFUNC(xpmGetCmt, void, (xpmData * mdata, char **cmt));
LFUNC(xpmOpenArray, int, (char **data, xpmData * mdata));
LFUNC(XpmDataClose, void, (xpmData * mdata));

/* RGB utility */

LFUNC(xpm_xynormalizeimagebits, void, (register unsigned char *bp,
                                     register XImage * img));
LFUNC(xpm_znormalizeimagebits, void, (register unsigned char *bp,
                                    register XImage * img));

/* Image utility */

LFUNC(SetColor, int, (Display * display, Colormap colormap, char *colorname,
		      unsigned int color_index, Pixel * image_pixel,
		      Pixel * mask_pixel, unsigned int *mask_pixel_index));

LFUNC(CreateXImage, int, (Display * display, Visual * visual,
			  unsigned int depth, unsigned int width,
			  unsigned int height, XImage ** image_return));

LFUNC(SetImagePixels, void, (XImage * image, unsigned int width,
			    unsigned int height, unsigned int *pixelindex,
			    Pixel * pixels));

LFUNC(SetImagePixels32, void, (XImage * image, unsigned int width,
			      unsigned int height, unsigned int *pixelindex,
			      Pixel * pixels));

LFUNC(SetImagePixels16, void, (XImage * image, unsigned int width,
			      unsigned int height, unsigned int *pixelindex,
			      Pixel * pixels));

LFUNC(SetImagePixels8, void, (XImage * image, unsigned int width,
			     unsigned int height, unsigned int *pixelindex,
			     Pixel * pixels));

LFUNC(SetImagePixels1, void, (XImage * image, unsigned int width,
			     unsigned int height, unsigned int *pixelindex,
			     Pixel * pixels));

LFUNC(atoui, unsigned int, (char *p, unsigned int l, unsigned int *ui_return));

/*
 * Macros
 *
 * The XYNORMALIZE macro determines whether XY format data requires
 * normalization and calls a routine to do so if needed. The logic in
 * this module is designed for LSBFirst byte and bit order, so
 * normalization is done as required to present the data in this order.
 *
 * The ZNORMALIZE macro performs byte and nibble order normalization if
 * required for Z format data.
 *
 * The XYINDEX macro computes the index to the starting byte (char) boundary
 * for a bitmap_unit containing a pixel with coordinates x and y for image
 * data in XY format.
 *
 * The ZINDEX* macros compute the index to the starting byte (char) boundary
 * for a pixel with coordinates x and y for image data in ZPixmap format.
 *
 */

#define XYNORMALIZE(bp, img) \
    if ((img->byte_order == MSBFirst) || (img->bitmap_bit_order == MSBFirst)) \
        xpm_xynormalizeimagebits((unsigned char *)(bp), img)

#define ZNORMALIZE(bp, img) \
    if (img->byte_order == MSBFirst) \
        xpm_znormalizeimagebits((unsigned char *)(bp), img)

#define XYINDEX(x, y, img) \
    ((y) * img->bytes_per_line) + \
    (((x) + img->xoffset) / img->bitmap_unit) * (img->bitmap_unit >> 3)

#define ZINDEX(x, y, img) ((y) * img->bytes_per_line) + \
    (((x) * img->bits_per_pixel) >> 3)

#define ZINDEX32(x, y, img) ((y) * img->bytes_per_line) + ((x) << 2)

#define ZINDEX16(x, y, img) ((y) * img->bytes_per_line) + ((x) << 1)

#define ZINDEX8(x, y, img) ((y) * img->bytes_per_line) + (x)

#define ZINDEX1(x, y, img) ((y) * img->bytes_per_line) + ((x) >> 3)

#if __STDC__
#define Const const
#else
#define Const
#endif

Pixmap XPM_PIXMAP
(Widget w,char ** pixmapName)
{
    XpmAttributes	attributes;
    int			argcnt;
    Arg			args[10];
    Pixmap		pixmap;
    Pixmap		shape;
    int			returnValue;
    
    argcnt = 0;
    XtSetArg(args[argcnt], XmNdepth, &(attributes.depth)); argcnt++;
    XtSetArg(args[argcnt], XmNcolormap, &(attributes.colormap)); argcnt++;
    XtGetValues(w, args, argcnt);

    attributes.visual = DefaultVisual(XtDisplay(w),
				      DefaultScreen(XtDisplay(w)));
    attributes.valuemask = (XpmDepth | XpmColormap | XpmVisual);
    
    returnValue = XpmCreatePixmapFromData(XtDisplay(w),
					  DefaultRootWindow(XtDisplay(w)),
					  pixmapName, &pixmap, &shape,
					  &attributes);
    if ( shape )
    {
	XFreePixmap(XtDisplay(w), shape);
    }	

    switch(returnValue)
    {
    case XpmOpenFailed:
    case XpmFileInvalid:
    case XpmNoMemory:
    case XpmColorFailed:
	XtWarning("Could not create pixmap.");
	return(XmUNSPECIFIED_PIXMAP);
    default:
	return(pixmap);
    }
}

static unsigned int atoui
ARGLIST((p, l, ui_return))
ARG(register char *, p)
ARG(unsigned int, l)
GRA(unsigned int *, ui_return)
{
    register int n, i;

    n = 0;
    for (i = 0; i < l; i++)
        if (*p >= '0' && *p <= '9')
            n = n * 10 + *p++ - '0';
        else
            break;

    if (i != 0 && i == l) {
        *ui_return = n;
        return 1;
    } else
        return 0;
}

static int XpmCreatePixmapFromData
ARGLIST((display, d, data, pixmap_return, shapemask_return, attributes))
ARG(Display *, display)
ARG(Drawable, d)
ARG(char **, data)
ARG(Pixmap *, pixmap_return)
ARG(Pixmap *, shapemask_return)
GRA(XpmAttributes *,attributes)
{
    XImage *image, **imageptr = NULL;
    XImage *shapeimage, **shapeimageptr = NULL;
    int ErrorStatus;
    XGCValues gcv;
    GC gc;

    /*
     * initialize return values
     */
    if (pixmap_return) {
        *pixmap_return = NULL;
        imageptr = &image;
    }
    if (shapemask_return) {
        *shapemask_return = NULL;
        shapeimageptr = &shapeimage;
    }

    /*
     * create the images
     */
    ErrorStatus = XpmCreateImageFromData(display, data, imageptr,
                                         shapeimageptr, attributes);
    if (ErrorStatus < 0)
        return (ErrorStatus);

    /*
     * create the pixmaps
     */
    if (imageptr && image) {
        *pixmap_return = XCreatePixmap(display, d, image->width,
                                       image->height, image->depth);
        gcv.function = GXcopy;
        gc = XCreateGC(display, *pixmap_return, GCFunction, &gcv);

        XPutImage(display, *pixmap_return, gc, image, 0, 0, 0, 0,
                  image->width, image->height);

        XDestroyImage(image);
        XFreeGC(display, gc);
    }
    if (shapeimageptr && shapeimage) {
        *shapemask_return = XCreatePixmap(display, d, shapeimage->width,
                                          shapeimage->height,
                                          shapeimage->depth);
        gcv.function = GXcopy;
        gc = XCreateGC(display, *shapemask_return, GCFunction, &gcv);

        XPutImage(display, *shapemask_return, gc, shapeimage, 0, 0, 0, 0,
                  shapeimage->width, shapeimage->height);

        XDestroyImage(shapeimage);
        XFreeGC(display, gc);
    }
    return (ErrorStatus);
}


static int XpmCreateImageFromData
ARGLIST((display, data, image_return, shapeimage_return, attributes))
ARG(Display *,display)
ARG(char **, data)
ARG(XImage **, image_return)
ARG(XImage **, shapeimage_return)
GRA(XpmAttributes *, attributes)
{
    xpmData mdata;
    int ErrorStatus;
    xpmInternAttrib attrib;

    /*
     * initialize return values
     */
    if (image_return)
        *image_return = NULL;
    if (shapeimage_return)
        *shapeimage_return = NULL;

    if ((ErrorStatus = xpmOpenArray(data, &mdata)) != XpmSuccess)
        return (ErrorStatus);

    xpmInitInternAttrib(&attrib);

    ErrorStatus = xpmParseData(&mdata, &attrib, attributes);

    if (ErrorStatus == XpmSuccess)
        ErrorStatus = xpmCreateImage(display, &attrib, image_return,
                                     shapeimage_return, attributes);

    if (ErrorStatus >= 0)
        xpmSetAttributes(&attrib, attributes);
    else if (attributes)
        XpmFreeAttributes(attributes);

    xpmFreeInternAttrib(&attrib);
    XpmDataClose(&mdata);

    return (ErrorStatus);
}

/*
 * open the given array to be read or written as an xpmData which is returned
 */
static int xpmOpenArray
ARGLIST((data, mdata))
ARG(char **,data)
GRA(xpmData *,mdata)
{
    mdata->type = XPMARRAY;
    mdata->stream.data = data;
    mdata->cptr = *data;
    mdata->line = 0;
    mdata->CommentLength = 0;
    mdata->Bcmt = mdata->Ecmt = NULL;
    mdata->Bos = mdata->Eos = '\0';
    mdata->InsideString = 0;
    return (XpmSuccess);
}

/*
 * Intialize the xpmInternAttrib pointers to Null to know
 * which ones must be freed later on.
 */
static void xpmInitInternAttrib
ARGLIST((attrib))
GRA(xpmInternAttrib *,attrib)
{
    attrib->ncolors = 0;
    attrib->colorTable = NULL;
    attrib->pixelindex = NULL;
    attrib->xcolors = NULL;
    attrib->colorStrings = NULL;
    attrib->mask_pixel = UNDEF_PIXEL;
}

/* function call in case of error, frees only localy allocated variables */
#undef RETURN
#define RETURN(status) \
  { if (colorTable) xpmFreeColorTable(colorTable, ncolors); \
    if (chars) free(chars); \
    if (pixelindex) free((char *)pixelindex); \
    if (hints_cmt)  free((char *)hints_cmt); \
    if (colors_cmt) free((char *)colors_cmt); \
    if (pixels_cmt) free((char *)pixels_cmt); \
    return(status); }

/*
 * This function parses an Xpm file or data and store the found informations
 * in an an xpmInternAttrib structure which is returned.
 */
static int xpmParseData
ARGLIST((data, attrib_return, attributes))
ARG(xpmData *,data)
ARG(xpmInternAttrib *, attrib_return)
GRA(XpmAttributes *, attributes)
{
    /* variables to return */
    unsigned int width, height;
    unsigned int ncolors = 0;
    unsigned int cpp;
    unsigned int x_hotspot, y_hotspot, hotspot = 0;
    char ***colorTable = NULL;
    unsigned int *pixelindex = NULL;
    char *hints_cmt = NULL;
    char *colors_cmt = NULL;
    char *pixels_cmt = NULL;

    /* calculation variables */
    unsigned int rncolors = 0;		/* read number of colors, it is
					 * different to ncolors to avoid
					 * problem when freeing the
					 * colorTable in case an error
					 * occurs while reading the hints
					 * line */
    unsigned int key = 0;		/* color key */
    char *chars = NULL, buf[BUFSIZ];
    unsigned int *iptr;
    unsigned int a, b, x, y, l;

    unsigned int curkey;		/* current color key */
    unsigned int lastwaskey;		/* key read */
    char curbuf[BUFSIZ];		/* current buffer */

    /*
     * read hints: width, height, ncolors, chars_per_pixel 
     */
    if (!(xpmNextUI(data, &width) && xpmNextUI(data, &height)
	  && xpmNextUI(data, &rncolors) && xpmNextUI(data, &cpp)))
	RETURN(XpmFileInvalid);

    ncolors = rncolors;

    /*
     * read hotspot coordinates if any 
     */
    hotspot = xpmNextUI(data, &x_hotspot) && xpmNextUI(data, &y_hotspot);

    /*
     * store the hints comment line 
     */
    if (attributes && (attributes->valuemask & XpmReturnInfos))
	xpmGetCmt(data, &hints_cmt);

    /*
     * read colors 
     */
    colorTable = (char ***) calloc(ncolors, sizeof(char **));
    if (!colorTable)
	RETURN(XpmNoMemory);

    for (a = 0; a < ncolors; a++) {
	xpmNextString(data);		/* skip the line */
	colorTable[a] = (char **) calloc((NKEYS + 1), sizeof(char *));
	if (!colorTable[a])
	    RETURN(XpmNoMemory);

	/*
	 * read pixel value 
	 */
	colorTable[a][0] = (char *) malloc(cpp);
	if (!colorTable[a][0])
	    RETURN(XpmNoMemory);
	for (b = 0; b < cpp; b++)
	    colorTable[a][0][b] = xpmGetC(data);

	/*
	 * read color keys and values 
	 */
	curkey = 0;
	lastwaskey = 0;
	while ((l = xpmNextWord(data, buf))) {
	    if (!lastwaskey) {
		for (key = 1; key < NKEYS + 1; key++)
		    if ((strlen(xpmColorKeys[key - 1]) == l)
			&& (!strncmp(xpmColorKeys[key - 1], buf, l)))
			break;
	    }
	    if (!lastwaskey && key <= NKEYS) {	/* open new key */
		if (curkey) {		/* flush string */
		    colorTable[a][curkey] =
			(char *) malloc(strlen(curbuf) + 1);
		    if (!colorTable[a][curkey])
			RETURN(XpmNoMemory);
		    strcpy(colorTable[a][curkey], curbuf);
		}
		curkey = key;		/* set new key  */
		curbuf[0] = '\0';	/* reset curbuf */
		lastwaskey = 1;
	    } else {
		if (!curkey)
		    RETURN(XpmFileInvalid);	/* key without value */
		if (!lastwaskey)
		    strcat(curbuf, " ");/* append space */
		buf[l] = '\0';
		strcat(curbuf, buf);	/* append buf */
		lastwaskey = 0;
	    }
	}
	if (!curkey)
	    RETURN(XpmFileInvalid);	/* key without value */
	colorTable[a][curkey] = (char *) malloc(strlen(curbuf) + 1);
	if (!colorTable[a][curkey])
	    RETURN(XpmNoMemory);
	strcpy(colorTable[a][curkey], curbuf);
    }

    /*
     * store the colors comment line 
     */
    if (attributes && (attributes->valuemask & XpmReturnInfos))
	xpmGetCmt(data, &colors_cmt);

    /*
     * read pixels and index them on color number 
     */
    pixelindex =
	(unsigned int *) malloc(sizeof(unsigned int) * width * height);
    if (!pixelindex)
	RETURN(XpmNoMemory);

    iptr = pixelindex;

    chars = (char *) malloc(cpp);
    if (!chars)
	RETURN(XpmNoMemory);

    for (y = 0; y < height; y++) {
	xpmNextString(data);
	for (x = 0; x < width; x++, iptr++) {
	    for (a = 0; a < cpp; a++)
		chars[a] = xpmGetC(data);
	    for (a = 0; a < ncolors; a++)
		if (!strncmp(colorTable[a][0], chars, cpp))
		    break;
	    if (a == ncolors)
		RETURN(XpmFileInvalid);	/* no color matches */
	    *iptr = a;
	}
    }

    /*
     * store the pixels comment line 
     */
    if (attributes && (attributes->valuemask & XpmReturnInfos))
	xpmGetCmt(data, &pixels_cmt);

    free(chars);

    /*
     * store found informations in the xpmInternAttrib structure 
     */
    attrib_return->width = width;
    attrib_return->height = height;
    attrib_return->cpp = cpp;
    attrib_return->ncolors = ncolors;
    attrib_return->colorTable = colorTable;
    attrib_return->pixelindex = pixelindex;

    if (attributes) {
	if (attributes->valuemask & XpmReturnInfos) {
	    attributes->hints_cmt = hints_cmt;
	    attributes->colors_cmt = colors_cmt;
	    attributes->pixels_cmt = pixels_cmt;
	}
	if (hotspot) {
	    attributes->x_hotspot = x_hotspot;
	    attributes->y_hotspot = y_hotspot;
	    attributes->valuemask |= XpmHotspot;
	}
    }
    return (XpmSuccess);
}

/*
 * set the color pixel related to the given colorname,
 * return 0 if success, 1 otherwise.
 */

static int SetColor
ARGLIST((display, colormap,colorname, color_index, image_pixel, mask_pixel, mask_pixel_index))
ARG(Display *, display)
ARG(Colormap, colormap)
ARG(char *, colorname)
ARG(unsigned int, color_index)
ARG(Pixel *, image_pixel)
ARG(Pixel *, mask_pixel)
GRA(unsigned int *, mask_pixel_index)
{
    XColor xcolor;

    if (STRCASECMP(colorname, TRANSPARENT_COLOR)) {
	if (!XParseColor(display, colormap, colorname, &xcolor)
	    || (!XAllocColor(display, colormap, &xcolor)))
	    return (1);
	*image_pixel = xcolor.pixel;
	*mask_pixel = 1;
    } else {
	*image_pixel = 0;
	*mask_pixel = 0;
	*mask_pixel_index = color_index;/* store the color table index */
    }
    return (0);
}

/* function call in case of error, frees only localy allocated variables */
#undef RETURN
#define RETURN(status) \
  { if (image) XDestroyImage(image); \
    if (shapeimage) XDestroyImage(shapeimage); \
    if (image_pixels) free((char *)image_pixels); \
    if (mask_pixels) free((char *)mask_pixels); \
    return(status); }

static int xpmCreateImage
ARGLIST((display, attrib, image_return, shapeimage_return, attributes))
ARG(Display *, display)
ARG(xpmInternAttrib *, attrib)
ARG(XImage **, image_return)
ARG(XImage **, shapeimage_return)
GRA(XpmAttributes *, attributes)
{
    /* variables stored in the XpmAttributes structure */
    Visual *visual;
    Colormap colormap;
    unsigned int depth;
    XpmColorSymbol *colorsymbols = NULL;
    unsigned int numsymbols;

    /* variables to return */
    XImage *image = NULL;
    XImage *shapeimage = NULL;
    unsigned int mask_pixel;
    unsigned int ErrorStatus, ErrorStatus2;

    /* calculation variables */
    Pixel *image_pixels = NULL;
    Pixel *mask_pixels = NULL;
    char *colorname;
    unsigned int a, b, l;
    Boolean pixel_defined;
    unsigned int key;


    /*
     * retrieve information from the XpmAttributes 
     */
    if (attributes && attributes->valuemask & XpmColorSymbols) {
	colorsymbols = attributes->colorsymbols;
	numsymbols = attributes->numsymbols;
    } else
	numsymbols = 0;

    if (attributes && attributes->valuemask & XpmVisual)
	visual = attributes->visual;
    else
	visual = DefaultVisual(display, DefaultScreen(display));

    if (attributes && attributes->valuemask & XpmColormap)
	colormap = attributes->colormap;
    else
	colormap = DefaultColormap(display, DefaultScreen(display));

    if (attributes && attributes->valuemask & XpmDepth)
	depth = attributes->depth;
    else
	depth = DefaultDepth(display, DefaultScreen(display));


    ErrorStatus = XpmSuccess;

    /*
     * alloc pixels index tables 
     */

    key = xpmVisualType(visual);
    image_pixels = (Pixel *) malloc(sizeof(Pixel) * attrib->ncolors);
    if (!image_pixels)
	RETURN(XpmNoMemory);

    mask_pixels = (Pixel *) malloc(sizeof(Pixel) * attrib->ncolors);
    if (!mask_pixels)
	RETURN(XpmNoMemory);

    mask_pixel = UNDEF_PIXEL;

    /*
     * get pixel colors, store them in index tables 
     */
    for (a = 0; a < attrib->ncolors; a++) {
	colorname = NULL;
	pixel_defined = False;

	/*
	 * look for a defined symbol 
	 */
	if (numsymbols && attrib->colorTable[a][1]) {
	    for (l = 0; l < numsymbols; l++)
		if (!strcmp(colorsymbols[l].name, attrib->colorTable[a][1]))
		    break;
	    if (l != numsymbols) {
		if (colorsymbols[l].value)
		    colorname = colorsymbols[l].value;
		else
		    pixel_defined = True;
	    }
	}
	if (!pixel_defined) {		/* pixel not given as symbol value */

	    if (colorname) {		/* colorname given as symbol value */
		if (!SetColor(display, colormap, colorname, a,
			   &image_pixels[a], &mask_pixels[a], &mask_pixel))
		    pixel_defined = True;
		else
		    ErrorStatus = XpmColorError;
	    }
	    b = key;
	    while (!pixel_defined && b > 1) {
		if (attrib->colorTable[a][b]) {
		    if (!SetColor(display, colormap, attrib->colorTable[a][b],
				  a, &image_pixels[a], &mask_pixels[a],
				  &mask_pixel)) {
			pixel_defined = True;
			break;
		    } else
			ErrorStatus = XpmColorError;
		}
		b--;
	    }

	    b = key + 1;
	    while (!pixel_defined && b < NKEYS + 1) {
		if (attrib->colorTable[a][b]) {
		    if (!SetColor(display, colormap, attrib->colorTable[a][b],
				  a, &image_pixels[a], &mask_pixels[a],
				  &mask_pixel)) {
			pixel_defined = True;
			break;
		    } else
			ErrorStatus = XpmColorError;
		}
		b++;
	    }

	    if (!pixel_defined)
		RETURN(XpmColorFailed);

	} else {
	    image_pixels[a] = colorsymbols[l].pixel;
	    mask_pixels[a] = 1;
	}
    }

    /*
     * create the image 
     */
    if (image_return) {
	ErrorStatus2 = CreateXImage(display, visual, depth,
				    attrib->width, attrib->height, &image);
	if (ErrorStatus2 != XpmSuccess)
	    RETURN(ErrorStatus2);

	/*
	 * set the image data 
	 *
	 * In case depth is 1 or bits_per_pixel is 4, 6, 8, 24 or 32 use
	 * optimized functions, otherwise use slower but sure general one. 
	 *
	 */

	if (image->depth == 1)
	    SetImagePixels1(image, attrib->width, attrib->height,
			    attrib->pixelindex, image_pixels);
	else if (image->bits_per_pixel == 8)
	    SetImagePixels8(image, attrib->width, attrib->height,
			    attrib->pixelindex, image_pixels);
	else if (image->bits_per_pixel == 16)
	    SetImagePixels16(image, attrib->width, attrib->height,
			     attrib->pixelindex, image_pixels);
	else if (image->bits_per_pixel == 32)
	    SetImagePixels32(image, attrib->width, attrib->height,
			     attrib->pixelindex, image_pixels);
	else
	    SetImagePixels(image, attrib->width, attrib->height,
			   attrib->pixelindex, image_pixels);
    }

    /*
     * create the shape mask image 
     */
    if (mask_pixel != UNDEF_PIXEL && shapeimage_return) {
	ErrorStatus2 = CreateXImage(display, visual, 1, attrib->width,
				    attrib->height, &shapeimage);
	if (ErrorStatus2 != XpmSuccess)
	    RETURN(ErrorStatus2);

	SetImagePixels1(shapeimage, attrib->width, attrib->height,
			attrib->pixelindex, mask_pixels);
    }
    free((char *)mask_pixels);

    /*
     * if requested store allocated pixels in the XpmAttributes structure 
     */
    if (attributes &&
	(attributes->valuemask & XpmReturnInfos
	 || attributes->valuemask & XpmReturnPixels)) {
	if (mask_pixel != UNDEF_PIXEL) {
	    Pixel *pixels, *p1, *p2;

	    attributes->npixels = attrib->ncolors - 1;
	    pixels = (Pixel *) malloc(sizeof(Pixel) * attributes->npixels);
	    if (pixels) {
		p1 = image_pixels;
		p2 = pixels;
		for (a = 0; a < attrib->ncolors; a++, p1++)
		    if (a != mask_pixel)
			*p2++ = *p1;
		attributes->pixels = pixels;
	    } else {
		/* if error just say we can't return requested data */
		attributes->valuemask &= ~XpmReturnPixels;
		attributes->valuemask &= ~XpmReturnInfos;
		attributes->pixels = NULL;
		attributes->npixels = 0;
	    }
	    free((char *)image_pixels);
	} else {
	    attributes->pixels = image_pixels;
	    attributes->npixels = attrib->ncolors;
	}
	attributes->mask_pixel = mask_pixel;
    } else
	free((char *)image_pixels);


    /*
     * return created images 
     */
    if (image_return)
	*image_return = image;

    if (shapeimage_return)
	*shapeimage_return = shapeimage;

    return (ErrorStatus);
}


/*
 * Create an XImage
 */
static int CreateXImage
ARGLIST((display, visual, depth, width, height, image_return))
ARG(Display *, display)
ARG(Visual *, visual)
ARG(unsigned int, depth)
ARG(unsigned int, width)
ARG(unsigned int, height)
GRA(XImage **, image_return)
{
    int bitmap_pad;

    /* first get bitmap_pad */
    if (depth > 16)
	bitmap_pad = 32;
    else if (depth > 8)
	bitmap_pad = 16;
    else
	bitmap_pad = 8;

    /* then create the XImage with data = NULL and bytes_per_line = 0 */

    *image_return = XCreateImage(display, visual, depth, ZPixmap, 0, 0,
				 width, height, bitmap_pad, 0);
    if (!*image_return)
	return (XpmNoMemory);

    /* now that bytes_per_line must have been set properly alloc data */

    (*image_return)->data =
	(char *) malloc((*image_return)->bytes_per_line * height);

    if (!(*image_return)->data) {
	XDestroyImage(*image_return);
	*image_return = NULL;
	return (XpmNoMemory);
    }
    return (XpmSuccess);
}


/*
 * The functions below are written from X11R5 MIT's code (XImUtil.c)
 *
 * The idea is to have faster functions than the standard XPutPixel function
 * to build the image data. Indeed we can speed up things by supressing tests
 * performed for each pixel. We do exactly the same tests but at the image
 * level. Assuming that we use only ZPixmap images. 
 */

LFUNC(_putbits, void, (register char *src, int dstoffset,
		      register int numbits, register char *dst));

LFUNC(_XReverse_Bytes, void, (register unsigned char *bpt, register int nb));

static unsigned char Const _reverse_byte[0x100] = {
			    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
			    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
			    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
			    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
			    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
			    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
			    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
			    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
			    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
			    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
			    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
			    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
			    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
			    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
			    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
			    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
			    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
			    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
			    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
			    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
			    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
			    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
			    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
			    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
			    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
			    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
			    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
			    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
			    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
			    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
			    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
			     0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

static void _XReverse_Bytes
ARGLIST((bpt, nb))
ARG(register unsigned char *, bpt)
GRA(register int, nb)
{
    do {
	*bpt = _reverse_byte[*bpt];
	bpt++;
    } while (--nb > 0);
}

static void xpm_xynormalizeimagebits
ARGLIST((bp,img))
ARG(register unsigned char *, bp)
GRA(register XImage *, img)
{
    register unsigned char c;

    if (img->byte_order != img->bitmap_bit_order) {
	switch (img->bitmap_unit) {

	case 16:
	    c = *bp;
	    *bp = *(bp + 1);
	    *(bp + 1) = c;
	    break;

	case 32:
	    c = *(bp + 3);
	    *(bp + 3) = *bp;
	    *bp = c;
	    c = *(bp + 2);
	    *(bp + 2) = *(bp + 1);
	    *(bp + 1) = c;
	    break;
	}
    }
    if (img->bitmap_bit_order == MSBFirst)
	_XReverse_Bytes(bp, img->bitmap_unit >> 3);
}

static void xpm_znormalizeimagebits
ARGLIST((bp,img))
ARG(register unsigned char *, bp)
GRA(register XImage *, img)
{
    register unsigned char c;

    switch (img->bits_per_pixel) {

    case 4:
	*bp = ((*bp >> 4) & 0xF) | ((*bp << 4) & ~0xF);
	break;

    case 16:
	c = *bp;
	*bp = *(bp + 1);
	*(bp + 1) = c;
	break;

    case 24:
	c = *(bp + 2);
	*(bp + 2) = *bp;
	*bp = c;
	break;

    case 32:
	c = *(bp + 3);
	*(bp + 3) = *bp;
	*bp = c;
	c = *(bp + 2);
	*(bp + 2) = *(bp + 1);
	*(bp + 1) = c;
	break;
    }
}

static unsigned char Const _lomask[0x09] = {
		     0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
static unsigned char Const _himask[0x09] = {
		     0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};

static void _putbits
ARGLIST((src, dstoffset, numbits, dst))
ARG(register char *, src)		/* address of source bit string */
ARG(int, dstoffset)			/* bit offset into destination;
					 * range is 0-31 */
ARG(register int, numbits)		/* number of bits to copy to
					 * destination */
GRA(register char *, dst)		/* address of destination bit string */
{
    register unsigned char chlo, chhi;
    int hibits;

    dst = dst + (dstoffset >> 3);
    dstoffset = dstoffset & 7;
    hibits = 8 - dstoffset;
    chlo = *dst & _lomask[dstoffset];
    for (;;) {
	chhi = (*src << dstoffset) & _himask[dstoffset];
	if (numbits <= hibits) {
	    chhi = chhi & _lomask[dstoffset + numbits];
	    *dst = (*dst & _himask[dstoffset + numbits]) | chlo | chhi;
	    break;
	}
	*dst = chhi | chlo;
	dst++;
	numbits = numbits - hibits;
	chlo = (unsigned char) (*src & _himask[hibits]) >> hibits;
	src++;
	if (numbits <= dstoffset) {
	    chlo = chlo & _lomask[numbits];
	    *dst = (*dst & _himask[numbits]) | chlo;
	    break;
	}
	numbits = numbits - dstoffset;
    }
}

/*
 * Default method to write pixels into a Z image data structure.
 * The algorithm used is:
 *
 *	copy the destination bitmap_unit or Zpixel to temp
 *	normalize temp if needed
 *	copy the pixel bits into the temp
 *	renormalize temp if needed
 *	copy the temp back into the destination image data
 */

static void SetImagePixels
ARGLIST((image, width, height, pixelindex, pixels))
ARG(XImage *, image)
ARG(unsigned int, width)
ARG(unsigned int, height)
ARG(unsigned int *, pixelindex)
GRA(Pixel *, pixels)
{
    Pixel pixel;
    unsigned long px;
    register char *src;
    register char *dst;
    int nbytes;
    register unsigned int *iptr;
    register int x, y, i;

    iptr = pixelindex;
    if (image->depth == 1) {
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		pixel = pixels[*iptr];
		for (i = 0, px = pixel;
		     i < sizeof(unsigned long); i++, px >>= 8)
		    ((unsigned char *) &pixel)[i] = (unsigned char)px;
		src = &image->data[XYINDEX(x, y, image)];
		dst = (char *) &px;
		px = 0;
		nbytes = image->bitmap_unit >> 3;
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
		XYNORMALIZE(&px, image);
		i = ((x + image->xoffset) % image->bitmap_unit);
		_putbits((char *) &pixel, i, 1, (char *) &px);
		XYNORMALIZE(&px, image);
		src = (char *) &px;
		dst = &image->data[XYINDEX(x, y, image)];
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
	    }
    } else {
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		pixel = pixels[*iptr];
		if (image->depth == 4)
		    pixel &= 0xf;
		for (i = 0, px = pixel;
		     i < sizeof(unsigned long); i++, px >>= 8)
		    ((unsigned char *) &pixel)[i] = (unsigned char)px;
		src = &image->data[ZINDEX(x, y, image)];
		dst = (char *) &px;
		px = 0;
		nbytes = (image->bits_per_pixel + 7) >> 3;
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
		ZNORMALIZE(&px, image);
		_putbits((char *) &pixel,
			 (x * image->bits_per_pixel) & 7,
			 image->bits_per_pixel, (char *) &px);
		ZNORMALIZE(&px, image);
		src = (char *) &px;
		dst = &image->data[ZINDEX(x, y, image)];
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
	    }
    }
}

/*
 * write pixels into a 32-bits Z image data structure
 */

#ifndef WORD64
static unsigned long byteorderpixel = MSBFirst << 24;

#endif

static void SetImagePixels32
ARGLIST((image, width, height, pixelindex, pixels))
ARG(XImage *, image)
ARG(unsigned int, width)
ARG(unsigned int, height)
ARG(unsigned int *, pixelindex)
GRA(Pixel *, pixels)
{
    register unsigned char *addr;
    register unsigned int *paddr;
    register unsigned int *iptr;
    register int x, y;

    iptr = pixelindex;
#ifndef WORD64
    if (*((char *) &byteorderpixel) == image->byte_order) {
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		paddr =
		    (unsigned int *)(&(image->data[ZINDEX32(x, y, image)]));
		*paddr = (unsigned int)pixels[*iptr];
	    }
    } else
#endif
    if (image->byte_order == MSBFirst)
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &((unsigned char *) image->data)[ZINDEX32(x, y, image)];
		addr[0] = (unsigned char)(pixels[*iptr] >> 24);
		addr[1] = (unsigned char)(pixels[*iptr] >> 16);
		addr[2] = (unsigned char)(pixels[*iptr] >> 8);
		addr[3] = (unsigned char)(pixels[*iptr]);
	    }
    else
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &((unsigned char *) image->data)[ZINDEX32(x, y, image)];
		addr[3] = (unsigned char)(pixels[*iptr] >> 24);
		addr[2] = (unsigned char)(pixels[*iptr] >> 16);
		addr[1] = (unsigned char)(pixels[*iptr] >> 8);
		addr[0] = (unsigned char)(pixels[*iptr]);
	    }
}

/*
 * write pixels into a 16-bits Z image data structure
 */

static void SetImagePixels16
ARGLIST((image, width, height, pixelindex, pixels))
ARG(XImage *, image)
ARG(unsigned int, width)
ARG(unsigned int, height)
ARG(unsigned int *, pixelindex)
GRA(Pixel *, pixels)
{
    register unsigned char *addr;
    register unsigned int *iptr;
    register int x, y;

    iptr = pixelindex;
    if (image->byte_order == MSBFirst)
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &((unsigned char *) image->data)[ZINDEX16(x, y, image)];
		addr[0] = (unsigned char)(pixels[*iptr] >> 8);
		addr[1] = (unsigned char)(pixels[*iptr]);
	    }
    else
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &((unsigned char *) image->data)[ZINDEX16(x, y, image)];
		addr[1] = (unsigned char)(pixels[*iptr] >> 8);
		addr[0] = (unsigned char)(pixels[*iptr]);
	    }
}

/*
 * write pixels into a 8-bits Z image data structure
 */

static void SetImagePixels8
ARGLIST((image, width, height, pixelindex, pixels))
ARG(XImage *, image)
ARG(unsigned int, width)
ARG(unsigned int, height)
ARG(unsigned int *, pixelindex)
GRA(Pixel *, pixels)

{
    register unsigned int *iptr;
    register int x, y;

    iptr = pixelindex;
    for (y = 0; y < height; y++)
	for (x = 0; x < width; x++, iptr++)
	    image->data[ZINDEX8(x, y, image)] = (char)pixels[*iptr];
}

/*
 * write pixels into a 1-bit depth image data structure and **offset null**
 */

static void SetImagePixels1
ARGLIST((image, width, height, pixelindex, pixels))
ARG(XImage *, image)
ARG(unsigned int, width)
ARG(unsigned int, height)
ARG(unsigned int *, pixelindex)
GRA(Pixel *, pixels)
{
    unsigned char bit;
    int xoff, yoff;
    register unsigned int *iptr;
    register int x, y;

    if (image->byte_order != image->bitmap_bit_order)
	SetImagePixels(image, width, height, pixelindex, pixels);
    else {
	iptr = pixelindex;
	if (image->bitmap_bit_order == MSBFirst)
	    for (y = 0; y < height; y++)
		for (x = 0; x < width; x++, iptr++) {
		    yoff = ZINDEX1(x, y, image);
		    xoff = x & 7;
		    bit = 0x80 >> xoff;
		    if (pixels[*iptr] & 1)
			image->data[yoff] |= bit;
		    else
			image->data[yoff] &= ~bit;
		}
	else
	    for (y = 0; y < height; y++)
		for (x = 0; x < width; x++, iptr++) {
		    yoff = ZINDEX1(x, y, image);
		    xoff = x & 7;
		    bit = 1 << xoff;
		    if (pixels[*iptr] & 1)
			image->data[yoff] |= bit;
		    else
			image->data[yoff] &= ~bit;
		}
    }
}

/*
 * Store into the XpmAttributes structure the required informations stored in
 * the xpmInternAttrib structure.
 */

static void xpmSetAttributes
ARGLIST((attrib, attributes))
ARG(xpmInternAttrib *, attrib)
GRA(XpmAttributes *, attributes)
{
    if (attributes) {
        if (attributes->valuemask & XpmReturnInfos) {
            attributes->cpp = attrib->cpp;
            attributes->ncolors = attrib->ncolors;
            attributes->colorTable = attrib->colorTable;

            attrib->ncolors = 0;
            attrib->colorTable = NULL;
        }
        attributes->width = attrib->width;
        attributes->height = attrib->height;
        attributes->valuemask |= XpmSize;
    }
}

/*
 * Free the XpmAttributes structure members
 * but the structure itself
 */

static void XpmFreeAttributes
ARGLIST((attributes))
GRA(XpmAttributes *, attributes)
{
    if (attributes) {
        if (attributes->valuemask & XpmReturnPixels && attributes->pixels) {
            free((char *)attributes->pixels);
            attributes->pixels = NULL;
            attributes->npixels = 0;
        }
        if (attributes->valuemask & XpmInfos) {
            if (attributes->colorTable) {
                xpmFreeColorTable(attributes->colorTable, attributes->ncolors);
                attributes->colorTable = NULL;
                attributes->ncolors = 0;
            }
            if (attributes->hints_cmt) {
                free(attributes->hints_cmt);
                attributes->hints_cmt = NULL;
            }
            if (attributes->colors_cmt) {
                free(attributes->colors_cmt);
                attributes->colors_cmt = NULL;
            }
            if (attributes->pixels_cmt) {
                free(attributes->pixels_cmt);
                attributes->pixels_cmt = NULL;
            }
            if (attributes->pixels) {
                free((char *)attributes->pixels);
                attributes->pixels = NULL;
            }
        }
        attributes->valuemask = 0;
    }
}

/*
 * Free the xpmInternAttrib pointers which have been allocated
 */

static void xpmFreeInternAttrib
ARGLIST((attrib))
GRA(xpmInternAttrib *, attrib)
{
    unsigned int a;

    if (attrib->colorTable)
        xpmFreeColorTable(attrib->colorTable, attrib->ncolors);
    if (attrib->pixelindex)
        free((char *)attrib->pixelindex);
    if (attrib->xcolors)
        free((char *)attrib->xcolors);
    if (attrib->colorStrings) {
        for (a = 0; a < attrib->ncolors; a++)
            if (attrib->colorStrings[a])
                free((char *)attrib->colorStrings[a]);
        free((char *)attrib->colorStrings);
    }
}

/*
 * close the file related to the xpmData if any
 */
static void XpmDataClose
ARGLIST((mdata))
GRA(xpmData *, mdata)
{
    switch (mdata->type) {
    case XPMARRAY:
        break;
    case XPMFILE:
        if (mdata->stream.file != (stdout) && mdata->stream.file != (stdin))
            fclose(mdata->stream.file);
        break;
#ifdef ZPIPE
    case XPMPIPE:
        pclose(mdata->stream.file);
#endif
    }
}

/*
 * skip whitespace and compute the following unsigned int,
 * returns 1 if one is found and 0 if not
 */
static int xpmNextUI
ARGLIST((mdata, ui_return))
ARG(xpmData *, mdata)
GRA(unsigned int *, ui_return)
{
    char buf[BUFSIZ];
    int l;

    l = xpmNextWord(mdata, buf);
    return atoui(buf, l, ui_return);
}

/*
 * get the current comment line
 */
static void xpmGetCmt
ARGLIST((mdata, cmt))
ARG(xpmData *, mdata)
GRA(char **, cmt)
{
    switch (mdata->type) {
    case XPMARRAY:
        *cmt = NULL;
        break;
    case XPMFILE:
    case XPMPIPE:
        if (mdata->CommentLength) {
            *cmt = (char *) malloc(mdata->CommentLength + 1);
            strncpy(*cmt, mdata->Comment, mdata->CommentLength);
            (*cmt)[mdata->CommentLength] = '\0';
            mdata->CommentLength = 0;
        } else
            *cmt = NULL;
        break;
    }
}

/*
 * skip to the end of the current string and the beginning of the next one
 */
static void xpmNextString
ARGLIST((mdata))
GRA(xpmData *, mdata)
{
    int c;

    switch (mdata->type) {
    case XPMARRAY:
        mdata->cptr = (mdata->stream.data)[++mdata->line];
        break;
    case XPMFILE:
    case XPMPIPE:
        if (mdata->Eos)
            while ((c = xpmGetC(mdata)) != mdata->Eos && c != EOF);
        if (mdata->Bos)                 /* if not natural XPM2 */
            while ((c = xpmGetC(mdata)) != mdata->Bos && c != EOF);
        break;
    }
}

/*
 * return the current character, skipping comments
 */
static int xpmGetC
ARGLIST((mdata))
GRA(xpmData *, mdata)
{
    int c;
    register unsigned int n = 0, a;
    unsigned int notend;

    switch (mdata->type) {
    case XPMARRAY:
        return (*mdata->cptr++);
    case XPMFILE:
    case XPMPIPE:
        c = getc(mdata->stream.file);

        if (mdata->Bos && mdata->Eos
            && (c == mdata->Bos || c == mdata->Eos)) {
            /* if not natural XPM2 */
            mdata->InsideString = !mdata->InsideString;
            return (c);
        }
        if (!mdata->InsideString && mdata->Bcmt && c == mdata->Bcmt[0]) {
            mdata->Comment[0] = c;

            /*
             * skip the string begining comment
             */
            do {
                c = getc(mdata->stream.file);
                mdata->Comment[++n] = c;
            } while (c == mdata->Bcmt[n] && mdata->Bcmt[n] != '\0'
                     && c != EOF);

            if (mdata->Bcmt[n] != '\0') {
                /* this wasn't the begining of a comment */
                /* put characters back in the order that we got them */
                for (a = n; a > 0; a--)
                    xpmUngetC(mdata->Comment[a], mdata);
                return (mdata->Comment[0]);
            }

            /*
             * store comment
             */
            mdata->Comment[0] = mdata->Comment[n];
            notend = 1;
            n = 0;
            while (notend) {
                while (mdata->Comment[n] != mdata->Ecmt[0] && c != EOF) {
                    c = getc(mdata->stream.file);
                    mdata->Comment[++n] = c;
                }
                mdata->CommentLength = n;
                a = 0;
                do {
                    c = getc(mdata->stream.file);
                    n++;
                    a++;
                    mdata->Comment[n] = c;
                } while (c == mdata->Ecmt[a] && mdata->Ecmt[a] != '\0'
                         && c != EOF);
                if (mdata->Ecmt[a] == '\0') {
                    /* this is the end of the comment */
                    notend = 0;
                    xpmUngetC(mdata->Comment[n], mdata);
                }
            }
            c = xpmGetC(mdata);
        }
        return (c);
    default:
	abort();
    }
  return 0;
}


/*
 * push the given character back
 */
static int xpmUngetC
ARGLIST((c, mdata))
ARG(int, c)
GRA(xpmData *, mdata)
{
    switch (mdata->type) {
    case XPMARRAY:
        return (*--mdata->cptr = c);
    case XPMFILE:
    case XPMPIPE:
        if (mdata->Bos && (c == mdata->Bos || c == mdata->Eos))
            /* if not natural XPM2 */
            mdata->InsideString = !mdata->InsideString;
        return (ungetc(c, mdata->stream.file));
    default:
	abort();
    }
  return 0;
}

/*
 * skip whitespace and return the following word
 */
static unsigned int xpmNextWord
ARGLIST((mdata, buf))
ARG(xpmData *, mdata)
GRA(char *, buf)
{
    register unsigned int n = 0;
    int c;

    switch (mdata->type) {
    case XPMARRAY:
        while (isspace(c = *mdata->cptr) && c != mdata->Eos)
            mdata->cptr++;
        do {
            c = *mdata->cptr++;
            buf[n++] = c;
        } while (!isspace(c) && c != mdata->Eos && c != '\0');
        n--;
        mdata->cptr--;
        break;
    case XPMFILE:
    case XPMPIPE:
        while (isspace(c = xpmGetC(mdata)) && c != mdata->Eos);
        while (!isspace(c) && c != mdata->Eos && c != EOF) {
            buf[n++] = c;
            c = xpmGetC(mdata);
        }
        xpmUngetC(c, mdata);
        break;
    }
    return (n);
}

static int xpmVisualType
ARGLIST((visual))
GRA(Visual *, visual)
{
#if defined(__cplusplus) || defined(c_plusplus)
    switch ( visual->c_class )
#else
    switch ( visual->class )
#endif
    {
    case StaticGray:
    case GrayScale:
	switch (visual->map_entries)
	{
	case 2:
	    return (MONO);
	case 4:
	    return (GRAY4);
	default:
	    return (GRAY);
	}
    default:
	return (COLOR);
    }
}

/*
 * Free the computed color table
 */

static void xpmFreeColorTable
ARGLIST((colorTable, ncolors))
ARG(char ***, colorTable)
GRA(int, ncolors)
{
    int a, b;

    if (colorTable) {
        for (a = 0; a < ncolors; a++)
            if (colorTable[a]) {
                for (b = 0; b < (NKEYS + 1); b++)
                    if (colorTable[a][b])
                        free(colorTable[a][b]);
                free((char *)colorTable[a]);
            }
        free((char *)colorTable);
    }
}
#endif
