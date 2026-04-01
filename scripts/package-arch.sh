#!/bin/bash
set -e

VERSION=$(grep -oP 'project\(logitune VERSION \K[0-9]+\.[0-9]+\.[0-9]+' CMakeLists.txt)

echo "📦 Building Arch package v$VERSION"

# Create PKGBUILD
cat > /tmp/PKGBUILD << EOF
# Maintainer: Mina Maher <mina.maher88@hotmail.com>
pkgname=logitune
pkgver=$VERSION
pkgrel=1
pkgdesc="Logitech device configurator for Linux"
arch=('x86_64')
url="https://github.com/mmaher88/logitune"
license=('GPL-3.0-or-later')
depends=('qt6-base' 'qt6-declarative' 'qt6-svg' 'systemd-libs')
makedepends=('cmake' 'ninja' 'qt6-tools')
source=()

build() {
    cd "$startdir"
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -Wno-dev
    cmake --build build -j\$(nproc)
}

package() {
    cd "$startdir"
    DESTDIR="\$pkgdir" cmake --install build
}
EOF

# Build with makepkg
cd /tmp
makepkg -p PKGBUILD -f --noconfirm 2>&1 | tail -5
mv logitune-*.pkg.tar.* "$(pwd)/" 2>/dev/null || true

echo "✅ Package built"
