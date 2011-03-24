#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <tre/regex.h>
#define REGEX_MAX	60


typedef struct record_t
{
	int index;
	int match;
	char file[PATH_MAX];
	char name[REGEX_MAX];
} record_t;

char* basename(const char* path);
record_t* find_in_file(const char* filename, const char* needle);
int get_file_count(const char* path);
char** get_file_list(const char* path, int count);

char* basename(const char* path)
{
	char *ptr = strrchr(path, '/');
	return ptr ? ptr + 1 : (char*)path;
}

const char* regex_fmt = "\%s$";
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
	strncpy(rptr->name, needle, strlen(needle));
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

	char **list = (char**)calloc(count, sizeof(char*));
	if(list == NULL)
	{
		perror("calloc");
		exit(1);
	}
	while((ep = readdir(dp)))
	{
		if(ep->d_type == DT_REG && !strstr(ep->d_name, "."))
		{
			list[i] = (char*)malloc(sizeof(char) * strlen(ep->d_name));
			memset(list[i], 0L, sizeof(ep->d_name));
			strncpy(list[i], ep->d_name, strlen(ep->d_name));
			i++;
		}
	}
	closedir(dp);

	return list;
}
static int file_count = 0;
const char* list_path = "/internal/1/domotest/opt/majordomo/majordomo-1.94.3/lists/";

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Please define an email address to search for\n");
		exit(1);
	}
	file_count = get_file_count(list_path);
	char** list;
	list = get_file_list(list_path, file_count);
	int i = 0;
	for(i = 0 ; list[i] != NULL ; i++)
	{
		char tmp[PATH_MAX];
		sprintf(tmp, "%s%s", list_path, list[i]);
		record_t *rp;
		if((rp = find_in_file(tmp, argv[1])) != NULL)
		{
			printf("%s (Index: %d)\n", basename(rp->file), rp->index);
		}
	}
	printf("%d files\n", file_count);

	return 0;
}
