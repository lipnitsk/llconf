/* -*- linux-c -*- */
/*
    This file is part of llconf2

    Copyright (C) 2004,2005  Oliver Kurth <oku@debian.org>

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

struct cnfnode{
	struct cnfnode *next;
	char *name;
	char *value;
	struct cnfnode *first_child;
	struct cnfnode *parent;
	void *user_data;
};

struct cnfnode *create_cnfnode(const char *name);

struct cnfnode *clone_cnfnode(const struct cnfnode *cn);
struct cnfnode *clone_cnftree(const struct cnfnode *cn_root);

inline
const char *cnfnode_getval(const struct cnfnode *cn);

inline
const char *cnfnode_getname(const struct cnfnode *cn);

void cnfnode_setval(struct cnfnode *cn, const char *value);
void cnfnode_setname(struct cnfnode *cn, const char *name);

/* free memory of node, leaving children intact */
void destroy_cnfnode(struct cnfnode *cn);

/* free recursively all children, then cn */
void destroy_cnftree(struct cnfnode *cn);

/* append a node */
void append_node(struct cnfnode *cn_parent, struct cnfnode *cn);

/* insert a node */
void insert_node_before(struct cnfnode *cn_before, struct cnfnode *cn);

/* remove a node from tree list (but do not destroy it) */
void unlink_node(struct cnfnode *cn);

/* walks through list starting with cnf_list,
   and returns 1st node with matching name */
struct cnfnode *find_node(struct cnfnode *cn_list, const char *name);

int compare_cnftree(const struct cnfnode *cn_root1, const struct cnfnode *cn_root2);
int compare_cnfnode(const struct cnfnode *cn1, const struct cnfnode *cn2);

void dump_nodes(struct cnfnode *cn_root, int level);
