#############################################################
#
# E2 Tally
#
#############################################################
RPI_MATRIX_VERSION = 86f243c2203cdf5f3a73057cee2a675738dfea9c
RPI_MATRIX_SITE = git://github.com/depili/rpi-matrix.git

define RPI_MATRIX_EXTRACT_CMDS
     rm -rf $(@D)
     (git clone --depth 1 $(RPI_MATRIX_SITE) $(@D) && \
     cd $(@D) && \
     git checkout $(RPI_MATRIX_VERSION) && \
     git submodule init && \
     git submodule update)
     touch $(@D)/.stamp_downloaded
endef

define RPI_MATRIX_BUILD_CMDS
     #$(MAKE) CXX=$(TARGET_CXX) CC=$(TARGET_CC) LD=$(TARGET_LD) -C $(@D)/matrix/lib all
     $(MAKE) CXX=$(TARGET_CXX) CC=$(TARGET_CC) LD=$(TARGET_LD) -C $(@D) all
endef

define RPI_MATRIX_INSTALL_TARGET_CMDS
     mkdir -p $(TARGET_DIR)/root/rpi-matrix
     mkdir -p $(TARGET_DIR)/root/rpi-matrix/fonts
     cp -r $(@D)/matrix/fonts/* $(TARGET_DIR)/root/rpi-matrix/fonts/
     $(INSTALL) -D -m 0755 $(@D)/udp $(TARGET_DIR)/root/rpi-matrix/udp
     $(INSTALL) -D -m 0755 $(@D)/matrix/lib/librgbmatrix.so.1  $(TARGET_DIR)/usr/lib/librgbmatrix.so.1

     # init scripts
     $(INSTALL) -D -m 0755 $(TOPDIR)/package/rpi-matrix/S02copy_files $(TARGET_DIR)/etc/init.d/S02copy_files
     $(INSTALL) -D -m 0755 $(TOPDIR)/package/rpi-matrix/S90matrix $(TARGET_DIR)/etc/init.d/S90matrix
     
     echo "/root/rpi-matrix/udp $(BR2_PACKAGE_RPI_MATRIX_CONFIG)" > $(@D)/matrix_cmd.sh
     $(INSTALL) -D -m 0755 $(@D)/matrix_cmd.sh $(TARGET_DIR)/root/rpi-matrix/matrix_cmd.sh
endef

define RPI_MATRIX_UNINSTALL_TARGET_CMDS
     rm -fr $(TARGET_DIR)/root/rpi-matrix
     rm -fr $(TARGET_DIR)/usr/lib/librgbmatrix.so.1
     rm -fr $(TARGET_DIR)/etc/init.d/S02copy_files
     rm -fr $(TARGET_DIR)/etc/init.d/S90matrix
endef

$(eval $(generic-package))
