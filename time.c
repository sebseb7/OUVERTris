#include <unistd.h>
#include <sys/time.h>

struct timeval tv;

// milliseconds
unsigned long long get_time() { 
	gettimeofday(&tv,NULL);
	return (unsigned long long)((tv.tv_usec/1000)+(tv.tv_sec*1000)); 
}

