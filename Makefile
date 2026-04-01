.PHONY: build test test-qml test-tray test-all run install uninstall release clean help setup-hooks package-deb package-rpm package-arch

IS_CONTAINER := $(shell test -f /.dockerenv && echo 1)

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "} !seen[$$1]++ {printf "\033[36m%-15s\033[0m %s\n", $$1, $$2}' | sort

build: ## Build the project
	@cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -Wno-dev
	@cmake --build build -j$$(nproc)

test: ## Run C++ unit/integration tests
	@QT_QPA_PLATFORM=offscreen ./build/tests/logitune-tests

test-qml: ## Run QML component tests
	@QT_QPA_PLATFORM=offscreen ./build/tests/qml/logitune-qml-tests

test-tray: ## Run tray manager tests
	@QT_QPA_PLATFORM=offscreen ./build/tests/logitune-tray-tests

test-all: test test-tray test-qml ## Run all tests

ifdef IS_CONTAINER
setup-hooks: ## Install git hooks
	@sudo cp scripts/pre-push .git/hooks/pre-push
	@sudo chmod +x .git/hooks/pre-push
	@echo "Git hooks installed"
else
run: build ## Build and run the app (host only)
	@./build/src/app/logitune --debug

install: ## Install to system (host only)
	@cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -Wno-dev
	@cmake --build build -j$$(nproc)
	@sudo cmake --install build
	@sudo udevadm control --reload-rules
	@sudo udevadm trigger
	@echo "✅ Installed. Run: logitune"

uninstall: ## Uninstall from system (host only)
	@sudo rm -f /usr/bin/logitune
	@sudo rm -f /usr/lib/udev/rules.d/71-logitune.rules
	@sudo rm -f /usr/share/applications/logitune.desktop
	@sudo rm -f /usr/share/icons/hicolor/scalable/apps/com.logitune.Logitune.svg
	@sudo rm -f /usr/share/metainfo/com.logitune.Logitune.metainfo.xml
	@sudo udevadm control --reload-rules
	@echo "✅ Uninstalled"

package-deb: ## Build .deb package (Debian/Ubuntu)
	@scripts/package-deb.sh

package-rpm: ## Build .rpm package (Fedora/openSUSE)
	@scripts/package-rpm.sh

package-arch: ## Build Arch/AUR PKGBUILD
	@scripts/package-arch.sh

release: ## Release — version bump, tag, push (host only)
	@./scripts/release.sh $(or $(BUMP),patch)

setup-hooks: ## Install git hooks
	@cp scripts/pre-push .git/hooks/pre-push
	@chmod +x .git/hooks/pre-push
	@echo "Git hooks installed"
endif

clean: ## Remove build artifacts
	@rm -rf build
