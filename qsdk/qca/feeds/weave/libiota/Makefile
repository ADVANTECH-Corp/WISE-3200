include $(TOPDIR)/rules.mk

PKG_NAME:=libiota
PKG_VERSION:=1.1.0
PKG_RELEASE=$(PKG_SOURCE_VERSION)
PKG_USE_MIPS16:=0


PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://source.codeaurora.org/quic/qsdk/oss/upstream/libiota
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=af4538bb04a15705fc56f7a10a6f90851b9a4dc9
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz


BUILD_MODE:=Release

include $(INCLUDE_DIR)/package.mk

define Package/libiota
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Google IOTA library
  DEPENDS:= +libstdcpp +libpthread +ca-certificates +libcurl +librt +libopenssl +libcrypto
  URL:=https://weave.googlesource.com/libiota
endef

define Package/libiota/description
	libIOTA is the library with implementation of
	IOTA protocol on device side.
endef

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

TARGET_BUILD_ARGS= \
		   -D_GLIBCXX_USE_C99=1 \
		   -DSUPPORTS_FLOATING_POINT_FORMAT \
		   -fPIC \

define Package/libiota/config
menu "Configuration"
    depends on PACKAGE_libiota
config LIBIOTA_EXAMPLES
    bool "Build example application"
    default y
endmenu
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		CC="$(TARGET_CC) $(TARGET_BUILD_ARGS) -std=gnu99" \
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
		CXX="$(TARGET_CXX) $(TARGET_BUILD_ARGS) -std=gnu++11" \
		CXXFLAGS="$(TARGET_CXXFLAGS) $(EXTRA_CFLAGS)" \
		LINK="$(TARGET_CROSS)g++ $(TARGET_BUILD_ARGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		AR="$(TARGET_CROSS)ar"
ifdef CONFIG_LIBIOTA_EXAMPLES
	$(MAKE) -C $(PKG_BUILD_DIR)/examples/host/light \
		CC="$(TARGET_CC) $(TARGET_BUILD_ARGS) -std=gnu99 -lcrypto -lssl" \
		LINK="$(TARGET_CROSS)g++ $(TARGET_BUILD_ARGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		AR="$(TARGET_CROSS)ar"
endif
endef

define Package/libiota/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/out/host/libiota/libiota.a $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/out/host/third_party/googletest/googletest/libgtest.a $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/out/host/third_party/googletest/googlemock/libgmock.a $(1)/lib
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/out/host/test/gtest_runner $(1)/usr/sbin
ifdef CONFIG_LIBIOTA_EXAMPLES
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/out/host/examples/light/light $(1)/usr/sbin
endif
endef

$(eval $(call BuildPackage,libiota))
