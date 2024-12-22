/*
	Program to search for Minimal Perfect Hash Functions for
	use in lexical analyzers. C.D. Havener Jan 26 1982
	GenRad Inc. 37 Great Road, Bolton Mass. 01740
	Based on paper "Minimal Perfect Hash Functions Made Simple"
	by Richard J. Cichelli - Comm. of ACM Jan 1980 pp 17-19

	Synopsis: The hash function is h = assoc value of 1st
	letter + length of keyword + assoc value of last letter

	This program finds the associated values of the letters
	given a list of keywords, 1 per line. It works most of
	the time for up to about 40 keywords but certain
	pathalogical cases exist. A semi-perfect hash is usually
	found by the program. The user can then tighten the
	default limits for max associated char value or the
	table limit using the -v and -t options. Sometimes the
	presort heuristics actually make the search process
	much more difficult. The user can try his luck at manual
	sorting using the -n option. Since the hash function
	produces such a limited range of numbers it can only work
	for up to about 40 keywords. If a language needs say 80
	keywords just split them up into two tables and let the
	lexical analyzer look in first one then the other, this
	will still be much faster than any other keyword lookup.

	This program has run sucessfully on a VAX 11/780 under
	4.1BSD. 
*/

#include "perf.h"	/* misc standard defines used at GenRad */
#define UNIX 1
#ifdef UNIX
#include <signal.h>
#undef EOF
#include <stdio.h> /* needed for the stream I/O */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif
#define STATIC static
#define MAXKEYS 100
#define MAXCHARS 0377
#ifdef UNIX
#define BPW 4		/* the number of bytes per word on this computer */
#else
#define BPW 2
#endif
#define UNDEF -1	/* the undefined value */


	/* no attempt to be efficient on a VAX with mucho memory */
TINY cval[MAXCHARS];	/* a place for every char known to Ascii */
COUNT cused[MAXCHARS];	/* keep count of how many times char used */
COUNT order[MAXKEYS];	/* current ordering of key words by subscript */
COUNT neworder[MAXKEYS];	/* the new supposedly improved ordering */
COUNT hashval[MAXKEYS];	/* the current hashvalue of the key word */
COUNT value[MAXCHARS];	/* the associated value of the character */
BOOL mapped[MAXKEYS];	/* keeps track of which table entries are in use */
TEXT name[50];		/* much bigger than any keyword should be ! */
TEXT *keywds[MAXKEYS];
#ifdef UNIX
VOID status();
LONG time();
TEXT *ctime();
#endif

COUNT debug = 0;
COUNT nkeys = sizeof(keywds)/BPW; 
COUNT tablesize = sizeof(keywds)/BPW;	
COUNT trys = 0;
COUNT nletters = 0;
COUNT kilotrys = 0;
COUNT atime = 600;	/* default alarm status time 10 min. */
COUNT hash();
BOOL aredefined();
TEXT *klimit = 0;
TEXT *textp =0;
LONG bigcount = 0;
COUNT depth = 0;
COUNT k_now = 0;	/* the present value of k for status reports */
BOOL sort = TRUE;

TEXT lettrs[37];	/* string of defined chars */
COUNT ptr = 0;

/*-----------------------------------------------------------*/

main(argc,argv)
	int argc;
	char *argv[];
{
	COUNT i,j,k,m = 0;
	STATIC COUNT vlimit = 0;
	STATIC COUNT keylimit = 0;
	STATIC COUNT tlimit = 0;
	LONG start,stop;		/* the start and finish times */
	char *u;

	while ( --argc > 0 && (*++argv)[0] == '-')
		for ( u = argv[0]+1 ; *u != '\0' ; u++ )
			switch(*u)
			{
			/* things like vlimit,tlimit don't work as stack */
			/* variables! sscanf doesn't like them on stack */
			case 'v': /* assoc value limit */
				textp = *++argv;
				sscanf(textp,"%d",&vlimit);
				break;
			case 't': /* tablesize limit */
				textp = *++argv;
				sscanf(textp,"%d",&tlimit);
				break;
			case 'a': /* alarm time for status */
				textp = *++argv;
				sscanf(textp,"%d",&atime);
				break;
			case 'k': /* keyword limit */
				klimit = *++argv;
				sscanf(klimit,"%d",&keylimit);
				break;
			case 'n': /* no heuristic sort wanted */
				sort = FALSE;
				break;
			case '1':	/* debug printout level */
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				debug = *u - '0';
				break;
			default:
				printf("\nt32: illegal option %c\n", *u);
				printf("\nUsage: -n -t # -k # -v # -# -a #"); 

			}
#ifdef UNIX
	start = time(0);
	signal(SIGALRM,(sig_t)status);
	signal(SIGTERM,(sig_t)status);	/* on kill -TERM pid , give status */
	alarm(atime);	/* print status every N secs */
#endif

	i = 0;
	while ( (scanf("%s",name)) != EOF )
		{
		keywds[i++] = u = malloc(strlen(name)+1);
		if ( u == 0 )
			{
			printf("\n malloc() failure ");
			exit(1);
			}
		strcpy(u,name);
		}
	nkeys = ( keylimit == 0 ? i : keylimit );

	for ( i=0 ; i<MAXKEYS ; i++)
		{
		hashval[i] = UNDEF;
		order[i] = i;
		mapped[i] = FALSE;
		}

	for ( i=0 ; i<MAXCHARS ; i++ )
		{
		cval[i] = 0;
		value[i] = UNDEF;
		}

	if ( precheck() == FALSE )
		{
		printf("\nPerfect hash search terminated \n");
		exit(1);
		}


	for ( i=0 ; i<nkeys ; i++ )
		{
		j = *keywds[i];
		k = *(keywds[i] + strlen(keywds[i]) -1  );
		cval[j] = j;	/* put char val in array at chars subscript value */
		cval[k] = k;
		if ( cused[j] == 0 )
			++nletters;
		++cused[j];	/* keep count of how often each letter is used */
		if ( cused[k] == 0 )
			++nletters;	/* also count distinct letters */
		++cused[k];
		}
	tablesize = (tlimit == 0 ? MAXKEYS : tlimit );
	printf("\nPERFECT HASH FUNCTION FINDER , CDH Ver. 2.9\n");
	printf("\nStart time = %s",ctime(&start) );
	printf("Number of keywords = %d",nkeys);
	printf("\nNumber of distinct letters = %d",nletters);
	nletters = ( vlimit > 0 ? vlimit : nletters );
	printf("\nThe associated char value limit is %d",nletters );
	printf("\nThe table size limit is %d",tablesize);
	/* usually make it at least nkeys + 1 since first hash */
	/* value is usually 1 or 2 even if both assoc char values */
	/* are zero since the keyword length is included !  */


	if ( sort == FALSE )
		{
		printf("\n ----> Presorting of keywords is turned off");
		goto begin;
		}

	/* first order by sum of frequencies of occurrences of each */
	/* keys 1st and last letter */

	for ( i=0 ; i < nkeys ; i++ )
		{
		order[i] = cused[*keywds[i]] + cused[*(keywds[i]
						+ strlen(keywds[i]) -1)];
		}

	for ( m=0 ; m<nkeys ; m++ )
		{
		for ( k = -1 , i=0 ; i<nkeys ; i++ )
			{
			if ( order[i] > k )
				{
				k = order[i];
				j = i;			/* remember the keywd subscript */
				}
			}
		order[j] = 0;
		neworder[m] = j;
		}

	for ( i=0 ; i<nkeys; i++ )
		order[i] = neworder[i];

	if ( debug > 2 )
		{
		printf("\nAfter first ordering\n");
		prntorder();
		}

/* the second ordering follows */
/* keywds whose values are defined by keywds earlier in the */
/* order are placed immediately after they are defined. */
/* This causes hash value conflicts to occur as early during */
/* the search as possible */


	for ( i=0 ; i<sizeof(lettrs) ; i++ )
		lettrs[i] = '\0';
	ptr = 0;
	merge(order[0]);	/* prime the pump */
	neworder[0] = order[0];
	order[0] = UNDEF;
	for ( i=1 ; i<nkeys ; )
		{
		BOOL newvalues = TRUE;
		while ( newvalues )
			{
			newvalues = FALSE;
			for ( k=0 ; k<nkeys ; k++ )
				{
				if ( order[k] == UNDEF )
					continue;
				if ( aredefined(order[k]) )
					{
					neworder[i++] = order[k];
					order[k] = UNDEF;
					continue;
					}
				}
			for ( k=0; k<nkeys ; k++ )
				if ( order[k] != UNDEF )
					{
					neworder[i++] = order[k];
					merge(order[k]);
					order[k] = UNDEF;
					newvalues = TRUE;
					break;
					}
			}
		}

	for ( i=0 ; i<nkeys ; i++ )
		order[i] = neworder[i];

	if ( precheck() == FALSE)
		{
		printf("\nOOPS - call a Guru, the presort botched it");
		prntorder();
		exit(1);
		}

/* - - - - - - - BEGIN SEARCHING - - - - - - - - - - - - - - - */

begin:	printf("\n The search ordering is \n");
	prntorder();
	fflush(stdout); /* needed to force initial text out */

	if ( search(0) )
		{
		printf("\nSUCCESS ! Associated Char Values Follow: \n");
		prntvals();
		prnthash();
		}
	else
		{
		printf("\nFAILED to find char values for hash function");
		}
	printf("\nTotal search() invocations = %ld ",bigcount);
	stop = time(0);
	printf("\nStarted %s",ctime(&start));
	printf("Finished %s",ctime(&stop));
	printf("\n");
}

/*-----------------------------------------------------------*/
/* merge - adds keywd letters to the string of those defined */
VOID merge(n)
	ARGINT n;
{
	BOOL install_a,install_b = TRUE;
	TEXT a,b;
	COUNT i;

	if ( debug > 2 )
		printf("\nmerging in %s",keywds[n]);
	a = *(keywds[n]);
	b = *(keywds[n] + strlen(keywds[n]) - 1 );
	for ( i=0 ; i <= ptr ; i++ )
		{
		if ( a == lettrs[i] )
			install_a = FALSE;
		if ( b == lettrs[i] )
			install_b = FALSE;
		}
	if ( install_a )
		lettrs[ptr++] = a;
	if ( install_b && ( a != b ) )
		lettrs[ptr++] = b;
}

/*----------------------------------------------------------*/
/* aredefined - see if 1st & last char of keywd are defined */

BOOL aredefined(n)
	ARGINT n;
{
	TEXT a,b;
	COUNT i,k;

	a = *keywds[n];
	b = *(keywds[n] + strlen(keywds[n]) - 1 );
	for ( i=0, k=0 ; i <= ptr ; i++ )
		{
		if ( (a == lettrs[i]) || (b == lettrs[i]) )
			k++;
		}
	return ( k==2 ? TRUE : FALSE );
}

/*---------------------------------------------------------*/
/* precheck - all keywds length,1st and last char disjoint */

BOOL precheck()
{
	BOOL pretest = TRUE;
	COUNT i,j,a,b,c,d,leni,lenj;
	COUNT m,k;

	for ( m = 0 ; m < nkeys ; m++ )
		{
		i = order[m];
		for ( k = m+1; k < nkeys-1 ; k++ )
			{
			j = order[k];
			if ( (leni=strlen(keywds[i]) ) ==
				( lenj=strlen(keywds[j]) ) )
				{
				a = *keywds[i];
				b = *(keywds[i] + leni - 1);
				c = *keywds[j];
				d = *(keywds[j] + lenj -1 );
				if ( (a==c && b==d) || ( a==d && b==c) )
					{
					pretest = FALSE;
					printf("\nPrecheck fails on %s and %s",
									keywds[i],keywds[j]);

					}
				}
			}
		}
	return pretest;
}
/*--------------------------------------------------------*/
/* prntorder - printout the current order of the keywords */

VOID prntorder()
{
	COUNT i,j;
	for( i=0, j=0 ; i<nkeys ; i++, j++ )
		{
		if ( j >= 8 )
			{
			printf("\n");
			j = 0;
			}
		printf("%s ",keywds[order[i]]);
		}
	printf("\n");
}

/*----------------------------------------------------*/
/* prntvals - prints out the letter associated values */

prntvals()
{

		COUNT i,j;
		for( i=0, j=0 ; i<MAXCHARS ; j++ ,i++ )
			if( cval[i] )
				{
				if( j > 10 )
					{
					j = 0;
					printf("\n");
					}
				printf("%c  %d,",cval[i],value[i]);
				}
	printf("\n");
}

/*------------------------------------------------------*/
/* prnthash - prints out the hash values for the keywds */

VOID prnthash()
{

		COUNT i,j;
		BOOL swap = TRUE;
		COUNT hmin = MAXKEYS;
		COUNT hmax = 0;
		COUNT spread = 0;

		for ( i = 0 ; i < nkeys ; i++ )
			{
			j = hashval[i] = hash( keywds[i] );
			hmin = ( hmin < j ? hmin : j );
			hmax = ( hmax > j ? hmax : j );
			order[i] = i;
			}
		while ( swap )		/* plain vanilla bubble sort */
			{
			swap = FALSE;
			for ( i = 0 ; i < nkeys-1 ; i++ )
					{
					if ( hashval[order[i+1]] < hashval[order[i]] )
						{	
						swap = TRUE;
						j = order[i];
						order[i] = order[i+1];
						order[i+1] = j ;
						}
					}
			}
		printf("\nHash min =%d, max =%d, spread = %d\n",
					hmin,hmax,hmax-hmin+1);

		for ( i=0, j=0 ; i<nkeys ; i++, j++)
				{
				if ( j >= 8 )
					{
					printf("\n");
					j = 0;
					}
				printf("%s %d,",keywds[order[i]],
							hash(keywds[order[i]]));
				}
		printf("\n");
}

/*--------------------------------------------------------*/

/* hash - passed ptr to string. Returns hash value 	*/
COUNT hash(p)
	TEXT *p;
{
	return( value[*p] + strlen(p) + value[ *(p + strlen(p) -1) ] );
}

/*-------------------------------------------------------*/
/* search - calls itself recursively to find char values */

BOOL search(k)
	COUNT k;
{
	TEXT *p;
	COUNT v1,v2,num,m;
	COUNT sub1,sub2,subn;
	BOOL thesame = FALSE;

	bigcount++;
	k_now = k;			/* for status reporting */
	if ( k >= nkeys )	/* hey - we may be all done */
		return TRUE;

	if ( k > depth )
		depth = k;		/* keep track of how deep we searched */

	m = order[k];
	p = keywds[m];
	sub1 = *p;
	sub2 = *(p + strlen(p) -1);
	if ( sub1 == sub2 )
		thesame = TRUE;
	v1 = value[sub1];
	v2 = value[sub2];

	if ( defined(v1)  && defined(v2)  )
		{
		num = hash(p);
		if ( used(num) )
			return FALSE;	/* bad hash value, already in use */
		else
			{
			hashval[m] = num;	/* install it */
			mapped[num] = TRUE;
			if ( search(k+1) )
				return TRUE;
			else
				{
				hashval[m] = UNDEF;
				mapped[num] = FALSE;
				return FALSE;
				}
			}
		}
	else if ( defined(v1) )
			{
			COUNT j;
			for ( j=0 ; j<=nletters ; j++)
				{
				v2 = j;
				num = v1 + strlen(p) + v2;
				if ( !used(num) )
					{
					hashval[m] = num;
					mapped[num] = TRUE;
					value[sub2] = v2;
					subn = sub2;
					if ( search(k+1) )
						return TRUE;
					else
						remove_hash(m,sub2);
					}
				}
			return FALSE;
			}
	else if ( defined(v2) )
			{
			COUNT j;
			for ( j=0 ; j<=nletters ; j++ )
				{
				v1 = j;
				num = v1 + strlen(p) + v2;
				if ( !used(num) )
					{
					hashval[m] = num;
					mapped[num] =TRUE;
					value[sub1] = v1;
					subn = sub1;
					if ( search(k+1) )
						return TRUE;
					else
						remove_hash(m,sub1);
					}
				}
			return FALSE;
			}

	else		/* neither defined */
		{
		COUNT j;
		for ( j=0 ; j <= nletters ; j++ )
			{
			if ( thesame )
				{
				v1 = v2 = j;
				num = v1 + strlen(p) + v2;
				if ( !used(num) )
					{
					hashval[m] = num;
					mapped[num] = TRUE;
					value[sub1] = v1;	/* same place as value[sub2] */
					subn = sub1;
					if ( search(k+1) )
						return TRUE;
					else
						remove_hash(m,subn);
					}
				}
			else
				{
				value[sub1] = j;
				if ( search(k) )	/* if never TRUE 
								thru for loop, then FALSE */
					return TRUE;
				else
					value[sub1] = UNDEF;
				}
			}
		return FALSE;
		}
}

/*-------------------------------------------------------*/
/* remove - backup by deleting keywds hash value etc */

VOID remove_hash(m,subn)
	FAST COUNT m;
	FAST COUNT subn;
{
	if ( debug > 6)
		printf("\nremoving %s, subn = %d",keywds[m],subn);
	mapped[ hashval[m] ] = FALSE;
	hashval[m] = UNDEF;
	value[subn] = UNDEF;
}

/*--------------------------------------------------------*/
/* used - tests if the hash value is in use or is illegal */

BOOL used(num)
	FAST COUNT num;
{
	return( num > tablesize || num < 0 ? TRUE : mapped[num] );
}

/*-----------------------------------------------------*/
/* defined - checks if the character value is defined yet */

BOOL defined(value)
	FAST COUNT value;
{
	return( value != UNDEF ? TRUE : FALSE );
}
/*--------------------------------------------------------*/
/* status - on signal this reports the current statistics */

#ifdef UNIX
VOID status(int unused)
{
	(void)unused;
	fprintf(stderr,
		"\nSTATUS: nkeys=%d depth=%d k_now=%d, bigcount=%ld\n"
					,nkeys,depth,k_now,bigcount);
	fflush(stderr);

	signal(SIGTERM,(sig_t)status);
	signal(SIGALRM,(sig_t)status);
	alarm(atime);
}
#endif

