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

/** @file entry.c High level functions to access a cnfnode tree.
 * Functions here provide a higher level access to the cnfnode tree
 * than the cnfnode_* functions from node.c. They usually handle with
 * path names and values, rather than pointers to the nodes.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <errno.h>

#include "strutils.h"
#include "nodes.h"
#include "entry.h"

/** Create a cnfresult structure.
 * @param cn the cnfnode
 * @param path the path
 * @return the pointer to the newly allocated cnfresult
 */
struct cnfresult *create_cnfresult(struct cnfnode *cn, const char *path)
{
	struct cnfresult *cr = NULL;

	cr = (struct cnfresult *)malloc(sizeof(struct cnfresult));
	if(cr){
		memset(cr, 0, sizeof(struct cnfresult));
    
		cr->cnfnode = cn;
		if(path)
			cr->path = strdup(path);
	}
	return cr;
}

/** Free a cnfresult structure.
 * @param cr pointer to the cnfresult structure
 */
void destroy_cnfresult(struct cnfresult *cr)
{
	if(cr){
		if(cr->path) free(cr->path);
		free(cr);
	}
}

/** Free a cnfresult structure list by calling .destroy_cnfresult() for all list members
 * @param cr_list pointer to the cnfresult list
 */
void destroy_cnfresult_list(struct cnfresult *cr_list)
{
	struct cnfresult *cr, *cr_next;

	for(cr = cr_list; cr; cr = cr_next){
		cr_next = cr->next;
		destroy_cnfresult(cr);
	}
}

/** Append a cnfresult to the list.
 * @param cr_first the first entry in the list
 * @param cr the entry to be appended
 */
void append_cnfresult(struct cnfresult *cr_first, struct cnfresult *cr)
{
	struct cnfresult **crp;

	for(crp = &(cr_first->next); *crp; crp = &((*crp)->next));
	*crp = cr;
}

#define FIND_ENTRY_FLAG_NOPATH 0x01
#define FIND_ENTRY_FLAG_FIRST 0x02

static
void _cnf_find_entry(struct cnfresult **pcr, struct cnfnode *cn_parent,
		     char *fullpath, const char *path, int flags)
{
	struct cnfresult *cr = NULL;
	struct cnfnode *cn_list;
	struct cnfnode *cn;
	char dname[256], *value = NULL, *q;
	const char *p, *p0;
	int index = -1, i = 0, j = 0;

	p = path;
	cn_list = cn_parent->first_child;

	q = dname;
	p0 = p;
	while(*p && (*p != '/' || (p > p0 && p[-1] == '\\')) &&
	      (*p != '[') && (q < dname+255)){
		if(*p == '\\')
			p++;
		else{
			if(*p == '='){
				*q++ = 0; p++;
				value = q;
			}else
				*(q++) = *(p++);
		}
	}
	*q = 0;

	if(*p == '['){
		char tmp[4];

		p++;
		q = tmp;
		while(*p && isdigit(*p) && (q < tmp+3))
			*(q++) = *(p++);
		*q = 0;

		if(*p != ']')
			return; // FIXME: error condition

		index = atoi(tmp);
		p++;
	}

	/* we use two different indices here -
	   i indexes paths that include a value, and count only matching paths
	   j indexes paths as if there were no value specified.
	   => i == j iff the path does not have a value specified.
	*/

	if((*p == '/') && p[1]){
		p++;
		for(cn = cn_list; cn; cn = cn->next){
			if(strcmp(cn->name, dname) == 0){
				if((value == NULL) || (strcmp(cn->value, value) == 0)){
					if(index == -1 || i == index){
						char tmp[256];
						
						if(fullpath)
							snprintf(tmp, 255, "%s%s[%d]/",
								 fullpath, dname, j);
						
						_cnf_find_entry(pcr, cn, fullpath ? tmp : NULL, p, flags);
						
						if(index != -1)
							break;
					}
					i++;
				}
				j++;
			}
		}
	}else{
		for(cn = cn_list; cn; cn = cn->next){
			if(strcmp(cn->name, dname) == 0){
				if((value == NULL) || (strcmp(cn->value, value) == 0)){
					if(index == -1 || i == index){
						char tmp[256];
						
						if(fullpath)
							snprintf(tmp, 255, "%s%s[%d]", fullpath, dname, j);
						
						cr = create_cnfresult(cn, fullpath ? tmp : NULL);
						if(*pcr)
							append_cnfresult(*pcr, cr);
						else
							*pcr = cr;
						
						if(flags & FIND_ENTRY_FLAG_FIRST)
							return;
						
						if(index != -1)
							break;
					}
					i++;
				}
				j++;
			}
		}
	}
}
      
static
struct cnfresult *cnf_find_entry_f(struct cnfnode *cn_root, const char *path, int flags)
{
	struct cnfresult *cnf_res = NULL;
	char fullpath[256];

	fullpath[0] = 0;

	if(strcmp(path, ".") == 0)
		cnf_res = create_cnfresult(cn_root, path);
	else
		_cnf_find_entry(&cnf_res, cn_root, flags & FIND_ENTRY_FLAG_NOPATH ? NULL : fullpath, path, flags);

	return cnf_res;
}

/** Find all entries in a tree.
 * Search in the cnfnode tree for matching entries
 * @param cn_root root of the cnfnode tree
 * @param path the patch to be searched for, eg. iface/eth0/address.
 * Can also be indexed, like iface/eth1/up[1], and/or a value of the node can be specified like iface/eth1/address=10.0.0.1 .
 * @return pointer to a list of matching results.
 */
struct cnfresult *cnf_find_entry(struct cnfnode *cn_root, const char *path)
{
	return cnf_find_entry_f(cn_root, path, 0);
}

/** Add a branch to a tree.
 * Add a branch to an existing tree of nodes. If part of the tree already exists,
 * and do_merge==1, the remainder will be created. For example, if path is
 * "iface/eth0/address", and "iface/eth0" already exists and do_merge==1, just "address"
 * will be appended to "iface/eth0". If do_merge==0, a whole new subtree, starting with "iface"
 * will be appended.
 * @param cn_root pointer to the root of the tree
 * @param path the path to append to
 * @param do_merge flag as described above
 * @return pointer to the last newly created node
 */
struct cnfnode *cnf_add_branch(struct cnfnode *cn_root, const char *path, int do_merge)
{
	struct cnfnode *cn = NULL, *cn_parent;
	const char *p;
  
	cn_parent = cn_root;
	p = path;
	while(*p){
		char dname[256], *q;
		const char *p0;
#if 0		
		q = dname;
		while(*p && (*p != '/') && (*p != '=') && (q < dname+255)) *q++ = *p++;
		*q = 0;
#else
		q = dname;
		p0 = p;
		while(*p && ((*p != '/' && *p != '=') || (p > p0 && p[-1] == '\\')) &&
		      (q < dname+255)){
			if(*p == '\\')
				p++;
			else
				*(q++) = *(p++);
		}
		*q = 0;
#endif
		cn = NULL;
		if(do_merge){
			for(cn = cn_parent->first_child; cn; cn = cn->next){
				if(strcmp(cn->name, dname) == 0){
					break;
				}
			}
		}
		if(cn == NULL){
			append_node(cn_parent, cn = create_cnfnode(dname));
		}
		cn_parent = cn;

		if(*p == '='){
			char buf[256];
			p++;
			if(*p)
				cnfnode_setval(cn, dup_next_line_b(&p, buf, 255));
			else
				cnfnode_setval(cn, "");
			break;
		}else if(*p == '/'){
			p++;
		}else break;
	}
	return cn;
}

/** Delete a branch.
 * Deletes a subtree pointed to by path.  If more than one matching entry
 * exists, the first one will be deleted. If del_empty==1, and by removing
 * the parent tree becomes empty, the parents will be removed recursively.
 * If del_empty==0, an empty subtree may be left behind.
 * In any case, memory of the subtree and deleted parents will be freed.
 * @param cn_root pointer to the root of the tree
 * @param path the path of the tree to be deleted
 * @param del_empty set to 1 if empty parents shall be removed.
 */
int cnf_del_branch(struct cnfnode *cn_root, const char *path, int del_empty)
{
	struct cnfresult *cr;

	cr = cnf_find_entry_f(cn_root, path, FIND_ENTRY_FLAG_NOPATH|FIND_ENTRY_FLAG_FIRST);
	if(cr){
		struct cnfnode *cn_top = cr->cnfnode;

		unlink_node(cn_top);

		if(del_empty){
			/* must be done after unlink_(), so it does not find this one in the search
			   below, and before destroy_(), because we still need to access the structure. */
			struct cnfnode *cn;

			for(cn = cn_top->parent; cn->parent && cn != cn_root; cn = cn->parent){
				if(cn->first_child == NULL){
					unlink_node(cn);
					destroy_cnftree(cn);
				}
			}
		}
		destroy_cnftree(cn_top);

		destroy_cnfresult_list(cr);
		return 0;
	}

	errno = ENOENT;
	return -ENOENT;
}

/** Set an entry.
 * Set the value of an entry pointed to by path.  If more than one matching entry
 * exists, the first one will be returned. If do_create==1, a new node
 * (or subtree) will be created, if the entry does not exist. if do_create==0,
 * -1 will be returned, and errno will be set to ENOENT.
 * @param cn_root pointer to the root of the tree
 * @param path the path of the entry
 * @param val the (new) value of the entry
 * @param do_create flag as described above
 * @return 0 on success, and -1 on error.
 */
int cnf_set_entry(struct cnfnode *cn_root, const char *path, const char *val, int do_create)
{
	struct cnfresult *cr;
	struct cnfnode *cn;

	cr = cnf_find_entry_f(cn_root, path, FIND_ENTRY_FLAG_NOPATH|FIND_ENTRY_FLAG_FIRST);
	if(cr){
		cn = cr->cnfnode;
		destroy_cnfresult_list(cr);
	}else if(do_create){
		cn = cnf_add_branch(cn_root, path, 1);
	}else{
		errno = ENOENT;
		return -1;
	}
	if(cn){
		cnfnode_setval(cn, val);
		return 0;
	}
	return -1;
}

/** Get an entry.
 * Get the value of an entry pointed to by path.
 * @param cn_root pointer to the root of the tree
 * @param path the path of the entry
 * @return 0 on success, and -1 on error.
 * @sa cnf_find_entry
 */
const char *cnf_get_entry(struct cnfnode *cn_root, const char *path)
{
	struct cnfresult *cr;
	struct cnfnode *cn;

	cr = cnf_find_entry_f(cn_root, path, FIND_ENTRY_FLAG_NOPATH|FIND_ENTRY_FLAG_FIRST);
	if(cr){
		cn = cr->cnfnode;
		destroy_cnfresult_list(cr);
	}else{
		errno = ENOENT;
		return NULL;
	}

	if(cn)
		return cn->value;

	return NULL;
}

/** Get a node.
 * Get the node of an entry pointed to by path. If more than one matching entry
 * exists, the first one will be returned.
 * @param cn_root pointer to the root of the tree
 * @param path the path of the entry
 * @return the pointer to the node on success, and NULL if the node is not found.
 * @sa cnf_find_entry
 */
struct cnfnode *cnf_get_node(struct cnfnode *cn_root, const char *path)
{
	struct cnfresult *cr;
	struct cnfnode *cn;

	cr = cnf_find_entry_f(cn_root, path, FIND_ENTRY_FLAG_NOPATH|FIND_ENTRY_FLAG_FIRST);
	if(cr){
		cn = cr->cnfnode;
		destroy_cnfresult_list(cr);
	}else{
		errno = ENOENT;
		return NULL;
	}

	return cn;
}

/** Strip comments and empty lines
 * Strip comments and empty lines (specific: all nodes with names starting with a dot) from the tree.
 * @param cn_root pointer to the root of the tree
 */

void strip_cnftree(struct cnfnode *cn_root)
{
	struct cnfnode *cn, *cn_next;

	for(cn = cn_root->first_child; cn; cn = cn_next){
		cn_next = cn->next;

		strip_cnftree(cn);

		if(cn->name[0] == '.'){
			unlink_node(cn);
			destroy_cnftree(cn);
		}
	}
}

