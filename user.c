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
#ifdef _NLINUX_
#	define HAVE_STRCHRNUL
#	define HAVE_STRCASESTR
#endif
#include "duser.h"

extern char list_path[PATH_MAX];
extern char logfile[PATH_MAX];
static char progname[FILENAME_MAX];
extern int CMD_FLAG_NOOPT;
extern int CMD_FLAG_DEL;
extern int CMD_FLAG_DEL_ALL;
extern int CMD_FLAG_DEL_LIST;
extern int CMD_FLAG_MOD;
extern int CMD_FLAG_ADD;
extern int CMD_FLAG_LIST;
extern int CMD_FLAG_HELP;
extern int CMD_FLAG_LOOK;
extern int CMD_FLAG_NEW;
extern int CMD_FLAG_NULL;


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
			printf("%20s\t%5d%23s\n", basename(rp->file), rp->index, rp->name);
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
#ifdef _NLINUX_
		char path[PATH_MAX];
		struct stat st;
		snprintf(path, PATH_MAX, "%s%s", list_path, ep->d_name);
		if((stat(path, &st)) == 0)
		{
			if(S_ISREG(st.st_mode))
			{
				if(!strstr(ep->d_name, "."))
				{
					file_count++;
				}
			}
		}
#else
		if(ep->d_type == DT_REG && !strstr(ep->d_name, "."))
		{
			file_count++;
		}
#endif
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
#ifdef _NLINUX_
		char path[PATH_MAX];
		struct stat st;
		snprintf(path, PATH_MAX, "%s%s", list_path, ep->d_name);
		if((stat(path, &st)) == 0)
		{
			if(S_ISREG(st.st_mode))
			{
				if(!strstr(ep->d_name, "."))
				{
					list[i] = (char*)malloc(sizeof(char) * strlen(ep->d_name)+1);
					memset(list[i], 0L, strlen(ep->d_name)+1);
					strncpy(list[i], ep->d_name, strlen(ep->d_name)+1);
					i++;
				}
			}
		}
#else
		if(ep->d_type == DT_REG && !strstr(ep->d_name, "."))
		{
			list[i] = (char*)malloc(sizeof(char) * strlen(ep->d_name)+1);
			memset(list[i], 0L, strlen(ep->d_name)+1);
			strncpy(list[i], ep->d_name, strlen(ep->d_name)+1);
			i++;
		}
#endif
	}
	closedir(dp);
	return list;
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

int user_cmd(const int argc, char* argv[])
{
	const char* cmd = argv[1];

	if(argc < 2)
		return CMD_FLAG_NULL;
	
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
		if((strncmp(cmd, "new", strlen(cmd))) == 0)
		{
			CMD_FLAG_NEW = 1;
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

	return 0;
}


int user_new_list(const char* fname)
{
	char *filename = strdup(fname);
	char *timestr;
	char message[BUFSIZ];
	FILE *fp = NULL;
	time_t ttm;
	struct tm *tmptr;

	if((access(filename, F_OK)) == 0)
	{
		fprintf(stderr, "%s: %s: File already exists\n", SELF, basename(filename));
		return -1;
	}
	
	if((fp = fopen(filename, "w+")) == NULL)
	{
		fprintf(stderr, "FATAL: %s: %s: %s\n", SELF, basename(filename), strerror(errno));
		return -1;
	}
	
	time(&ttm);
	tmptr = localtime(&ttm);
	timestr = asctime(tmptr);
	timestr[strlen(timestr)-1] = '\0';
	snprintf(message, BUFSIZ, "#\n# Created by %s on %s. UID %d\n#\n", basename(progname), timestr, getuid());	
	fputs(message, fp);
	fflush(fp);
	fclose(fp);

	free(filename);
	return 0;
}


