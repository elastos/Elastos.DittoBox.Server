include environ/$(HOST)-$(ARCH).mk

SRC_DIR  = $(ROOT_DIR)/src

define source-fetch
    @echo "Dummy source fetch ..."
endef

define configure
    @echo "Dummy configure ..."
endef

define compile
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make;
endef

define install
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make install;
endef

define uninstall
    @echo "Dummy uninstall ..."
endef

define make-clean
    cd $(SRC_DIR) && PREFIX=$(DIST_DIR) make clean;
endef

define config-clean
    $(call make-clean)
endef

define source-clean
    $(call make-clean)
endef

define dev-dist
    @case $(HOST) in \
        "Linux") \
            ;; \
        *) \
            echo "Error: Unsupported distribution package on $(HOST)"; \
            echo "Hint: Make debian dist package only allowed on Linux"; \
            exit 1; \
            ;; \
    esac

    ### clean generated environment.
    cd $(DIST_DIR) && { \
        rm -rf debian; \
        rm -rf elaoc-agentd.deb; \
    }

    cd $(DIST_DIR) && { \
        mkdir -p debian/usr/bin; \
        cp bin/elaoc-agentd debian/usr/bin; \
        strip debian/usr/bin/elaoc-agentd; \
        mkdir -p debian/usr/lib; \
	cp $(CARRIER_DIST_PATH)/$(HOST)-$(ARCH)/$(BUILD)/lib/libelacommon.so  debian/usr/lib; \
	cp $(CARRIER_DIST_PATH)/$(HOST)-$(ARCH)/$(BUILD)/lib/libelacarrier.so debian/usr/lib; \
	cp $(CARRIER_DIST_PATH)/$(HOST)-$(ARCH)/$(BUILD)/lib/libelasession.so debian/usr/lib; \
	strip debian/usr/lib/libelacommon.so; \
	strip debian/usr/lib/libelacarrier.so; \
	strip debian/usr/lib/libelasession.so; \
        mkdir -p debian/etc/elaoc; \
        cp $(ROOT_DIR)/config/elaoc-agent.conf debian/etc/elaoc; \
        mkdir -p debian/lib/systemd/system; \
        cp $(ROOT_DIR)/scripts/elaoc-agentd.service debian/lib/systemd/system; \
        mkdir -p debian/etc/init.d; \
        cp $(ROOT_DIR)/scripts/elaoc-agentd.sh debian/etc/init.d/elaoc-agentd; \
        mkdir -p debian/var/lib/elaoc-agentd; \
        mkdir -p debian/DEBIAN; \
        cp $(ROOT_DIR)/debian/* debian/DEBIAN; \
        fakeroot dpkg-deb --build debian elaoc-agentd.deb; \
    }
endef

source:
	$(call source-fetch)

config: source
	$(call configure)

make: config
	$(call compile) 

install: make
	mkdir -p $(DIST_DIR)
	$(call install)

dist: install
	$(call dev-dist)

clean:
	$(call make-clean)

config-clean:
	$(call config-clean)

source-clean:
	$(call source-clean)
