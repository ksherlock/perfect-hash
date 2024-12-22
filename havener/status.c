/* interprocess communication program for use with the perfect hash */
/* finding program. C.D. Havener Feb 19 1982. GenRad CTD Boloton Mass. */

#include <stdio.h>
#include <signal.h>

int wakeup();
int pid;
int interval;

main()
{

	printf("\nEnter PID: ");
	scanf("%d",&pid);
	printf("\nEnter reporting interval in seconds: ");
	scanf("%d",&interval);
	printf("\nWill send SIGTERM to process %d every %d seconds\n",pid,interval);
	signal(SIGALRM,wakeup);
	alarm(3);
	for(;;)
	{
		wait();	/* can't sleep() here it turns off alarm !! */
	}
}

wakeup()
{
	kill(pid,SIGTERM);
	signal(SIGALRM,wakeup);
	alarm(interval);
}
