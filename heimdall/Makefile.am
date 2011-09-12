AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CPPFLAGS = $(DEPS_CFLAGS) -I../libpit/Source
STATIC_LIBS = ../libpit/libpit-1.3.a

bin_PROGRAMS = heimdall
heimdall_SOURCES = source/BeginDumpPacket.h source/BridgeManager.cpp \
	source/BridgeManager.h source/DumpPartPitFilePacket.h source/DumpResponse.h \
	source/EndFileTransferPacket.h source/EndModemFileTransferPacket.h \
	source/EndPitFileTransferPacket.h source/EndPhoneFileTransferPacket.h \
	source/EndSessionPacket.h source/FileTransferPacket.h source/FlashPartFileTransferPacket.h \
	source/FlashPartPitFilePacket.h source/Heimdall.h source/InboundPacket.h \
	source/Interface.cpp source/Interface.h source/main.cpp source/OutboundPacket.h \
	source/Packet.h source/PitFilePacket.h source/PitFileResponse.h source/ReceiveFilePartPacket.h \
	source/ResponsePacket.h source/SendFilePartPacket.h source/SendFilePartResponse.h \
	source/ControlPacket.h source/SessionSetupPacket.h source/SessionSetupResponse.h \
	source/DumpPartFileTransferPacket.h

heimdall_LDADD = $(DEPS_LIBS) $(STATIC_LIBS)

if LINUXTARGET
udevrulesdir = /lib/udev/rules.d
udevrules_DATA = 60-heimdall-galaxy-s.rules

install-data-hook:
if UDEVADM
	sudo udevadm control --reload_rules
else
	sudo service udev restart
endif

endif

dist_noinst_SCRIPTS = autogen.sh
