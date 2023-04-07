include $(TOPDIR)/rules.mk

PKG_NAME:=port_control
PKG_RELEASE:=1
PKG_VERSION:=1.0.1

include $(INCLUDE_DIR)/package.mk

define Package/port_control
	DEPENDS:=+libserialport +libubus +libubox +libblobmsg-json 
	CATEGORY:=Utilities
	TITLE:=port_control
endef


define Package/port_control/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/port_control.init $(1)/etc/init.d/port_control
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/port_control $(1)/usr/bin
endef

$(eval $(call BuildPackage,port_control))

