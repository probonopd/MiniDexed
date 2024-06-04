RPI ?= 4

.PHONY: all
all: clean build build-stub zip

.PHONY: build
build:
	@mkdir -p build/kernels build/sdcard
	@./scripts/build.sh
	@cp ./src/kernel*.img ./build/kernels/

.PHONY: build-stub
build-stub:
	@mkdir -p build/kernels build/sdcard
	@./scripts/build-stub.sh

.PHONY: clean
clean:
	@rm -rf build

.PHONY: get-sysex
get-sysex:
	@mkdir -p build/sdcard && \
		cd ./build/sdcard && \
		../../scripts/get-sysex.sh && \
		cd -

.PHONY: get-performances
get-performances:
	@./scripts/get-performances.sh

.PHONY: install
install:
	@echo "TODO(antoniae)"

.PHONY: install-toolchain
install-toolchain:
	@./scripts/install-toolchain.sh

.PHONY: submodules
submodules:
	@./scripts/submodules.sh

.PHONY: zip
zip:
	@./scripts/zip.sh
