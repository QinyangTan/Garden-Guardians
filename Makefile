SHELL := /bin/sh

# Build configuration (override on CLI, e.g. make BUILD_TYPE=Debug)
BUILD_DIR ?= build
BUILD_TYPE ?= Release
TARGET ?= GardenGuardians
DEPS_DIR ?= /tmp/GardenGuardians_deps

# CMake command (override if needed)
CMAKE ?= cmake
CMAKE_ARGS ?=

.PHONY: help check-tools configure build run debug release rebuild clean distclean dist install-dist

help:
	@echo "Garden Guardians Makefile targets:"
	@echo "  make / make build       Configure + build ($(BUILD_TYPE))"
	@echo "  make run                Build then run executable"
	@echo "  make debug              Configure + build Debug"
	@echo "  make release            Configure + build Release"
	@echo "  make clean              Remove build outputs only"
	@echo "  make distclean          Remove full build directory"
	@echo "  make dist               Build Release and copy executable + assets/ into dist/ (share this folder)"
	@echo "  make install-dist       Same as dist, output under build/GardenGuardians-portable (CMake target)"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR=<dir>         Default: build"
	@echo "  BUILD_TYPE=<type>       Default: Release"
	@echo "  TARGET=<name>           Default: GardenGuardians"
	@echo "  DEPS_DIR=<dir>          Default: /tmp/GardenGuardians_deps"
	@echo "  CMAKE_ARGS=<args>       Extra cmake configure args"

check-tools:
	@if command -v "$(CMAKE)" >/dev/null 2>&1; then \
		exit 0; \
	else \
		echo "Error: '$(CMAKE)' is not installed or not in PATH."; \
		echo "Install CMake first:"; \
		echo "  macOS (Homebrew): brew install cmake"; \
		echo "  Ubuntu/Debian: apt install cmake"; \
		echo "  Fedora: dnf install cmake"; \
		echo "  Arch: pacman -S cmake"; \
		exit 127; \
	fi

configure: check-tools
	$(CMAKE) -S . -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" -DFETCHCONTENT_BASE_DIR="$(DEPS_DIR)" $(CMAKE_ARGS)

build: configure
	$(CMAKE) --build "$(BUILD_DIR)" --config "$(BUILD_TYPE)" -j

run: build
	@if [ -x "$(BUILD_DIR)/$(TARGET)" ]; then \
		"$(BUILD_DIR)/$(TARGET)"; \
	elif [ -x "$(BUILD_DIR)/$(BUILD_TYPE)/$(TARGET)" ]; then \
		"$(BUILD_DIR)/$(BUILD_TYPE)/$(TARGET)"; \
	elif [ -x "$(BUILD_DIR)/$(BUILD_TYPE)/$(TARGET).exe" ]; then \
		"$(BUILD_DIR)/$(BUILD_TYPE)/$(TARGET).exe"; \
	elif [ -x "$(BUILD_DIR)/$(TARGET).exe" ]; then \
		"$(BUILD_DIR)/$(TARGET).exe"; \
	else \
		echo "Executable not found. Checked common CMake output paths."; \
		exit 1; \
	fi

debug:
	$(MAKE) BUILD_TYPE=Debug build

release:
	$(MAKE) BUILD_TYPE=Release build

rebuild: clean build

clean:
	$(CMAKE) --build "$(BUILD_DIR)" --target clean --config "$(BUILD_TYPE)" || true

distclean:
	@echo "Removing $(BUILD_DIR) ..."
	@if [ -d "$(BUILD_DIR)" ]; then \
		chmod -R u+w "$(BUILD_DIR)" 2>/dev/null || true; \
		chflags -R nouchg "$(BUILD_DIR)" 2>/dev/null || true; \
		rm -rf "$(BUILD_DIR)" 2>/dev/null || true; \
		if [ -d "$(BUILD_DIR)" ]; then \
			$(CMAKE) -E remove_directory "$(BUILD_DIR)" 2>/dev/null || true; \
		fi; \
	fi
	@if [ -d "$(BUILD_DIR)" ]; then \
		echo "Warning: could not fully remove $(BUILD_DIR)."; \
		echo "Try manually: chmod -R u+w \"$(BUILD_DIR)\" && chflags -R nouchg \"$(BUILD_DIR)\" && rm -rf \"$(BUILD_DIR)\""; \
		exit 1; \
	fi

# Folder you can zip and share: binary + assets/ side by side (game resolves paths from the executable directory).
DIST_DIR ?= dist/GardenGuardians-portable

install-dist: build
	@$(CMAKE) --build "$(BUILD_DIR)" --config "$(BUILD_TYPE)" --target portable_bundle
	@echo "Portable folder: $(BUILD_DIR)/GardenGuardians-portable"

dist: release
	@$(CMAKE) --build "$(BUILD_DIR)" --config "$(BUILD_TYPE)" --target portable_bundle
	@echo "Copying to $(DIST_DIR) ..."
	@rm -rf "$(DIST_DIR)"
	@$(CMAKE) -E copy_directory "$(BUILD_DIR)/GardenGuardians-portable" "$(DIST_DIR)"
	@echo ""
	@echo "Portable build ready: $(DIST_DIR)"
	@echo "Zip this folder and share. Players run GardenGuardians (or .exe) from inside the folder."
