/* -*- linux-c -*- */
/*
    This file is part of llconf2

    Copyright (C) 2004  Oliver Kurth <oku@debian.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define MAX_CONFLINE 1024

struct confline
{
	struct confline *next;
	char *line;
};

struct confline *create_confline(const char *line);
void destroy_confline(struct confline *cl);
void destroy_confline_list(struct confline *cl_list);
struct confline *read_conflines(FILE *fptr);
struct confline *append_confline(struct confline *cl_list, struct confline *cl);

