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

define dev-dist
    @echo "TODO: ..."
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
