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

#include <string.h>
#include "strchrnul.h"

/*
 * Locate a char in a string
 *
 * The strchr() function returns a pointer to the first occurrence 
 * of the character c in the string s. 
 *
 * The strrchr() function returns a pointer to the last occurrence 
 * of the character c in the string s. 
 *
 * The strchrnul() function is like strchr() except that 
 * if c is not found in s, then it returns a pointer to the 
 * null byte at the end of s, rather than NULL. 
 */
char *
strchrnul (const char *s, int c_in)
{
  char c = c_in;
  while (*s && (*s != c))
    s++;

  return (char *) s;
}
