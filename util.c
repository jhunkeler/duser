#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "duser.h"

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

