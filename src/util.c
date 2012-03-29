/**
* duser - Manage MajorDomo lists
* Copyright (C) 2011 Joseph Hunkeler <jhunkeler@gmail.com, jhunk@stsci.edu>
*
* This file is part of duser.
*
* duser is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* duser is distributed in the hope that it will be useful,
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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include "duser.h"

char *__basename(char *path)
{
    char *base = strrchr(path, '/');
    return base ? base+1 : path;
}

void stats_init(stats_t *s)
{
	s->lines = 0;
	s->files = 0;
	s->matches = 0;
	s->added = 0;
	s->deleted = 0;
	s->modified = 0;
}

int user_choice(char c)
{
	if(c == 'y' || c == 'Y')
		return 0;

	return 1;
}

int check_cmd_string(char** args, const char* str2, int count)
{
	int i = 0;
	while(i < count)
	{
		if((strncmp(args[i], str2, strlen(args[i]))) == 0)
		{
			return 0;
		}
		i++;
	}
	return -1;
}

//I'm using this until regex can get its head out of its... 
//toilet bowl.
int strfind(const char* str1, const char* str2)
{
	if((strcasestr(str1, str2)) != 0)
	{
		if((strcasecmp(str1, str2)) == 0)
			return 0;
	}
	return -1;
}

int strval(const char* str)
{
	const char* bad = "!#$%^&*()+={}[]|\\<>,";
	unsigned int i = 0;
	unsigned int ibad = 0;
	for(i = 0 ; i < strlen(str) ; i++)
	{
		unsigned char c = str[i]; 
		if((i == 0) && !(isalpha(c)))
		{
			return -1;
		}
		for(ibad = 0 ; ibad <= strlen(bad) ; ibad++)
		{
			if(str[i] == bad[ibad])
			{
				return -1;
			}
		}
	}

	return 0;
}

int touch(const char* filename, mode_t mode)
{
	size_t bytes = 0;
	int fd = -1;

	if((fd = open(filename, O_CREAT | O_WRONLY, mode)) < 0)
	{
		COM(SELF, "FATAL: %s: %s: %s\n", filename, strerror(errno));
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, filename, strerror(errno));
		return errno;
	}
	char byte = '\0';
	if((bytes = write(fd, &byte, 1)) < 1)
	{
		return errno;
	}
	close(fd);
	return 0;
}
