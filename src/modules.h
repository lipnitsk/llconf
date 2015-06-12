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

extern struct cnf_module *cnf_modules;

struct cnfmodule
{
  struct cnfmodule *next;
  char *name;
  char *default_file;
  struct cnfnode *(*parser)(struct cnfmodule *cm, FILE *fptr);
  int (*unparser)(struct cnfmodule *cm, FILE *fptr, struct cnfnode *cn_root);
  struct cnfnode *opt_root;
};

void register_cnfmodule(struct cnfmodule *cm, struct cnfnode *opt_root);

void unregister_all(void);

struct cnfmodule *find_cnfmodule(const char *name);

void cnfmodule_setopts(struct cnfmodule *cm, struct cnfnode *opt_root);

struct cnfnode *parse_options(const char *string);

struct cnfnode *cnfmodule_parse(struct cnfmodule *cm, FILE *fin);

struct cnfnode *cnfmodule_parse_file(struct cnfmodule *cm, const char *fname);

void destroy_cnfmodule(struct cnfmodule *cm);

struct cnfmodule *clone_cnfmodule(struct cnfmodule *cm,
				  const char *new_name,
				  const char *default_file,
				  struct cnfnode *opt_root);

int cnfmodule_unparse(struct cnfmodule *cm, FILE* fout,
				  struct cnfnode *cn_root);

int cnfmodule_unparse_file(struct cnfmodule *cm, const char *fname,
			   struct cnfnode *cn_root);
