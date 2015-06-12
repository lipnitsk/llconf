/* -*- linux-c -*- */
/*
    This file is part of llconf2

    Copyright (C) 2004, 2005  Oliver Kurth <oku@debian.org>

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

struct cnfresult{
	struct cnfresult *next;
	char *path;
	struct cnfnode *cnfnode;
};

/* internal use: */
struct cnfresult *create_cnfresult(struct cnfnode *cn, const char *path);
void destroy_cnfresult(struct cnfresult *cr);
void destroy_cnfresult_list(struct cnfresult *cr_list);
void append_result(struct cnfresult *cr_prev, struct cnfresult *cr);

/* public use: */
struct cnfresult *cnf_find_entry(struct cnfnode *cn_root, const char *path);
struct cnfnode *cnf_add_branch(struct cnfnode *cn_root, const char *path, int do_merge);
int cnf_del_branch(struct cnfnode *cn_root, const char *path, int del_empty);
int cnf_set_entry(struct cnfnode *cn_root, const char *path, const char *val, int do_create);
const char *cnf_get_entry(struct cnfnode *cn_root, const char *path);
struct cnfnode *cnf_get_node(struct cnfnode *cn_root, const char *path);

void strip_cnftree(struct cnfnode *cn_root);
