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
#include <dirent.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <libgen.h>

#include "cfg.h"
#include "duser.h"

char list_path[PATH_MAX];
char logfile[PATH_MAX];
static char progname[FILENAME_MAX];

int CMD_FLAG_NOOPT = 0;
int CMD_FLAG_DEL = 0;
int CMD_FLAG_DEL_ALL = 0;
int CMD_FLAG_DEL_LIST = 0;
int CMD_FLAG_MOD = 0;
int CMD_FLAG_ADD = 0;
int CMD_FLAG_LIST = 0;
int CMD_FLAG_HELP = 0;
int CMD_FLAG_LOOK = 0;
int CMD_FLAG_NEW = 0;
int CMD_FLAG_NULL = 0;

void usage(const char* progname)
{
	printf("Domouser v0.1a - jhunk@stsci.edu\n");
	printf("Usage: %s command [address [list]]\n", progname);
	printf("Commands:\n");
	printf("  help		This usage statement\n");
	printf("  add		Add a user to a list\n");
	printf("  del		Delete a user from a list\n");
	printf("  delA		Delete a user from all lists\n");	
	printf("  delL		Completely delete a mailing list\n");
	printf("  mod		Modify a user in a list\n");
	printf("  new		Creates a new list\n");
	printf("  list		Find and list a user in all lists\n");
	printf("  look		Find user in specific list\n");
	printf("\n");
	exit(0);
}

int main(int argc, char* argv[])
{
	if((cfg_open(CFG_PATH)) == 0)
	{
		cfg_get_key(list_path, "path");
		cfg_get_key(logfile, "logfile");
		cfg_close();
	}
	else
	{
		fprintf(stderr, "%s: %s: %s\n", SELF, CFG_PATH, strerror(errno));
		return -1;
	}

	char filename[PATH_MAX];
	record_t *rec;
	strncpy(progname, argv[0], strlen(argv[0]));
	const char* needle = argv[2];
	const char* single_list = argv[3];

	if(single_list)	
		snprintf(filename, PATH_MAX, "%s%s", list_path, single_list);

	if(argc < 3)
	{
		usage(progname);
		return 0;
	}

	stats_init(&processed);
	user_cmd(argc, argv);

	if((strval(needle)) != 0)
	{
		fprintf(stderr, "%s: Invalid string\n", SELF);
		exit(1);
	}

	if(CMD_FLAG_DEL)
	{
		if(needle == NULL)
		{
			printf("You must specify an email address\n");
			return -1;
		}
		else if(single_list == NULL)
		{
			printf("You must specify a list to remove '%s' from\n", needle);
			return -1;
		}


		if((rec = find_in_file(filename, needle)) == NULL)
		{
			printf("'%s' not found in '%s'\n", needle, basename(filename));
			return -1;
		}
		
		printf("Please review the information below:\n\n");
		printf("Email:    %s\n", rec->name);
		printf("At line:  %d\n", rec->index);
		printf("In File:  %s\n", basename(rec->file));
		printf("\nDo you wish to wish to delete this record? [y/N] ");
			
		char choice = getchar();
		if((user_choice(choice)) == 0)
		{
			if((user_del(rec)) > 0)
			{
				printf("Record deleted\n");
				COM(SELF, "Commmand: DELETE\n");
				COM(SELF, "'%s' deleted from '%s' at line %d\n", rec->name, basename(rec->file), rec->index);
			}
		}
		else
		{
			printf("Aborting...\n");
		}
		return 0;
	}

	if(CMD_FLAG_DEL_ALL)
	{
		user_del_all(needle);
		return 0;
	}

	if(CMD_FLAG_DEL_LIST)
	{
		user_del_list(needle);
		return 0;
	}

	if(CMD_FLAG_ADD)
	{
		if(needle == NULL)
		{
			printf("You must specify an email address\n");
			return -1;
		}
		else if(single_list == NULL)
		{
			printf("You must specify a list in which to add '%s' to\n", needle);
			return -1;
		}

		printf("Please review the information below:\n\n");;
		printf("Email:    %s\n", needle);
		printf("In File:  %s\n", basename(filename));
		printf("\nDo you wish to wish to add this record? [y/N] ");
			
		char choice = getchar();
		if((user_choice(choice)) == 0)
		{
			if((user_add(filename, needle)) > 0)
			{
				printf("Record added\n");
				COM(SELF, "Commmand: ADD\n");
				COM(SELF, "'%s' added to '%s'\n", needle, basename(filename));
			}
		}
		else
		{
			printf("Aborting...\n");
		}
		return 0;
	}
	if(CMD_FLAG_MOD)
	{
		if(needle == NULL)
		{
			printf("You must specify an email address\n");
			return -1;
		}
		else if(single_list == NULL)
		{
			printf("You must specify a list in which to modify '%s' in\n", needle);
			return -1;
		}

		fprintf(stderr, "Not implemented, sorry.\n");
		return 0;
	}
	if(CMD_FLAG_LIST)
	{
		if(needle == NULL)
		{
			printf("You must specify an email address\n");
			return -1;
		}
		user_list(needle);
		return 0;
	} 
	if(CMD_FLAG_LOOK)
	{
		if(needle == NULL)
		{
			printf("You must specify an email address\n");
			return -1;
		}
		else if(single_list == NULL)
		{
			printf("You must specify a list in which to find '%s' in\n", needle);
			return -1;
		}
			
		rec = find_in_file(filename, needle);
		if(rec)
		{
			if(rec->match)
			{
				printf("Found '%s' at line %d of list '%s'\n", rec->name, rec->index, basename(rec->file));
			}
			else
			{
				printf("Corrupt record?  This is not supposed to happen.\n");
				return -1;
			}
		}
		else
		{
			printf("Not found in '%s'\n", single_list);
		}
		return 0;
	}
	if(CMD_FLAG_NEW)
	{
		memset(filename, 0L, PATH_MAX);
		snprintf(filename, PATH_MAX, "%s%s", list_path, needle);

		if(needle == NULL)
		{
			printf("You must specify a list to create\n");
			return -1;
		}

		printf("Please review the information below:\n\n");;
		printf("File:  %s\n", filename);
		printf("\nDo you wish to wish to create this list? [y/N] ");
			
		char choice = getchar();
		if((user_choice(choice)) == 0)
		{
			if((user_new_list(filename)) == 0)
			{
				printf("List added\n");
				COM(SELF, "Commmand: NEW\n");
				COM(SELF, "Created new list '%s'\n", basename(filename));
			}
		}
		else
		{
			printf("Aborting...\n");
		}
		return 0;
	}
	if(CMD_FLAG_HELP)
	{
		usage(progname);
	}
	if(CMD_FLAG_NULL)
	{
		fprintf(stderr, "Command not specified\n");
		exit(1);
	}

	return 0;
}
