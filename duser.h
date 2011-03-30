#ifndef DUSER_H
#define DUSER_H
#include <limits.h>

#define REGEX_MAX   255
#define regex_fmt "\%s$"
//#define regex_fmt "m/%s$/"
#define list_path	"/internal/1/domotest/opt/majordomo/majordomo-1.94.3/lists/"
#define LOGFILE "/var/log/duser"
#define SELF	__FUNCTION__
typedef struct stats_t
{
    int lines;
    int files;
    int matches;
    int added;
    int deleted;
    int modified;
} stats_t;
//Global statistics struct
stats_t processed;

typedef struct record_t
{
    int index;
    int match;
    int pad1;
    char name[REGEX_MAX];
    int pad2;
    char file[PATH_MAX];
    int pad3;
} record_t;

int logcleanup();
int COM(const char* func, char *format, ...);
char* basename(const char* path);
record_t* find_in_file(const char* filename, const char* needle);
int get_file_count(const char* path);
char** get_file_list(const char* path, int count);
void free_file_list(char** list);
void stats_init(stats_t *s);
int user_list(const char* needle);
int find_in_file_ex(record_t* rec);
int user_del(record_t* rec);
int user_cmd(const int argc, char* argv[]);
int user_choice(char c);

#endif
