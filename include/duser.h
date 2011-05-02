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

#ifndef DUSER_H
#define DUSER_H

#include <limits.h>
#define REGEX_MAX   255
#define CFG_PATH "/usr/local/etc/duser"
#define SELF	__FUNCTION__
#ifndef _NLINUX_
#	define FMTLIST "%20s\t%5d%23s\n"
#else
#	define FMTLIST "%20s\t%5d%16s\n"
#endif

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

void usage();
int strval(const char* str);
int strfind(const char* str1, const char* str2);
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
int user_del_all(const char* needle);
int user_del_list(const char* filename);
int user_cmd(const int argc, char* argv[]);
int user_choice(char c);
int user_add(const char* filename, const char* needle);
int user_new_list(const char* fname);
#ifdef _NLINUX_
char *strcasestr(const char *s, const char *find);
char *strchrnul(const char* s, int c);
#endif

#endif
