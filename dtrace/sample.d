#!/usr/sbin/dtrace -Zs
#pragma D option quiet

dtrace:::BEGIN
{
	printf("Tracing... Hit Ctrl-C to end.\n");
}

jsondata*:::
{
        printf("%8d: %s[%s](%d)\n", pid, probefunc, probename, arg0);
}

dtrace:::END
{
}
