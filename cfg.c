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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "duser.h"

#define CFG_MAX		255
FILE *cfgfp;

int cfg_open(const char* filename)
{
	if((cfgfp = fopen(filename, "r")) == NULL)
		return errno;
	else
		return 0;
}

void cfg_close()
{
	fclose(cfgfp);
}

//Value of key
int cfg_get_key(char* val, const char* key)
{

	if(cfgfp == NULL)
		return -1;

	if((fseek(cfgfp, 0L, SEEK_SET)) != 0)
		return errno;

	char buffer[CFG_MAX];
	char *buf = buffer; //= (char*)malloc(sizeof(char) * CFG_MAX+1);
	if(buffer == NULL)
	{
		fprintf(stderr, "%s: %s\n", SELF, strerror(errno));
		exit(1);
	}

	int i = 0;
	while(!feof(cfgfp))
	{
		memset(buffer, 0L, CFG_MAX);
		fgets(buffer, CFG_MAX, cfgfp);
		if((strcasestr(buffer, key)))
		{
			while(buf)
			{
				if(isspace(*buf))
				{
					strncpy(val, &buf[i+1], CFG_MAX);
					val[strlen(val)-1] = 0;
					break;
				}
				buf++;
			}
			i++;
			break;
		}
	}

	return 0; 
}
