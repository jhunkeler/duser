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

#ifndef COMPAT_H
#define COMPAT_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "compat.h"

char *strchrnul(const char* s, int c);
char *strcasestr(const char *s, const char *find);

#endif

