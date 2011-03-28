#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <tre/regex.h>
#include <unistd.h>
#include <fcntl.h>
#include "duser.h"


char* basename(const char* path)
{
	char *ptr = strrchr(path, '/');
	return ptr ? ptr + 1 : (char*)path;
}

int user_del(record_t* rec)
{
	FILE *tfp;
	FILE *fp;
	int fd = 0;
	int bytes = 0;
	char buf[REGEX_MAX];
	char tmpfile[255];
	snprintf(tmpfile, sizeof(tmpfile), "/tmp/duser.%s.XXXXXX", basename(rec->file));
	if((fd = mkstemp(tmpfile)) < 0 || (tfp = fdopen(fd, "w+")) == NULL)
	{
		if(fd != -1)
		{
			close(fd);
			unlink(tmpfile);
		}
		fprintf(stderr, "%s: %s\n", tmpfile, strerror(errno));
		exit(1);
	}
	
	if((fp = fopen(rec->file, "r")) == NULL)
	{
		fprintf(stderr, "%s: %s\n", rec->file, strerror(errno));
		exit(1);
	}

	while(!feof(fp))
	{
		memset(buf, 0, sizeof(buf));
		fgets(buf, REGEX_MAX, fp);
		buf[strlen(buf) - 1] = '\0';
		if((strncmp(buf, rec->name, strlen(rec->name))) != 0 )
		{
			buf[strlen(buf)] = '\n';
			bytes = write(fd, buf, strlen(buf));
		}
	}
	//Rewind the temp file
	lseek(fd, 0L, SEEK_SET);
	//Close the original file
	fclose(fp);
	//Truncate original and copy data from tmp to original
	if((fp = fopen(rec->file, "w+")) == NULL)
	{
		fprintf(stderr, "%s: %s\n", rec->file, strerror(errno));
		exit(1);
	}

	while(!feof(tfp))
	{
		memset(buf, 0, sizeof(buf));
		fgets(buf, REGEX_MAX, tfp);
		buf[strlen(buf) - 1] = '\0';
		if((strncmp(buf, rec->name, strlen(rec->name))) != 0 )
		{
			buf[strlen(buf)] = '\n';
			if(buf[0] == '\n')
				buf[0] = '\0';

			bytes = fwrite(buf, strlen(buf), 1, fp);
		}
	}

	fclose(fp);
	close(fd);
	unlink(tmpfile);
	
	if(bytes)
		return bytes;

	return 0;
}
// VERIFY that the record is proper (useful for deletion of users)
int find_in_file_ex(record_t* rec)
{
	int match = -1;
	char buf[REGEX_MAX];
	FILE *fp;
	
	if((fp = fopen(rec->file, "r")) == NULL)
	{
		perror("find_in_file_ex");
		return -1;
	}
	
	while(!feof(fp))
	{
		fgets(buf, REGEX_MAX, fp);
		buf[strlen(buf) - 1] = '\0';
		if((strncmp(buf, rec->name, strlen(rec->name))) == 0) 
		{
			match = 1;
			break;
		}
	}

	fclose(fp);
	return match;
}

record_t* find_in_file(const char* filename, const char* needle)
{
	regmatch_t pmatch[10];
	regex_t preg;
	record_t record, *rptr;
	rptr = &record;
	rptr->index = 0;
	rptr->match = 0;
	
	int index = 0;
	char* base = basename(filename);
	char regex[REGEX_MAX];
	FILE *fp;
	if((fp = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "Failed to process: %s\nReason: %s\n", base, strerror(errno));
		exit(1);
	}
	snprintf(regex, REGEX_MAX, regex_fmt, needle);
	strncpy(rptr->file, filename, PATH_MAX);
	snprintf(rptr->name, REGEX_MAX, "%s", needle);
	regcomp(&preg, regex, REG_EXTENDED|REG_ICASE|REG_NOSUB);
	while(!feof(fp))
	{
		char cmp[REGEX_MAX];
		memset(cmp, 0L, REGEX_MAX);
		fgets(cmp, REGEX_MAX, fp);
		cmp[strlen(cmp)-1] = '\0';
		if((regexec(&preg, cmp, 3, pmatch, REG_NOTBOL)) == 0)
		{
			rptr->match = 1;
			rptr->index = index;
		}
		index++;
		processed.lines++;
	}

	if(rptr->match)
		return rptr;
	else
		return NULL;

}

int get_file_count(const char* path)
{
	DIR *dp;
	struct dirent *ep;
	int file_count = 0;

	if((dp = opendir(path)) == NULL)
	{
		perror("opendir");
		exit(1);
	}

	while((ep = readdir(dp)))
	{
		if(ep->d_type == DT_REG && !strstr(ep->d_name, "."))
		{
			file_count++;
		}
	}
	closedir(dp);

	return file_count;
}

void free_file_list(char** list)
{
	int i = 0;
	for( i = 0 ; list[i] != NULL ; i++ )
	{
		free(list[i]);
	}
	free(list);
}

char** get_file_list(const char* path, int count)
{
	DIR *dp;
	struct dirent *ep;
	int i = 0;

	if((dp = opendir(path)) == NULL)
	{
		perror("opendir");
		exit(1);
	}

	char **list = (char**)calloc(count, REGEX_MAX);
	if(list == NULL)
	{
		perror("calloc");
		exit(1);
	}
	while((ep = readdir(dp)))
	{
		if(ep->d_type == DT_REG && !strstr(ep->d_name, "."))
		{
			list[i] = (char*)malloc(sizeof(char) * strlen(ep->d_name)+1);
			memset(list[i], 0L, strlen(ep->d_name));
			strncpy(list[i], ep->d_name, strlen(ep->d_name));
			i++;
		}
	}
	closedir(dp);
	return list;
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

int user_list(const char* needle)
{
	processed.files = get_file_count(list_path);
	char** list;
	list = get_file_list(list_path, processed.files);

	printf("%20s%12s\n", "List", "At line");
	printf("\t\t%s\n", "================");
	int i = 0;
	for(i = 0 ; list[i] != NULL ; i++)
	{
		char tmp[PATH_MAX];
		sprintf(tmp, "%s%s", list_path, list[i]);
		record_t *rp;
		if((rp = find_in_file(tmp, needle)) != NULL)
		{
			printf("%20s\t%5d\n", basename(rp->file), rp->index);
			processed.matches++;
		}
	}
	free_file_list(list);	
	return 0;
}

void usage(const char* progname)
{
	printf("Domouser v0.1a - jhunk@stsci.edu\n");
	printf("Usage: %s command address [list]\n", progname);
	printf("Commands:\n");
	printf("  help		This usage statement\n");
	printf("  add		Add a user to a list\n");
	printf("  del		Delete a user from all lists\n");
	printf("  mod		Modify a user in a list\n");
	printf("  list		Find and list a user in all lists\n");
	printf("  look		Find user in specific list\n");
	printf("\n");
	exit(0);
}


int user_cmd(const char* arg)
{
	if(arg)
	{
		if((strcmp(arg, "del")) == 0)
		{
			return CMD_FLAG_DEL;
		}
		if((strcmp(arg, "add")) == 0)
		{
			return CMD_FLAG_ADD;
		}
		if((strcmp(arg, "mod")) == 0)
		{
			return CMD_FLAG_MOD;
		}
		if((strcmp(arg, "list")) == 0)
		{
			return CMD_FLAG_LIST;
		}
		if((strcmp(arg, "look")) == 0)
		{
			return CMD_FLAG_LOOK;
		}
		if((strcmp(arg, "help")) == 0)
		{
			return CMD_FLAG_HELP;
		}
	}
	else
		return CMD_FLAG_NULL;

	return CMD_FLAG_NULL;
}

int user_choice(char c)
{
	if(c == 'y' || c == 'Y')
		return 0;

	return 1;
}

int main(int argc, char* argv[])
{
	const char* progname = argv[0];
	const char* command = argv[1];
	const char* needle = argv[2];
	const char* single_list = argv[3];
	char filename[PATH_MAX];
	record_t *rec;

	stats_init(&processed);
	int c = 0;

	if(argc < 2)
	{
		usage(progname);
		return 0;
	}

	if(needle && single_list)
		snprintf(filename, PATH_MAX, "%s%s", list_path, single_list);

	c = user_cmd(command);
	switch(c)
	{
		case CMD_FLAG_DEL:
			if(needle == NULL)
			{
				printf("You must specify an email address\n");
				return -1;
			}
			else if(single_list == NULL)
			{
				printf("You must specify a list in which to delete '%s' from\n", needle);
				return -1;
			}
			
			if((rec = find_in_file(filename, needle)) == NULL)
			{
				printf("Record not found\n");
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
				int success = 0;
				if((success = user_del(rec)) > 0)
				{
					printf("Record deleted\n");
					rec = NULL;
				}
			}
			else
			{
				printf("Exiting...\n");
			}

			break;
		
		case CMD_FLAG_ADD:
			printf("add flag active\n");
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
			break;

		case CMD_FLAG_MOD:
			printf("modify flag active\n");
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
			break;

		case CMD_FLAG_LIST:
			if(needle == NULL)
			{
				printf("You must specify an email address\n");
				return -1;
			}
			user_list(needle);
			break;

		case CMD_FLAG_LOOK:
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
				}
			}
			else
			{
				printf("Not found in '%s'\n", single_list);
			}

			break;
		case CMD_FLAG_HELP:
			usage(progname);
			break;

		default:
			printf("'%s' is not a valid command\n", command);
			usage(progname);
			break;
	};
/*
	printf("\n%d matches\t%d files\t%d lines parsed\n", 
		processed.matches, processed.files, processed.lines);
*/
	return 0;
}
