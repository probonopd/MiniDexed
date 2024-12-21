#
# Rules.mk
#

-include $(CIRCLE_STDLIB_DIR)/Config.mk

NEWLIBDIR   ?= $(CIRCLE_STDLIB_DIR)/install/$(NEWLIB_ARCH)
CIRCLEHOME  ?= $(CIRCLE_STDLIB_DIR)/libs/circle

include $(CIRCLEHOME)/Rules.mk

INCLUDE += \
	   -I $(CIRCLE_STDLIB_DIR)/include \
	   -I $(NEWLIBDIR)/include \
	   -I $(NET_DIR)

LIBS += \
 	$(NEWLIBDIR)/lib/libm.a \
	$(NEWLIBDIR)/lib/libc.a \
	$(NEWLIBDIR)/lib/libcirclenewlib.a \
	$(CIRCLEHOME)/addon/display/libdisplay.a \
	$(CIRCLEHOME)/addon/sensor/libsensor.a \
	$(CIRCLEHOME)/addon/Properties/libproperties.a \
	$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
  	$(CIRCLEHOME)/lib/usb/libusb.a \
	$(CIRCLEHOME)/lib/usb/gadget/libusbgadget.a \
 	$(CIRCLEHOME)/lib/input/libinput.a \
	$(CIRCLEHOME)/lib/sound/libsound.a \
 	$(CIRCLEHOME)/addon/fatfs/libfatfs.a \
 	$(CIRCLEHOME)/lib/fs/libfs.a \
  	$(CIRCLEHOME)/lib/sched/libsched.a \
	$(CIRCLEHOME)/lib/libcircle.a \
	$(CIRCLEHOME)/addon/wlan/hostap/wpa_supplicant/libwpa_supplicant.a \
	$(CIRCLEHOME)/addon/wlan/libwlan.a \
	$(CIRCLEHOME)/lib/net/libnet.a

EXTRACLEAN += $(NET_DIR)/*.d $(NET_DIR)/*.o

-include $(DEPS)
