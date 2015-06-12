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

struct cnfnode *parse_funcexpr(struct cnfmodule *cm, FILE *fptr);
int unparse_funcexpr(struct cnfmodule *cm, FILE *fptr, struct cnfnode *cn_root);
void register_funcexpr(struct cnfnode *opt_root);
struct cnfnode *parse_funcexpr_b(const char *buffer, const char **pp);

