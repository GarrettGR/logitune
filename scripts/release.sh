#!/bin/bash
set -e

# Usage: ./scripts/release.sh [major|minor|patch]
# Default: patch

BUMP_TYPE="${1:-patch}"

# Read current version from CMakeLists.txt
CURRENT=$(grep -oP 'project\(logitune VERSION \K[0-9]+\.[0-9]+\.[0-9]+' CMakeLists.txt)
if [ -z "$CURRENT" ]; then
    echo "❌ Could not read version from CMakeLists.txt"
    exit 1
fi

IFS='.' read -r MAJOR MINOR PATCH <<< "$CURRENT"

case "$BUMP_TYPE" in
    major) MAJOR=$((MAJOR + 1)); MINOR=0; PATCH=0 ;;
    minor) MINOR=$((MINOR + 1)); PATCH=0 ;;
    patch) PATCH=$((PATCH + 1)) ;;
    *) echo "❌ Usage: $0 [major|minor|patch]"; exit 1 ;;
esac

NEW_VERSION="$MAJOR.$MINOR.$PATCH"
echo "📦 Releasing: $CURRENT → $NEW_VERSION ($BUMP_TYPE)"

# Check for uncommitted changes
if [ -n "$(git status --porcelain)" ]; then
    echo "❌ Uncommitted changes. Commit or stash first."
    exit 1
fi

# Run tests
echo "🔍 Running tests..."
cmake --build build -j$(nproc) > /dev/null 2>&1
QT_QPA_PLATFORM=offscreen ./build/tests/logitune-tests --gtest_print_time=0 > /dev/null 2>&1
QT_QPA_PLATFORM=offscreen ./build/tests/logitune-tray-tests --gtest_print_time=0 > /dev/null 2>&1
QT_QPA_PLATFORM=offscreen ./build/tests/qml/logitune-qml-tests > /dev/null 2>&1
echo "✅ All tests passed"

# Bump version in CMakeLists.txt
sed -i "s/project(logitune VERSION $CURRENT/project(logitune VERSION $NEW_VERSION/" CMakeLists.txt

# Update metainfo release
TODAY=$(date +%Y-%m-%d)
sed -i "s/<release version=\"[^\"]*\" date=\"[^\"]*\">/<release version=\"$NEW_VERSION\" date=\"$TODAY\">/" data/com.logitune.Logitune.metainfo.xml

# Commit and tag
git add CMakeLists.txt data/com.logitune.Logitune.metainfo.xml
git commit -m "release: v$NEW_VERSION"
git tag -a "v$NEW_VERSION" -m "Release v$NEW_VERSION"
echo "🏷️  Tagged v$NEW_VERSION"

# Build Flatpak bundle
echo "📦 Building Flatpak..."
flatpak-builder --user --install --force-clean build-flatpak com.logitune.Logitune.yml > /dev/null 2>&1
flatpak build-bundle ~/.local/share/flatpak/repo "logitune-$NEW_VERSION.flatpak" com.logitune.Logitune
echo "✅ Flatpak bundle: logitune-$NEW_VERSION.flatpak"

# Push commit + tag
git push origin master
git push origin "v$NEW_VERSION"
echo "🚀 Pushed to origin"

# Create GitHub release with the bundle
if command -v gh > /dev/null 2>&1; then
    gh release create "v$NEW_VERSION" \
        "logitune-$NEW_VERSION.flatpak" \
        --title "Logitune v$NEW_VERSION" \
        --generate-notes
    echo "✅ GitHub release created"
else
    echo "⚠️  gh CLI not installed — create the release manually at:"
    echo "   https://github.com/mmaher88/logitune/releases/new?tag=v$NEW_VERSION"
fi

echo ""
echo "🎉 Released v$NEW_VERSION"
