#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include "duser.h"

extern char logfile[PATH_MAX];
//extern char* logfile;

int logcleanup()
{
    int status = 0;
    if((access(logfile, W_OK)) == 0)
    {
        status = unlink(logfile);
    }

    return status;
}

int COM(const char* func, char *format, ...)
{
    struct tm *logtm;
    time_t logtime = time(NULL);
    char timestr[64];
    char *str = (char*)malloc(sizeof(char) * 256);
    char tmpstr[255];
    int n;

    FILE *logfp = NULL;
	logfp = fopen(logfile, "a+");
	if(logfp == NULL)
    {
    	fprintf(stderr, "Unable to open %s\n", logfile);
		exit(1);
    }
    
    va_list args;
    va_start (args, format);
    n = vsprintf (str, format, args);
    va_end (args);

    logtm = localtime(&logtime);

    snprintf(timestr, sizeof(timestr), "%02d-%02d-%02d %02d:%02d:%02d", logtm->tm_year+1900, logtm->tm_mon+1, logtm->tm_mday, logtm->tm_hour, logtm->tm_min, logtm->tm_sec);
    snprintf(tmpstr, sizeof(tmpstr), "%s %d _%s_: %s", timestr, getuid(), func, str);
    fprintf(logfp, "%s", tmpstr);
    fclose(logfp);
    
    free(str);
    return 0;
}
