#ifndef DUSER_H
#define DUSER_H
#include <limits.h>

#define REGEX_MAX   60
#define CMD_FLAG_NOOPT 0x00
#define CMD_FLAG_DEL 0x02
#define CMD_FLAG_MOD 0x04
#define CMD_FLAG_ADD 0x08
#define CMD_FLAG_LIST 0x16
#define CMD_FLAG_HELP 0x32
#define CMD_FLAG_LOOK 0x64
#define CMD_FLAG_NULL 0x254

#define regex_fmt "\%s$"
#define LOGFILE "/var/log/duser"
#define list_path	"/internal/1/domotest/opt/majordomo/majordomo-1.94.3/lists/"

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


char* basename(const char* path);
record_t* find_in_file(const char* filename, const char* needle);
int get_file_count(const char* path);
char** get_file_list(const char* path, int count);
void stats_init(stats_t *s);
int user_list(const char* needle);
int find_in_file_ex(record_t* rec);
int user_del(record_t* rec);
int user_cmd(const char* arg);
int user_choice(char c);

#endif
