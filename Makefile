LIBNAME:=lightmysql
LIBINCLUDES:=src
SEARCHPATHS="./ ../ ../../ /usr/include/ /usr/local/include/"
CXX=clang++

selectpath=$(abspath $(firstword $(foreach dir,$(1),$(wildcard $(dir)$(2)))))

LIBLIGHTSPEED=$(call selectpath,$(SEARCHPATHS),lightspeed)
NEEDLIBS:=$(LIBLIGHTSPEED)

include building/build_lib.mk


	