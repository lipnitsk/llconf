/* -*- linux-c -*- */
/*
    This file is part of llconf2

    Copyright (C) 2005  Oliver Kurth <oku@debian.org>

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "strutils.h"
#include "nodes.h"
#include "lines.h"
#include "modules.h"

#include "funcexpr.h"

static
struct cnfmodule this_module = {
	name: "funcexpr",
	parser: parse_funcexpr,
	unparser: unparse_funcexpr
};

struct cnfnode *parse_funcexpr_b(const char *buffer, const char **pp)
{
	struct cnfnode *cn_top = NULL, *cn;
	char buf[1024], *q;
	const char *p;

	p = buffer;
	while(*p && isspace(*p)) p++;

	q = buf;
	if(*p == '\"' || *p == '\''){
		char qchar = *p;

		*q++ = *p++;
		while(*p && *p != qchar && q < buf+1021)
			*q++ = *p++;
		*q++ = *p++;
	}else{
		while(*p && (isalnum(*p) || *p == '$' || *p == '_' || *p == '-') && q < buf+1023)
			*q++ = *p++;
	}
	*q = 0;

	cn_top = create_cnfnode(buf);
	while(*p && isspace(*p)) p++;
	if(*p == '('){
		p++;
		while(1){
			while(*p && isspace(*p)) p++;
			if(*p){
				cn = parse_funcexpr_b(p, &p);
				if(cn)
					append_node(cn_top, cn);
				else
					break;
				while(*p && isspace(*p)) p++;
				if(*p != ',')
					break;
				else
					p++;
			}else
				break;
		}
		if(*p == ')') p++;
	}

	*pp = p;
	return cn_top;
}

struct cnfnode *parse_funcexpr(struct cnfmodule *cm, FILE *fptr)
{
	struct cnfnode *cn_root, *cn_top;
	struct confline *cl_root, *cl;
	const char *p;
	char *buffer, *q;
	int buf_size = 0;

	cl_root = read_conflines(fptr);
	
	for(cl = cl_root; cl; cl = cl->next){
		buf_size += strlen(cl->line);
	}

	buffer = malloc(buf_size+1);

	q = buffer;
	for(cl = cl_root; cl; cl = cl->next){
		p = cl->line;
		while(*p) *q++ = *p++;
	}
	*q = 0;

	cn_root = create_cnfnode("(root)");

	p = buffer;
	while(*p){
		cn_top = parse_funcexpr_b(p, &p);
		if(cn_top)
			append_node(cn_root, cn_top);
		else
			break;
		while(*p && isspace(*p)) p++;
	}

	free(buffer);

	destroy_confline_list(cl_root);
	return cn_root;
}

int _unparse_funcexpr(FILE *fptr, struct cnfnode *cn_top)
{
	struct cnfnode *cn;

	fprintf(fptr, "%s", cn_top->name);
	if(cn_top->first_child){
		fprintf(fptr, "(");
		for(cn = cn_top->first_child; cn; cn = cn->next){
			_unparse_funcexpr(fptr, cn);
			if(cn->next)
				fprintf(fptr, ", ");
		}
		fprintf(fptr, ")");
	}
	return 0;
}
	
int unparse_funcexpr(struct cnfmodule *cm, FILE *fptr, struct cnfnode *cn_root)
{
	struct cnfnode *cn_top;

	for(cn_top = cn_root->first_child; cn_top; cn_top = cn_top->next){
		_unparse_funcexpr(fptr, cn_top);
		fprintf(fptr, "\n");
	}
	return 0;
}

void register_funcexpr(struct cnfnode *opt_root)
{
	register_cnfmodule(&this_module, opt_root);
}
