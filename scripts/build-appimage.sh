#!/usr/bin/env bash
set -euo pipefail

# Build in a separate folder so packaging never dirties the source tree.
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="$project_root/build-appimage"
app_dir="$project_root/AppDir"
dist_dir="$project_root/dist"

# Prefer system tools, but automatically use copies downloaded into tools/.
linuxdeploy_bin="$(command -v linuxdeploy || true)"
[[ -n "$linuxdeploy_bin" ]] || linuxdeploy_bin="$project_root/tools/linuxdeploy-x86_64.AppImage"
[[ -x "$linuxdeploy_bin" ]] || { echo "linuxdeploy is required"; exit 1; }
mkdir -p "$build_dir" "$app_dir/usr/bin" "$app_dir/usr/share/applications" "$app_dir/usr/share/icons/hicolor/scalable/apps" "$app_dir/usr/plugins/platforms" "$app_dir/usr/plugins/sqldrivers" "$app_dir/usr/plugins/imageformats" "$dist_dir"
(cd "$build_dir" && qmake6 "$project_root/FitnessTracker.pro" && make -j"$(nproc)")
install -m755 "$build_dir/Fitness" "$app_dir/usr/bin/"
install -m644 "$project_root/packaging/fitness-tracker.desktop" "$app_dir/usr/share/applications/"
install -m644 "$project_root/packaging/fitness-tracker.svg" "$app_dir/usr/share/icons/hicolor/scalable/apps/"
install -m644 "$project_root/packaging/qt.conf" "$app_dir/usr/bin/"
# Copy only the plugins this application uses. linuxdeploy then resolves their
# library dependencies without pulling unrelated database server drivers.
install -m644 /usr/lib/qt6/plugins/platforms/libqxcb.so "$app_dir/usr/plugins/platforms/"
install -m644 /usr/lib/qt6/plugins/platforms/libqoffscreen.so "$app_dir/usr/plugins/platforms/"
install -m644 /usr/lib/qt6/plugins/sqldrivers/libqsqlite.so "$app_dir/usr/plugins/sqldrivers/"
install -m644 /usr/lib/qt6/plugins/imageformats/libqjpeg.so "$app_dir/usr/plugins/imageformats/"
(cd "$project_root" && \
  APPIMAGE_EXTRACT_AND_RUN=1 \
  OUTPUT="$dist_dir/Fitness-v0.81-x86_64.AppImage" \
  "$linuxdeploy_bin" --appdir "$app_dir" --output appimage)
