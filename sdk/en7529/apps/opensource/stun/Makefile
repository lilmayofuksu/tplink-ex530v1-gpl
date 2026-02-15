# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stund
PKG_VERSION:=0.96
PKG_RELEASE:=7

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/uclibc++.mk
include $(INCLUDE_DIR)/package.mk

define Package/stun/Default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=$(CXX_DEPENDS)
  URL:=http://sourceforge.net/projects/stun
endef

define Package/stun/Default/description
  The STUN protocol (Simple Traversal of UDP through NATs) is described in the
  IETF RFC 3489, available at http://www.ietf.org/rfc/rfc3489.txt. It's used to
  help clients behind NAT to tunnel incoming calls through. This server is the
  counterpart to help the client identify the NAT and have it open the proper
  ports for it.
endef

define Package/stund
$(call Package/stun/Default)
  TITLE:=STUN server
endef

define Package/stund/description
$(call Package/stun/Default/description)
endef

define Package/stun-client
$(call Package/stun/Default)
  TITLE:=STUN test client
endef

define Package/stun-client/description
$(call Package/stun/Default/description)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
                CXX="$(TARGET_CXX)" \
                CFLAGS="$(TARGET_CFLAGS)" \
                DESTDIR="$(PKG_INSTALL_DIR)" \
                CXXFLAGS="$(TARGET_CFLAGS) -fno-rtti"  \
                CPPFLAGS="$$$$CPPFLAGS $(TARGET_CPPFLAGS)" \
                LDFLAGS="$$$$LDFLAGS $(TARGET_LDFLAGS)" \
                all
endef


define Package/stund/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/server $(1)/usr/sbin/stund
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/stund.init $(1)/etc/init.d/stund
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/stund.config $(1)/etc/config/stund
endef

define Package/stund/conffiles
/etc/config/stund
endef

define Package/stun-client/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/client $(1)/usr/sbin/stun-client
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/stuncd.init $(1)/etc/init.d/stuncd
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/stuncd.config $(1)/etc/config/stun
endef

$(eval $(call BuildPackage,stund))
$(eval $(call BuildPackage,stun-client))
