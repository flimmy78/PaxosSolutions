sbindir ?= $(PREFIX)/sbin
libdir ?= $(PREFIX)/lib/tgt

CFLAGS += -D_FILE_OFFSET_BITS=64
ifneq ($(shell test -e /usr/include/linux/signalfd.h && echo 1),)
CFLAGS += -DUSE_SIGNALFD
endif

ifneq ($(shell test -n $(shell find /usr/include -name "timerfd.h" | head -n1) && echo 1),)
CFLAGS += -DUSE_TIMERFD
endif

TGTDIR        = ../../../tgt-master
TGTUSR        = $(TGTDIR)/usr

VV_OBJS_DBG = bs_VV.o VV.o MC.o DB.o tgtdDbg.o mgmtDbg.o VVSession.o
VV_OBJS = bs_VV.o VV.o MC.o DB.o VVSession.o

LIBS    =  ../LVS/LVClient.o \
                -lStatus -lCacheBlock -lCacheDlg -lPaxosPFS \
                -lneo2 -lPaxosSession -lPaxos -lNWGadget -levent
LPATH   = -L../../../NWGadget/lib -L../../paxos/lib -L../../session/lib \
        -L../../status/lib -L../../PFS/lib -L../../cache/lib -L../../xjPing/lib
INCLUDES += -I../h -I../../../NWGadget/h -I../../paxos/h -I../../session/h \
                -I../../status/h -I../../cache/h -I../../PFS/h -I../../xjPing/h \
                        -I$(TGTUSR) -I$(TGTUSR)/iscsi

TGTD_OBJS += $(addprefix $(TGTUSR)/iscsi/, conn.o param.o session.o \
		iscsid.o target.o chap.o sha1.o md5.o transport.o iscsi_tcp.o \
		isns.o)

ifneq ($(CEPH_RBD),)
MODULES += bs_rbd.so
endif

ifneq ($(GLFS_BD),)
MODULES += bs_glfs.so
endif

ifneq ($(SD_NOTIFY),)
CFLAGS += -DUSE_SYSTEMD
endif

ifneq ($(shell test -e /usr/include/sys/eventfd.h && test -e /usr/include/libaio.h && echo 1),)
CFLAGS += -DUSE_EVENTFD
TGTD_OBJS += bs_aio.o
LIBS += -laio
endif

ifneq ($(ISCSI_RDMA),)
TGTD_OBJS += iscsi/iser.o iscsi/iser_text.o
LIBS += -libverbs -lrdmacm
endif

INCLUDES += -I.

CFLAGS += -D_GNU_SOURCE
CFLAGS += $(INCLUDES)
ifneq ($(DEBUG),)
CFLAGS += -g -O0 -ggdb -rdynamic
else
CFLAGS += -g -O2 -fno-strict-aliasing
endif
CFLAGS += -Wall -Wstrict-prototypes -fPIC
CFLAGS += -DTGT_VERSION=\"$(VERSION)$(EXTRAVERSION)\"
CFLAGS += -DBSDIR=\"$(DESTDIR)$(libdir)/backing-store\"

LIBS += -lpthread -ldl

ifneq ($(SD_NOTIFY),)
LIBS += -lsystemd
endif

#PROGRAMS += tgtd tgtadm tgtimg
PROGRAMS += tgtdVV tgtadm VVAdmin
PROGRAMS += MC2LVTest VVTest # VVAdmin
TGTD_OBJS += $(addprefix $(TGTUSR)/, tgtd.o mgmt.o target.o scsi.o log.o driver.o util.o work.o \
		concat_buf.o parser.o spc.o sbc.o mmc.o osd.o scc.o smc.o \
		ssc.o libssc.o bs_rdwr.o bs_ssc.o \
		bs_null.o bs_sg.o bs.o libcrc32c.o ) #bs_sheepdog.o)

TGTD_DEP = $(TGTD_OBJS:.o=.d)

LDFLAGS = -Wl,-E,-rpath=$(libdir)

.PHONY:all
all: $(PROGRAMS) $(MODULES)

MC2LVTest: MC2LVTest.o MC.o VVSession.o
	$(CC) $(LPATH) $^ -o $@ $(LIBS)
	cp $@ ../bin

VVTest: VVTest.o VV.o MC.o VVSession.o DB.o
	$(CC) $(LPATH) $^ -o $@ $(LIBS)
	cp $@ ../bin

VVAdmin: VVAdmin.o VV.o MC.o VVSession.o DB.o
	$(CC) $(LPATH) $^ -o $@ $(LIBS)
	cp $@ ../bin

tgtdVV: $(TGTD_OBJS) $(VV_OBJS)
	echo $(CC) $(LPATH) $^ -o $@ $(LIBS)
	$(CC) $(LPATH) $^ -o $@ $(LDFLAGS) $(LIBS)
	cp $@ ../bin

-include $(TGTD_DEP)

TGTADM_OBJS = $(addprefix $(TGTUSR)/, tgtadm.o concat_buf.o)
TGTADM_DEP = $(TGTADM_OBJS:.o=.d)

tgtadm: $(TGTADM_OBJS)
	$(CC) $^ -o $@
	cp $@ ../bin

-include $(TGTADM_DEP)

TGTIMG_OBJS = tgtimg.o libssc.o libcrc32c.o
TGTIMG_DEP = $(TGTIMG_OBJS:.o=.d)

tgtimg: $(TGTIMG_OBJS)
	$(CC) $^ -o $@

-include $(TGTIMG_DEP)

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	@$(CC) -MM $(CFLAGS) -MF $*.d -MT $*.o $*.c

%.so: %.c
	$(CC) -shared $(CFLAGS) $*.c -o $*.so

bs_rbd.so: bs_rbd.c
	$(CC) -shared $(CFLAGS) bs_rbd.c -o bs_rbd.so -lrados -lrbd

bs_glfs.so: bs_glfs.c
	$(CC) -I/usr/include/glusterfs/api -shared $(CFLAGS) bs_glfs.c -o bs_glfs.so -lgfapi

.PHONY: install
#install: $(PROGRAMS) $(MODULES)
#	install -d -m 755 $(DESTDIR)$(sbindir)
#	install -m 755 $(PROGRAMS) $(DESTDIR)$(sbindir)
#ifneq ($(MODULES),)
#	install -d -m 755 $(DESTDIR)$(libdir)/backing-store
#	install -m 755 $(MODULES) $(DESTDIR)$(libdir)/backing-store
#endif

.PHONY: clean
clean:
	rm -f *.[od] *.so $(PROGRAMS) iscsi/*.[od] ibmvio/*.[od] fc/*.[od]

	cd ../bin; rm -f MC2LVTest VVTest VVAdmin tgtdVV tgtadm

H = ../h/
BIN = ../bin/
ROOT = ../../

install:
	cp $(H)VV.h $(ROOT)h
	cp $(BIN)tgtdVV $(ROOT)bin
	cp $(BIN)tgtadm $(ROOT)bin
	cp $(BIN)VVAdmin $(ROOT)bin

uninstall:
	rm -f $(ROOT)h/VV.h
	rm -f $(ROOT)bin/tgtdVV
	rm -f $(ROOT)bin/tgtadm
	rm -f $(ROOT)bin/VVAdmin
	
