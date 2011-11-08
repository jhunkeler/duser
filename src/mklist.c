/**
* duser - Manage MajorDomo lists
* Copyright (C) 2011 Joseph Hunkeler <jhunkeler@gmail.com, jhunk@stsci.edu>
*
* This file is part of duser.
*
* duser is free software: you can redifilenameibute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* duser is difilenameibuted in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with duser. If not, see <http://www.gnu.org/licenses/>.
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <libgen.h>
#include "duser.h"

char* str_replace(char* str, char* old, char* new)
{
	static char buffer[BUFSIZ];
	char *p;
	if(!(p = strstr(str, old)))
		return NULL;

	strncpy(buffer, str, p-str);
	buffer[p-str] = 0;
	sprintf(buffer+(p-str), "%s%s", new, p+strlen(old));
	
	return buffer;
}

char* getfile(const char* filename)
{
	int bytes = 0;
	static char buffer[BUFSIZ];
	FILE* fp = NULL;
	if((fp = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "%s: fopen: %s\n", SELF, strerror(errno));
		return NULL;
	}

	memset(buffer, 0, strlen(buffer));
	if((bytes = fread(buffer, BUFSIZ, 1, fp)) < 0)
	{
		fprintf(stderr, "%s: fread: %s\n", SELF, strerror(errno));
		exit(1);
	}
	buffer[strlen(buffer)] = 0;
	fclose(fp);

	return buffer;
}

static char* mailfmt = "mail -s '%s' -r %s %s"; 
int mailer(const char* subj, const char* from, const char* to, const char* mesg)
{
	char mail[BUFSIZ];
	if((snprintf(mail, BUFSIZ, mailfmt, subj, from, to)) < 1 )
	{
		fprintf(stderr, "%s: Failed to generate mail string.\n", SELF);
		return -1;
	}

	FILE *pipe;
	if((pipe = popen(mail, "w")) == NULL)
	{
		fprintf(stderr, "%s: Unable to open pipe.\n", SELF);
		return -1;
	}
	fputs(mesg, pipe);
	pclose(pipe);
	return 0;
}

int mklist(char* filename)
{
	uid_t uid;
	struct passwd *pwd;
	char tmpname[strlen(filename)+1];
//	char* list = basename(filename);
	int status = 0;
	uid = getuid();
	pwd = getpwuid(uid);

	if((status = access(filename, F_OK)) == 0)
	{
		COM(SELF, "FATAL: %s: '%s' already exists\n", basename(filename), strerror(errno));
		fprintf(stderr, "'%s' already exists\n", basename(filename));
		return -1;
	}

	strncpy(tmpname, filename, strlen(filename));
	strncat(tmpname, ".info", strlen(filename));
	if((status = touch(tmpname, 0644)) != 0)
	{
		COM(SELF, "FATAL: %s: %s: %s\n", tmpname, strerror(errno));
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, tmpname, strerror(errno));
	}

	char* tmptpl;
//	char* tpl; 
	if((tmptpl = getfile("/usr/local/etc/duser/tpl/aliases")) == NULL)
	{
		fprintf(stderr, "%s: %s\n", SELF, strerror(errno));
		exit(1);
	}

	return 0;
}
