include $(TOPDIR)/rules.mk

PKG_NAME:=ath10k-firmware
PKG_DK_FW_VERSION:=WLAN.TF.1.0-00002-QCATFSWPZ-5
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk

include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=QCA ath10k firmware
endef

define Package/$(PKG_NAME)/description
  This package contains the ATH10k Firmware binaries
endef

Build/Compile=true

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9377/hw1.0
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/ath10k/QCA9377/hw1.0/board.bin  \
	$(1)/lib/firmware/ath10k/QCA9377/hw1.0/
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/ath10k/QCA9377/hw1.0/firmware-5-$(PKG_DK_FW_VERSION).bin \
	$(1)/lib/firmware/ath10k/QCA9377/hw1.0/firmware-5.bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
