#define _GNU_SOURCE
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
#include <libgen.h>
#include "duser.h"

char list_path[PATH_MAX];
char logfile[PATH_MAX];

int CMD_FLAG_NOOPT = 0;
int CMD_FLAG_DEL = 0;
int CMD_FLAG_DEL_ALL = 0;
int CMD_FLAG_DEL_LIST = 0;
int CMD_FLAG_MOD = 0;
int CMD_FLAG_ADD = 0;
int CMD_FLAG_LIST = 0;
int CMD_FLAG_HELP = 0;
int CMD_FLAG_LOOK = 0;
int CMD_FLAG_NULL = 0;

int user_del_list(const char* filename)
{
	char tmp[PATH_MAX];
	snprintf(tmp, PATH_MAX, "%s%s", list_path, filename);

	if((access(tmp, F_OK|W_OK)) != 0)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, tmp, strerror(errno));
		return -1;
	}
	
	printf("\n!!!WARNING!!!\n");
	printf("You are about to delete the mailing list '%s'\n\n", basename(tmp));
	printf("Are SURE you want to continue? [y/N]");
	int choice = getchar();
	if((user_choice(choice)) != 0)
	{
		printf("Aborting...\n");
		exit(1);
	}
	else
	{
		COM(SELF, "Command: DELETE LIST\n");
		if((unlink(tmp)) != 0)
		{
			fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, tmp, strerror(errno));
			return errno;
		}
		else
		{
			COM(SELF, "List '%s' deleted\n", basename(tmp));
			printf("Deleted list: '%s'\n", basename(tmp));
		}
	}

	return 0;
}

int user_del_all(const char* needle)
{
	processed.files = get_file_count(list_path);
	char** list;
	char tmp[PATH_MAX];
	record_t *rp;
	list = get_file_list(list_path, processed.files);

	int i = 0;
	for(i = 0 ; list[i] != NULL ; i++)
	{
		rp = NULL;
		snprintf(tmp, PATH_MAX, "%s%s", list_path, list[i]);
		if((rp = find_in_file(tmp, needle)) != NULL)
		{
			printf("%20s\t%5d\n", basename(rp->file), rp->index);
			processed.matches++;
		}
	}

	printf("\n%d matches\t%d files\t%d lines parsed\n", 
		processed.matches, processed.files, processed.lines);
	
	if(processed.matches < 1)
	{
		free_file_list(list);	
		return -1;
	}

	printf("\n!!!WARNING!!!\n");
	printf("You are about to delete '%s' from every list\n", needle);
	printf("that it appears in!\n\n");
	printf("Are SURE you want to continue? [y/N]");

	int choice = getchar();
	if((user_choice(choice)) != 0)
	{
		printf("Aborting...\n");
		exit(1);
	}
	else
	{
		COM(SELF, "Commmand: DELETE\n");
		for(i = 0 ; list[i] != NULL ; i++)
		{
			rp = NULL;
			snprintf(tmp, PATH_MAX, "%s%s", list_path, list[i]);
			if((rp = find_in_file(tmp, needle)) != NULL)
			{
				if((user_del(rp)) > 0)
				{
					COM(SELF, "'%s' deleted from '%s' at line %d\n", rp->name, basename(rp->file), rp->index);
					printf("'%s' deleted from '%s'\n", rp->name, basename(rp->file));
				}
			}
		}
	}
	free_file_list(list);
	return 0;
	
}

int user_del(record_t* rec)
{
	FILE *tfp;
	FILE *fp;
	int i = 0;
	int fd = 0;
	int bytes = 0;
	int bytes_total = 0;
	char buf[REGEX_MAX];
	char tmpfile[255];
	snprintf(tmpfile, sizeof(tmpfile), "/tmp/duser.%s.XXXXXX", basename(rec->file));
	if((fd = mkstemp(tmpfile)) < 0 || (tfp = fdopen(fd, "r+")) == NULL)
	{
		if(fd != -1)
		{
			close(fd);
			unlink(tmpfile);
		}
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, tmpfile, strerror(errno));
		exit(1);
	}
	
	if((fp = fopen(rec->file, "r")) == NULL)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, rec->file, strerror(errno));
		exit(1);
	}

	while(!feof(fp))
	{
		memset(buf, 0, sizeof(buf));
		fgets(buf, REGEX_MAX, fp);
		buf[strlen(buf) - 1] = '\0';
		if((strncmp(buf, rec->name, strlen(rec->name))) != 0 && (i != rec->index))
		{
			buf[strlen(buf)] = '\n';
			bytes = write(fd, buf, strlen(buf));
		}
		i++;
	}
	//Rewind the temp file
	lseek(fd, 0L, SEEK_SET);
	//Close the original file
	fclose(fp);
	//Truncate original and copy data from tmp to original
	if((fp = fopen(rec->file, "w+")) == NULL)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, rec->file, strerror(errno));
		exit(1);
	}

	i = 0;
	while(!feof(tfp))
	{
		memset(buf, 0, sizeof(buf));
		fgets(buf, REGEX_MAX, tfp);
		buf[strlen(buf) - 1] = '\0';
		if((strncmp(buf, rec->name, strlen(rec->name))) != 0 && (i != rec->index))
		{
			buf[strlen(buf)] = '\n';
			if(buf[0] == '\n')
				buf[0] = '\0';

			bytes = fwrite(buf, strlen(buf), 1, fp);
			bytes_total += bytes;
		}
	}

	fclose(fp);
	close(fd);
	unlink(tmpfile);
	
	if(bytes_total)
		return bytes_total;

	return 0;
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

// VERIFY that the record is proper (useful for deletion of users)
int find_in_file_ex(record_t* rec)
{
	int match = -1;
	char buf[REGEX_MAX];
	FILE *fp;
	
	if((fp = fopen(rec->file, "r")) == NULL)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, rec->file, strerror(errno));
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
	//regmatch_t pmatch[10];
	//regex_t preg;
	record_t record, *rptr;
	rptr = &record;
	rptr->index = 0;
	rptr->match = 0;
	
	int index = 0;
	
	FILE *fp;
	char *fname = strdup(filename);
	
	if((fp = fopen(fname, "r")) == NULL)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, basename(fname), strerror(errno));
		exit(1);
	}
	strncpy(rptr->file, fname, PATH_MAX);
	while(!feof(fp))
	{
		char cmp[REGEX_MAX];
		memset(cmp, 0L, REGEX_MAX);
		fgets(cmp, REGEX_MAX, fp);
		cmp[strlen(cmp)-1] = '\0';
		if((strfind(cmp, needle)) == 0)
		{
			snprintf(rptr->name, REGEX_MAX, "%s", cmp); 
			rptr->match = 1;
			rptr->index = index;
		}
		index++;
		processed.lines++;
	}

	free(fname);
	fclose(fp);
	if(rptr->match)
		return rptr;
	else
		return NULL;

}

int get_file_count(const char* path)
{
	DIR *dp = NULL;
	struct dirent *ep = NULL;
	int file_count = 0;

	if((dp = opendir(path)) == NULL)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, path, strerror(errno));
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
	DIR *dp = NULL;
	struct dirent *ep = NULL;
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
			memset(list[i], 0L, strlen(ep->d_name)+1);
			strncpy(list[i], ep->d_name, strlen(ep->d_name)+1);
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
	char** list = get_file_list(list_path, processed.files);
	if(list == NULL)
	{
		fprintf(stderr, "abort\n");
		exit(1);
	}

	printf("%20s%12s%16s\n", "List", "At line", "Match");
	printf("\t\t%s\n", "=====================================");
	int i = 0;
	for(i = 0 ; list[i] != NULL ; i++)
	{
		char tmp[PATH_MAX];
		snprintf(tmp, PATH_MAX, "%s%s", list_path, list[i]);
		record_t *rp;
		if((rp = find_in_file(tmp, needle)) != NULL)
		{
			printf("%20s\t%5d%23s\n", basename(rp->file), rp->index, rp->name);
			processed.matches++;
		}
	}
	free_file_list(list);	

	printf("\n%d matches\t%d files\t%d lines parsed\n", 
		processed.matches, processed.files, processed.lines);
	return 0;
}

int user_add(const char* filename, const char* needle)
{
	int bytes = 0;
	record_t *rp;
	char *fname = strdup(filename);
	char *newline = NULL;
	char buf[REGEX_MAX];
	FILE *fp;
	if((access(filename, W_OK|F_OK)) != 0)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, filename, strerror(errno));
		return -1;
	}
	
	if((fp = fopen(filename, "r+")) == NULL)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, filename, strerror(errno));
		return -1;
	}
	
	if((rp = find_in_file(filename, needle)) != NULL)
	{
		fprintf(stderr, "%s: '%s' already exists in '%s'\n", SELF, needle, basename(fname));
		return -1;
	}
	rewind(fp);
	//Go to end of file and find out what's where
	fseek(fp, 0L, SEEK_END);
	fgets(buf, REGEX_MAX, fp);

	//Check for a newline at the end of the file.
	//If so, add one.  If not, do not add a preceding newline.
	if((newline = strchrnul(buf, '\n')) != NULL)
	{
		snprintf(buf, REGEX_MAX, "%s\n", needle);
	}
	else
	{
		//probably very rarely used.
		snprintf(buf, REGEX_MAX, "\n%s\n", needle);
	}

	bytes = fwrite(buf, strlen(buf), 1, fp);
	//fputs(buf, fp);
	fflush(fp);
	free(fname);
	fclose(fp);
	return bytes;
}

void usage(const char* progname)
{
	printf("Domouser v0.1a - jhunk@stsci.edu\n");
	printf("Usage: %s command address [list]\n", progname);
	printf("Commands:\n");
	printf("  help		This usage statement\n");
	printf("  add		Add a user to a list\n");
	printf("  del		Delete a user from a list\n");
	printf("  delA		Delete a user from all lists\n");	
	printf("  delL		Completely delete a mailing list\n");
	printf("  mod		Modify a user in a list\n");
	printf("  list		Find and list a user in all lists\n");
	printf("  look		Find user in specific list\n");
	printf("\n");
	exit(0);
}

int user_cmd(const int argc, char* argv[])
{
	const char* cmd = argv[1];
	int i = 2;

	while(i < argc)
	{
		if(cmd)
		{
			if((strncmp(cmd, "del", strlen(cmd))) == 0)
			{
				CMD_FLAG_DEL = 1;
			}
			if((strncmp(cmd, "delA", strlen(cmd))) == 0)
			{
				CMD_FLAG_DEL_ALL = 1;
			}
			if((strncmp(cmd, "delL", strlen(cmd))) == 0)
			{
				CMD_FLAG_DEL_LIST = 1;
			}
			if((strncmp(cmd, "add", strlen(cmd))) == 0)
			{
				CMD_FLAG_ADD = 1;
			}
			if((strncmp(cmd, "mod", strlen(cmd))) == 0)
			{
				CMD_FLAG_MOD = 1;
			}
			if((strncmp(cmd, "list", strlen(cmd))) == 0)
			{
				CMD_FLAG_LIST = 1;
			}
			if((strncmp(cmd, "look", strlen(cmd))) == 0)
			{
				CMD_FLAG_LOOK = 1;
			}
			if((strncmp(cmd, "help", strlen(cmd))) == 0)
			{
				CMD_FLAG_HELP = 1;
			}
		}
		i++;
	}


	return 0;
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

	const char* progname = argv[0];
	const char* needle = argv[2];
	const char* single_list = argv[3];

	if(argc < 3)
	{
		usage(progname);
		return 0;
	}

	char filename[PATH_MAX];
	record_t *rec;
	stats_init(&processed);
	user_cmd(argc, argv);

	snprintf(filename, PATH_MAX, "%s%s", list_path, single_list);

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

	if(CMD_FLAG_HELP)
	{
		usage(progname);
	}

	return 0;
}
