MOD_SOURCES := vxi.cpp vxi11_clnt.c vxi11_xdr.c
MOD_HEADERS := vxi.h
PROGRAMS := vxi_test
PKG_CONFIG := libtirpc

# VXI sources should be built before running get_gep script
$(shell make -f Makefile.vxi 1>&2)

include ../Makefile.inc

vxi11_clnt.o:vxi11_clnt.c
vxi11_xdr.o:vxi11_xdr.c

clean: rpc_clean
rpc_clean:
	rm -f vxi11.h vxi11_*.c

