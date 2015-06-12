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
#include <unistd.h>
#include <errno.h>

#include "strutils.h"
#include "nodes.h"
#include "lines.h"
#include "entry.h"
#include "modules.h"

#include "allmodules.h"

FILE *open_file_or_exit(const char *fname, const char *mode)
{
	FILE *fptr;

	if(strcmp(fname, "-") == 0){
		if(mode[0] == 'r')
			fptr = stdin;
		else
			fptr = stdout;
	}else{
		fptr = fopen(fname, mode);
		if(!fptr){
			fprintf(stderr, "could not open '%s': %s\n", fname, strerror(errno));
			exit(1);
		}
	}
	return fptr;
}

FILE *open_tmpfile_or_exit(const char *fname)
{
	if(strcmp(fname, "-") == 0){
		return stdout;
	}else{
		char tmpname[256];
		snprintf(tmpname, 256, "%s.tmp", fname);
		return open_file_or_exit(tmpname, "w");
	}
}

void move_tmpfile_or_exit(const char *fname)
{
	if(strcmp(fname, "-") == 0)
		return;
	else{
		char tmpname[256];
		snprintf(tmpname, 256, "%s.tmp", fname);
		if(rename(tmpname, fname) != 0){
			fprintf(stderr, "could not move '%s' to '%s' : %s\n",
				tmpname, fname, strerror(errno));
			exit(1);
		}
	}
}


int main(int argc, char *argv[])
{
	char *fname_in = NULL, *fname_out = NULL;
	char *modname = NULL;
	struct cnfnode *opt_root = NULL;
	int exit_code = 0;
	int do_strip = 0;

	if(argc > 1){
		modname = argv[1];
		argc--;
		argv++;
	}else
		exit(1);

	while(1){
		int c;

		c = getopt(argc, argv, "i:o:f:sO:");

		if (c == -1)
			break;

		switch(c){
		case 'i':
			fname_in = optarg;
			break;
		case 'o':
			fname_out = optarg;
			break;
		case 'f':
			fname_out = fname_in = optarg;
			break;
		case 's':
			do_strip = 1;
			break;
		case 'O':
			opt_root = parse_options(optarg);
			break;
		case '?':
			break;
		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind < argc) {
		FILE *fin, *fout;
		struct cnfnode *cn_root;
		struct cnfresult *cnf_res = NULL, *cr;
		char *action = argv[optind];
		struct cnfmodule *module;
    
		register_all();

		module = find_cnfmodule(modname);
		if(!module){
			fprintf(stderr, "no module with name '%s' found.\n", modname);
			exit(1);
		}
      
		cnfmodule_setopts(module, opt_root);

		if(!fname_in)
			fname_in = module->default_file;

		if(!fname_out)
			fname_out = module->default_file;

		if(fname_in){
			fin = open_file_or_exit(fname_in, "r");
		}else{
			fprintf(stderr, "no input filename given, and the module has no default.\n");
			exit(1);
		}

		cn_root = module->parser(module, fin);
    
		if(!cn_root){
			fprintf(stderr, "parser failed\n");
			exit(1);
		}

		if(do_strip)
			strip_cnftree(cn_root);

		if(fin != stdin) fclose(fin);

		if(strcmp(action, "dump") == 0){

			dump_nodes(cn_root, 0);

		}else if(strcmp(action, "get") == 0){

			cnf_res = cnf_find_entry(cn_root, argv[optind+1]);

			if(cnf_res){
				for(cr = cnf_res; cr; cr = cr->next){
					printf("%s\n", cr->cnfnode->value ? cr->cnfnode->value : "");
				}
			}else{
				exit_code = 1;
			}

		}else if(strcmp(action, "exists") == 0){

			cnf_res = cnf_find_entry(cn_root, argv[optind+1]);

			if(!cnf_res){
				exit_code = 1;
			}

		}else if((strcmp(action, "set") == 0) || (strcmp(action, "unparse") == 0)){
			int result;

			if(!fname_out){
				fprintf(stderr, "no output filename given, and the module has no default.\n");
				exit(1);
			}

			if(strcmp(action, "set") == 0){
				cnf_res = cnf_find_entry(cn_root, argv[optind+1]);
				if(cnf_res){
					/* we set the _first_ found result */
					cnfnode_setval(cnf_res->cnfnode, argv[optind+2]);
				}else{
					fprintf(stderr, "%s not found\n", argv[optind+1]);
					exit_code = 1;
				}
			}

			fout = open_tmpfile_or_exit(fname_out);

			result = module->unparser(module, fout, cn_root);
			if(fout != stdout) fclose(fout);

			move_tmpfile_or_exit(fname_out);

		}else if((strcmp(action, "add") == 0) || (strcmp(action, "addne") == 0) || 
			 (strcmp(action, "del") == 0) || (strcmp(action, "rem") == 0)){
			const char *path = argv[optind+1], *value = argv[optind+2];
			int result;

			if(!fname_out){
				fprintf(stderr, "no output filename given, and the module has no default.\n");
				exit(1);
			}

			if(strcmp(action, "add") == 0){
				struct cnfnode *cn_path;
				
				cn_path = cnf_get_node(cn_root, path);
				if(cn_path){
					cnf_add_branch(cn_path, value, 0);
				}else{
					fprintf(stderr, "%s not found\n", path);
					exit_code = 1;
				}
			}else if(strcmp(action, "addne") == 0){
				struct cnfnode *cn_path;
				
				cn_path = cnf_get_node(cn_root, path);
				if(cn_path){
					if(!cnf_get_node(cn_path, value))
						cnf_add_branch(cn_path, value, 0);
				}else{
					fprintf(stderr, "%s not found\n", path);
					exit_code = 1;
				}
			}else
				cnf_del_branch(cn_root, argv[optind+1], strcmp(action, "del") == 0);

			fout = open_tmpfile_or_exit(fname_out);

			result = module->unparser(module, fout, cn_root);
			if(fout != stdout) fclose(fout);

			move_tmpfile_or_exit(fname_out);

		}else if(strcmp(action, "list") == 0){

			if(argc - optind > 1)
				cnf_res = cnf_find_entry(cn_root, argv[optind+1]);
			else
				cnf_res = cnf_find_entry(cn_root, ".");

			for(cr = cnf_res; cr; cr = cr->next){
				struct cnfnode *cn = cr->cnfnode;
				for(cn = cn->first_child;cn; cn = cn->next)
					printf("%s\n", cn->name);
			}
		}
	}

	exit(exit_code);
}
