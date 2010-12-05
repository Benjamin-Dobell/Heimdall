AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4
AM_CPPFLAGS = $(DEPS_CFLAGS)

bin_PROGRAMS = heimdall
heimdall_SOURCES = source/BeginDumpPacket.h source/BridgeManager.cpp \
	source/BridgeManager.h source/ControlPacket.h source/DeviceInfoPacket.h  \
	source/DeviceInfoResponse.h source/DumpPartFileTransferPacket.h \
	source/DumpPartPitFilePacket.h source/DumpResponse.h source/EndFileTransferPacket.h \
	source/EndModemFileTransferPacket.h source/EndPhoneFileTransferPacket.h \
	source/FileTransferPacket.h source/FlashPartFileTransferPacket.h \
	source/FlashPartPitFilePacket.h source/Heimdall.h \
	source/InboundPacket.h source/InterfaceManager.cpp source/InterfaceManager.h \
	source/main.cpp source/OutboundPacket.h source/Packet.h source/PitFilePacket.h \
	source/PitFileResponse.h source/RebootDevicePacket.h source/ReceiveFilePartPacket.h \
	source/ResponsePacket.h source/SendFilePartPacket.h source/SendFilePartResponse.h
heimdall_LDADD = $(DEPS_LIBS)

dist_noinst_SCRIPTS = autogen.sh
