# Overview #

The problem: every program for Unix and Linux has it own configuration
file syntax. This makes it difficult to write configuration editor
frontends. Also, if a program is being updated, it may bring new
option with it, which are difficult to get into already existing,
customized configuration files.

llconf (loss less configuration) is meant as a middle ware to unify
control over configuration files. I tries to parse different
configuration files using different modules, and rewrite them after
applying changes, without destroying user changes and comments, so
that it is still possible to edit the files with a common text editor
like vi or emacs.

llconf is _not_ meant as a meta configuration or registry, although
the access may look similar as access to a registry, and you can use
it to do such a thing. All information is still stored in the original
config files.

The idea behind it the realization that every config file can be
represented in a tree, with the values as its leaves.

## Introduction ##

llconf (Loss Less Configuration) is a utility and a C library to parse and unparse configuration files.
llconf can parse any configuration file, provided it has a parser module.
llconf parses the configuration file into a tree, and operations can then be made on that tree, like changing values, adding or removing branches.

The idea assumes that a configuration always has a structure, which can be represented in a tree. This tree can be very simple, with only one level, or more complex.

Also, many simple configuration files are very similar. For example /etc/passwd is a simple table, separated by colons. /etc/fstab is a table separated by spaces. llconf uses the same parser for both.

A more complex example is /etc/network/interfaces.

## Installation ##

After download and unpacking, cd to the source directory, and do:

```
 # ./configure && make
 # su
 # make install
```

This will install llconf, libraries and header files into /usr/local/

Debian users will prefer to make a package first:

```
 # dpkg-buildpackage -rfakeroot -us -uc
```

and then install the packages.

## Parsers ##

So far, llconf implements these parsers: file, shell, table, ifupdown, pair, ppp, ini, route, pslave, ipsec, funcexpr, cyconf, syslogng, iptables, mgetty, snmpd, conserver.

## llconf utility ##

The llconf utility can be used in shell scripts to parse configuration files and also to modify them. It can also output the tree representation.

Examples:
```
 #llconf ifupdown -i examples/etc/network/interfaces -s dump
 (root)
        auto
                lo
                eth0
        iface
                eth0
                        family = 'inet'
                        method = 'static'
                        address = '192.168.1.1'
                        netmask = '255.255.255.0'
                        gateway = '192.168.48.1'
                        broadcast = '192.168.1.255'
        iface
                eth1
                        family = 'inet'
                        method = 'dhcp'

 # llconf pair -i /etc/resolv.conf -s dump
 (root)
        search = 'foobar.com'
        nameserver = '192.168.1.1'
        nameserver = '192.168.1.2'
```

Each item in the conf files is accessible with a path, separated by slashes. For example, the path to the IP address of eth0 is
> iface/eth0/address

The llconf utility can fetch those values using the get command:

```
 # llconf ifupdown -i /etc/network/interfaces get iface/eth0/address
 192.168.1.1
```

But it can do more: it can also set that value, thus recreating the whole file identically, but with that change:

```
 # llconf ifupdown -i examples/etc/network/interfaces -o - set iface/eth0/address 192.168.2.2
 auto lo eth0
 
 iface eth0 inet static
        address 192.168.2.2
        netmask 255.255.255.0
        gateway 192.168.48.1
        broadcast 192.168.1.255
 
 iface eth1 inet dhcp
```

We did not show the comments in that file, but those are preserved too.

Also note that llconf does not completely understand the configuration file, and it does not have to. For example, we can have an interface configuration like this:

```
 iface eth0 inet dhcp
    wireless-essid wlantest
    wireless-key s:topsecret
```

and llconf wouldn't choke at all, although it does not have a clue about WLAN (it has no idea about IP addresses either). Setting iface/eth0/wireless-essid just works. llconf understands the structure of a file, but not its contents.

Note that this works for any configuration file which has a parser imlemented. We can set in the same manner the nameserver in /etc/resolv.conf.

## C API ##

Using the llconf binary program every time to set or get a value is not very fast. For repeated settings it is not very efficient. A program using llconf therefore can link the library libllconf, and use the API to parse a conf file and access the values.

This eases parsing a configuration for ''any'' application, also if the application only needs to read the file, but never modifies it. The majority of applications do not ''write'' their configuration files.

A typical application will read its configuration when it starts, and parse it into binary structures. llconf makes this easy. Let us suppose we have a server application, which needs some port configuration, and chooses to use the ini syntax for its configuration file:

```
 [listen]
   interface = 127.0.0.1
   port = 1234
```

The tree structure for this file would look like this, if read in using the llconf utility:

```
 (root)
        listen
                interface = '127.0.0.1'
                port = '1234'
```
The application code that reads this file would look like this:

```
 void readconf(const char *file)
 {
    struct cnf_node *cn_conf, *cn_top;
    struct cnf_module *mod_ini;
 
    register_ini(NULL); /* register the module. We do not need any options. */
    mod_ini = find_cnfmodule("ini"); /* find the module - fast, we only have one registered */
 
    cn_conf = cnfmodule_parse_file(mod_ini, file); /* parse the file into the tree */
 
    /* put values into a structure: */
    /* we may want to add some error checking... */
    for(cn_top = cn_conf->first_child; cn_top; cn_top = cn_top->next){ /* iterate through all sub trees */
      if(strcmp(cnfnode_getname(cn_top), "listen) == 0){
        for(cn = cn_top->first_child; cn; cn = cn->next){ /* iterate through all children of the sub tree */
          if(strcmp(cnfnode_getname(cn), "interface") == 0){
            inet_aton(cnfnode_getval(cn), &conf.listen_addr);
          }else if(strcmp(cnfnode_getname(cn), "port") == 0){
            conf.listen_port = atoi(cnfnode_getval(cn));
          }
        }
      }
    }
 
    /* we are done */
    destroy_cnftree(cn_conf); /* free memory used by the tree */
 }

```

The code snippet above uses the tree as parsed in by the llconf API to fill in values into a binary structure. It needs to do this only once when the application starts (and maybe again when it gets a HUP signal, if that is desired).


# An example is /etc/network/interfaces:

/etc/network/interfaces -- configuration file for ifup(8), ifdown(8):

```
auto lo

iface lo inet loopback

auto eth0

# stupid comment: my ethernet device
iface eth0 inet dhcp
 address 192.168.1.2
 netmask 255.255.255.0
 gateway 192.168.1.254
```

And the tree representation:

> auto
> > lo

> iface
> > lo
> > > family = 'inet'
> > > method = 'loopback'

> auto
> > eth0

> iface
> > eth0
> > > family = 'inet'
> > > method = 'dhcp'
> > > address = '192.168.1.2'
> > > netmask = '255.255.255.0'
> > > gateway = '192.168.1.254'

So you can access the ip address of eth0 in a filesystem like syntax:
iface/eth0/address

llconf parses, using the correct module, the file and represents it
internally as a tree. An entry can be changed, and then parsed back
into the original syntax. And it does more: by putting the comments
into the tree, we can preserve them as well.

The drawback of this approach is that lots of parsers will be needed,
one for each syntax. Fortunately, the situation is not that bad (I
hope..). Lots of config files use simple syntaxes like

key value

or

key=value

Others consist just of coloumns, separated by a character,
eg. /etc/passwd. A little bit more complex examples are
/etc/network/interfaces or /etc/X11/XF86Config. But there are common
among most of them, and a parser for one syntax can be reused for
another, using some options.

Also, in representing a config file as a tree, and we use the variable
names for names of the branches, names are not uniq any more. For
/e/n/i, we have the stanzas, eg. 'iface', several times. So we need
subscipts to unambiguously identify an entry.

The unification now is done this way: once the parser is ready, all
variables in the config file can be accessed in a similar way. To get
the ip address of eth0, just write:

```
# llconf ifupdown get iface/eth0/address
```

To set it, write:
```
# llconf ifupdown set iface/eth0/address 192.168.1.1
```

Similar for XF86Config, eg. to get the device file for the mouse:
```
# llconf cf86config get InputDevice/Device
```

Once this works, it is possible to write configuration editors,
package update scripts and web interfaces, without worrying about the
syntax any more.
