.PHONY: build test test-qml test-tray test-all run flatpak flatpak-setup release clean help setup-hooks

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
flatpak-setup: ## Install Flatpak SDK (first time only, ~2GB)
	@flatpak --user remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
	@flatpak --user install -y flathub org.kde.Platform//6.10 org.kde.Sdk//6.10

flatpak: ## Build Flatpak (build only — install on host)
	@sudo sysctl -w kernel.unprivileged_userns_clone=1 > /dev/null 2>&1 || true
	@flatpak-builder --force-clean --disable-rofiles-fuse build-flatpak com.logitune.Logitune.yml
	@echo "✅ Flatpak built. To install, run on host: flatpak install --user flatpak-repo com.logitune.Logitune"

setup-hooks: ## Install git hooks
	@sudo cp scripts/pre-push .git/hooks/pre-push
	@sudo chmod +x .git/hooks/pre-push
	@echo "Git hooks installed"
else
run: build ## Build and run the app (host only)
	@./build/src/app/logitune --debug

flatpak-setup: ## Install Flatpak SDK (first time only, host only)
	@flatpak --user remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
	@flatpak --user install -y flathub org.kde.Platform//6.10 org.kde.Sdk//6.10

flatpak: ## Build and install Flatpak (host only)
	@flatpak-builder --user --install --force-clean build-flatpak com.logitune.Logitune.yml

release: ## Release — version bump, tag, Flatpak, GitHub release (host only)
	@./scripts/release.sh $(or $(BUMP),patch)

setup-hooks: ## Install git hooks
	@cp scripts/pre-push .git/hooks/pre-push
	@chmod +x .git/hooks/pre-push
	@echo "Git hooks installed"
endif

clean: ## Remove build artifacts
	@rm -rf build build-flatpak flatpak-repo
