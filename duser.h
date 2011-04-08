#ifndef DUSER_H
#define DUSER_H

#include <limits.h>
#define REGEX_MAX   255
#define CFG_PATH "/usr/local/etc/duser"
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

int cfg_open(const char* filename);
void cfg_close();
int cfg_get_key(char* val, const char* key);
int logcleanup();
int COM(const char* func, char *format, ...);
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
int user_add(const char* filename, const char* needle);
int user_new_list(const char* fname);
#ifdef _NLINUX_
char *strcasestr(const char *s, const char *find);
char *strchrnul(const char* s, int c);
#endif

#endif
