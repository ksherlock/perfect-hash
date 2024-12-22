/*	perf.h - a subset of a "std.h"
*/
/* the pseudo storage classes
 */
#define AFAST	register
#define FAST	register
#define GLOBAL	extern
#define IMPORT	extern
#define INTERN	static
#define LOCAL	static

/* the pseudo types
 */
typedef char TEXT, TINY;
typedef double DOUBLE;
typedef int ARGINT, BOOL, VOID;
typedef long LONG;
typedef int COUNT;
typedef unsigned short BITS;
typedef unsigned BYTES;
typedef unsigned char UTINY;
typedef unsigned long ULONG;
typedef unsigned short UCOUNT;

/* system parameters
 */
#define STDIN	0
#define STDOUT	1
#define STDERR	2
#define YES		1
#define TRUE	1
#define NO		0
#define FALSE	0
#define NULL	0
#define FOREVER	for (;;)
#define BUFSIZE	512
#define BWRITE	-1
#define READ	0
#define WRITE	1
#define UPDATE	2
#define EOF		-1

/*	macros
 */
#define isalpha(c)	(islower(c) || isupper(c))
#define isdigit(c)	('0' <= (c) && (c) <= '9')
#define islower(c)	('a' <= (c) && (c) <= 'z')
#define isupper(c)	('A' <= (c) && (c) <= 'Z')
#define iswhite(c)	((c) <= ' ' || 0177 <= (c)) /* ASCII ONLY */
#define tolower(c)	(isupper(c) ? ((c) + ('a' - 'A')) : (c))
#define toupper(c)	(islower(c) ? ((c) - ('a' - 'A')) : (c))
