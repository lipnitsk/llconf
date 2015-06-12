EXTRA_DIST=mkallmodules

pkginclude_HEADERS = entry.h file.h ifupdown.h lines.h modules.h nodes.h \
			pair.h ppp.h shell.h strutils.h table.h ini.h pslave.h route.h iproute.h ipsec.h \
			funcexpr.h cyconf.h syslogng.h iptables.h mgetty.h snmpd.h conserver.h cron.h dhcp.h allmodules.h \
			options.h dhcp_leases.h

AM_CFLAGS=-DLLCONF

lib_LTLIBRARIES = libllconf.la

bin_PROGRAMS=llconf
noinst_PROGRAMS=example

libllconf_la_SOURCES=\
	allmodules.c allmodules.h \
	modules.c modules.h \
	entry.c  entry.h \
	lines.c  lines.h \
	nodes.c  nodes.h \
	strutils.c  strutils.h \
	file.c file.h \
	ifupdown.c  ifupdown.h \
	shell.c  shell.h \
	table.c  table.h \
	pair.c  pair.h \
	ppp.c  ppp.h \
	ini.c ini.h \
	route.c route.h \
	iproute.c iproute.h \
	pslave.c pslave.h \
	ipsec.c ipsec.h \
	funcexpr.c funcexpr.h \
	cyconf.c cyconf.h \
	syslogng.c syslogng.h \
	iptables.c iptables.h \
	snmpd.c snmpd.h \
	mgetty.c mgetty.h \
	conserver.c conserver.h \
	dhcp_leases.c dhcp_leases.h \
	cron.c cron.h \
	dhcp.c dhcp.h \
	options.c options.h

llconf_SOURCES=	\
	allmodules.h \
	cyconf.h \
	dhcp.h \
	dhcp_leases.h \
	entry.h \
	funcexpr.h \
	file.h \
	ifupdown.h \
	ini.h \
	ipsec.h \
	lines.h \
	modules.h \
	nodes.h \
	pair.h \
	ppp.h \
	pslave.h \
	route.h \
	iproute.h \
	shell.h \
	strutils.h \
	syslogng.h \
	table.h \
	iptables.h \
	mgetty.h \
	snmpd.h \
	conserver.h \
	llconf.c

llconf_LDADD = libllconf.la

example_SOURCES=	\
	example.c

example_LDADD = libllconf.la

allmodules.h: allmodules
allmodules.c: allmodules
allmodules: mkallmodules
	./mkallmodules file shell table ifupdown pair ppp ini route iproute pslave ipsec funcexpr cyconf syslogng iptables mgetty snmpd conserver cron dhcp dhcp_leases

libllconf_la_LDFLAGS = -version-info 1:0:1

clean:
	rm -f allmodules.h allmodules.c

.PHONY: clean allmodules
