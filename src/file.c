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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "strutils.h"
#include "nodes.h"
#include "lines.h"
#include "modules.h"
#include "file.h"

static
struct cnfmodule this_module = {
	name: "file",
	default_file: NULL,
	parser: parse_file,
	unparser: unparse_file
};

struct cnfnode *parse_file(struct cnfmodule *cm, FILE *fptr)
{
	struct cnfnode *cn_root;
	char buf[4096];
	int n;

	cn_root = create_cnfnode("(root)");
	if(cn_root){
		n = fread(buf, 1, 4095, fptr);
		buf[n] = 0;
		cnfnode_setval(cn_root, buf);
	}

	return cn_root;
}

int unparse_file(struct cnfmodule *cm, FILE *fptr, struct cnfnode *cn_root)
{
	fprintf(fptr, "%s", cn_root->value ? cn_root->value : "");

	return 0;
}

void register_file(struct cnfnode *opt_root)
{
	register_cnfmodule(&this_module, opt_root);
}
	
