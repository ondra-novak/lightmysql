
ACLOCAL_AMFLAGS = -I Build 

SUBDIRS = .
LIGHTSPEED_H = $(abs_top_srcdir)/../lightspeed/src


noinst_LIBRARIES = liblightmysql.a


liblightmysql.a:
	$(MAKE) -C Build --file makefile all "GPP=g++ -O3  -c -fno-strict-aliasing -Wall -fPIC -I$(LIGHTSPEED_H)"
	ln -f Build/liblightmysql.a ./
	
	
clean-local: clean-src
distclean-local: clean-src
.PHONY: clean-src

clean-src:
	$(MAKE) -k -C Build --file makefile clean
	rm -f ./liblightmysql.a


FORCE: