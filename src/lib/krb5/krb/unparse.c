/*
 * $Source$
 * $Author$
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * krb5_unparse_name() routine
 *
 * Rewritten by Theodore Ts'o to propoerly unparse principal names
 * which have the component or realm separator as part of one of their
 * components.
 */

#if !defined(lint) && !defined(SABER)
static char rcsid_unparse_c[] =
"$Id$";
#endif	/* !lint & !SABER */

#include <stdio.h>
#include <krb5/krb5.h>
#include <krb5/ext-proto.h>

/*
 * converts the multi-part principal format used in the protocols to a
 * single-string representation of the name. 
 *  
 * The name returned is in allocated storage and should be freed by
 * the caller when finished.
 *
 * Conventions: / is used to separate components; @ is used to
 * separate the realm from the rest of the name.  If '/', '@', or '\0'
 * appear in any the component, they will be representing using
 * backslash encoding.  ("\/", "\@", or '\0', respectively)
 *
 * returns error
 *	KRB_PARSE_MALFORMED	principal is invalid (does not contain
 *				at least 2 components)
 * also returns system errors
 *	ENOMEM			unable to allocate memory for string
 */

#define REALM_SEP	'@'
#define	COMPONENT_SEP	'/'

krb5_error_code
krb5_unparse_name_ext(principal, name, size)
krb5_const_principal principal;
register char **name;
int	*size;
{
	register char *cp, *q;
	register int i,j;
	int	length, nelem;
	register int totalsize = 0;

	cp = krb5_princ_realm(principal)->data;
	length = krb5_princ_realm(principal)->length;
	totalsize += length;
	for (j = 0; j < length; j++,cp++)
		if (*cp == REALM_SEP  || *cp == COMPONENT_SEP ||
		    *cp == '\0' || *cp == '\\' || *cp == '\t' ||
		    *cp == '\n' || *cp == '\b')
			totalsize++;
	totalsize++;		/* This is for the separator */

	nelem = krb5_princ_size(principal);
	for (i = 0; i < nelem; i++) {
		cp = krb5_princ_component(principal, i)->data;
		length = krb5_princ_component(principal, i)->length;
		totalsize += length;
		for (j=0; j < length; j++,cp++)
			if (*cp == REALM_SEP || *cp == COMPONENT_SEP ||
			    *cp == '\0' || *cp == '\\' || *cp == '\t' ||
			    *cp == '\n' || *cp == '\b')
				totalsize++;
		totalsize++;	/* This is for the separator */
	}

	/*
	 * Allocate space for the ascii string; if space has been
	 * provided, use it, realloc'ing it if necessary.
	 * 
	 * We need only n-1 seperators for n components, but we need
	 * an extra byte for the NULL at the end.
	 */
	if (*name) {
		if (*size < (totalsize)) {
			*size = totalsize;
			*name = realloc(*name, totalsize);
		}
	} else {
		*name = malloc(totalsize);
		if (size)
			*size = totalsize;
	}
	
	if (!*name)
		return ENOMEM;

	q = *name;
	
	for (i = 0; i < nelem; i++) {
		cp = krb5_princ_component(principal, i)->data;
		length = krb5_princ_component(principal, i)->length;
		for (j=0; j < length; j++,cp++) {
		    switch (*cp) {
		    case COMPONENT_SEP:
		    case REALM_SEP:
		    case '\0':
			*q++ = '\\';
			*q++ = '0';
			break;
		    case '\\':
			*q++ = '\\';
			*q++ = '\\';
			break;
		    case '\t':
			*q++ = '\\';
			*q++ = 't';
			break;
		    case '\n':
			*q++ = '\\';
			*q++ = 'n';
			break;
		    case '\b':
			*q++ = '\\';
			*q++ = 'b';
			break;
		    default:
			*q++ = *cp;
		    }
		}
		*q++ = COMPONENT_SEP;
	}

	q--;			/* Back up last component separator */
	*q++ = REALM_SEP;
	
	cp = krb5_princ_realm(principal)->data;
	length = krb5_princ_realm(principal)->length;
	for (j=0; j < length; j++,cp++) {
		switch (*cp) {
		case COMPONENT_SEP:
		case REALM_SEP:
		case '\t':
		case '\\':
			*q++ = '\\';
			*q++ = *cp;
			break;
		case '\0':
			*q++ = '\\';
			*q++ = '0';
			break;
		default:
			*q++ = *cp;
		}
	}
	*q++ = '\0';
	
    return 0;
}

krb5_error_code
krb5_unparse_name(principal, name)
krb5_const_principal principal;
register char **name;
{
	*name = NULL;
	return(krb5_unparse_name_ext(principal, name, NULL));
}


