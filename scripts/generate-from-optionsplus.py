#!/usr/bin/env python3
"""Generate device descriptors from extracted Options+ data.

Combines Options+ device database (PIDs, names), depot images (front.png, side.png),
and metadata.json (button hotspots with CID-encoded slot IDs) to produce complete
JSON descriptors for the logitune-devices community repo.

Prerequisites:
    1. Extract the Options+ offline installer:
       python3 scripts/extract-depot.py --all /tmp/optionsplus/extracted --output-dir /tmp/optionsplus/devices
    2. Extract the main depot:
       python3 scripts/extract-depot.py /tmp/optionsplus/extracted/logioptionsplus.depot --output-dir /tmp/optionsplus/main

Usage:
    python3 scripts/generate-from-optionsplus.py \\
        --devices-dir /tmp/optionsplus/devices \\
        --main-dir /tmp/optionsplus/main/logioptionsplus \\
        --output-dir /tmp/logitune-devices-output
"""

import argparse
import json
import glob
import os
import re
import shutil
import sys

SLOT_NAME_MAP = {
    "SLOT_NAME_MIDDLE_BUTTON":       ("Middle click",     "default",           True),
    "SLOT_NAME_BACK_BUTTON":         ("Back",              "default",           True),
    "SLOT_NAME_FORWARD_BUTTON":      ("Forward",           "default",           True),
    "SLOT_NAME_GESTURE_BUTTON":      ("Gesture button",    "gesture-trigger",   True),
    "SLOT_NAME_DPI_BUTTON":          ("DPI button",        "default",           True),
    "SLOT_NAME_LEFT_SCROLL_BUTTON":  ("Shift wheel mode",  "smartshift-toggle", True),
    "SLOT_NAME_MODESHIFT_BUTTON":    ("Shift wheel mode",  "smartshift-toggle", True),
    "SLOT_NAME_SIDE_BUTTON_TOP":     ("Top button",        "default",           True),
    "SLOT_NAME_SIDE_BUTTON_BOTTOM":  ("Bottom button",     "default",           True),
    "SLOT_NAME_THUMBWHEEL":          ("Thumb wheel",       "default",           True),
}

# Left/right clicks are always present but not in metadata. Field names must
# match ControlDescriptor JSON keys parsed by src/core/devices/JsonDevice.cpp
# (controlId / buttonIndex / defaultName / defaultActionType). Mismatches are
# silently skipped at load time.
DEFAULT_CONTROLS = [
    {"controlId": "0x0050", "buttonIndex": 0, "defaultName": "Left click",  "defaultActionType": "default", "configurable": False},
    {"controlId": "0x0051", "buttonIndex": 1, "defaultName": "Right click", "defaultActionType": "default", "configurable": False},
]


def slugify(name):
    return re.sub(r'[^a-z0-9]+', '-', name.lower()).strip('-')


def parse_cid_from_slot_id(slot_id):
    """Extract CID from slot ID suffix like 'mx-vertical-eb020_c82' -> 0x0052.

    Slot IDs encode the CID as a decimal suffix after `_c`. Returns None for
    slots that don't match this pattern (e.g. thumb-wheel / gesture zones).
    """
    match = re.search(r'_c(\d+)$', slot_id)
    if match:
        return int(match.group(1))
    return None


# Virtual CID used for the thumb wheel. Matches the synthetic entry the MX
# Master descriptors use so thumb-wheel notifications have a controls slot
# to bind to — the hardware doesn't expose a real CID for it.
THUMBWHEEL_CID = 0x0000


def looks_like_thumbwheel_slot(slot_id, slot_name):
    return (
        slot_name == "SLOT_NAME_THUMBWHEEL"
        or 'thumb_wheel' in (slot_id or '').lower()
    )


def load_options_device_db(main_dir):
    """Load all mice from the Options+ device database."""
    mice = {}
    for f in sorted(glob.glob(os.path.join(main_dir, 'data/devices/devices*.json'))):
        try:
            data = json.load(open(f, encoding='utf-8-sig'))
        except Exception:
            continue
        devs = data.get('devices', []) if isinstance(data, dict) else []
        for d in devs:
            if d.get('type') != 'MOUSE':
                continue
            depot = d.get('depot', '')
            if not depot:
                continue
            pids = set()
            for mode in d.get('modes', []):
                for iface in mode.get('interfaces', []):
                    iid = iface.get('id', '')
                    if '046d' in iid.lower():
                        pid_hex = iid.lower().split('_')[-1] if '_' in iid else ''
                        if pid_hex:
                            pids.add(f"0x{pid_hex}")
            if pids:
                if depot not in mice:
                    mice[depot] = {
                        'name': d.get('displayName', depot),
                        'pids': set(),
                        'depot': depot,
                        'capabilities': d.get('capabilities', {}),
                    }
                mice[depot]['pids'].update(pids)
                # Prefer the first non-empty capabilities object we see
                if not mice[depot]['capabilities'] and d.get('capabilities'):
                    mice[depot]['capabilities'] = d.get('capabilities', {})
    return mice


def features_from_capabilities(caps):
    """Map Options+ capabilities object to our FeatureSupport bool dict."""
    swc = caps.get('scroll_wheel_capabilities', {})
    smooth = swc.get('smooth_scroll', {})
    if isinstance(smooth, bool):
        smooth_on = smooth
    else:
        smooth_on = bool(smooth.get('win') or smooth.get('mac'))

    has_adjustable_dpi = (
        bool(caps.get('hasHighResolutionSensor'))
        or 'highResolutionSensorInfo' in caps
        or bool(caps.get('pointerSpeed'))
    )
    has_programmable = bool(caps.get('specialKeys', {}).get('programmable'))

    return {
        "battery": bool(caps.get('hasBatteryStatus') or caps.get('unified_battery')),
        "adjustableDpi": has_adjustable_dpi,
        "smartShift": bool(swc.get('smartshift')),
        "hiResWheel": bool(swc.get('high_resolution')),
        # mouseThumbWheelOverride only appears on devices with a physical
        # thumb wheel (the MX Master line). virtual_thumbwheel is a different
        # software-emulated concept that doesn't map to our feature flag.
        "thumbWheel": 'mouseThumbWheelOverride' in caps,
        "reprogControls": has_programmable,
        "gestureV2": False,  # not represented in Options+ device db
        "smoothScroll": smooth_on,
        "hapticFeedback": False,  # not represented in Options+ device db
    }


def dpi_from_capabilities(caps):
    """Read DPI range from Options+ highResolutionSensorInfo, else defaults."""
    info = caps.get('highResolutionSensorInfo')
    if info:
        # Use SensorOn range (high-res sensor mode) for max possible DPI
        return {
            "min": info.get('minDpiValueSensorOn', 200),
            "max": info.get('maxDpiValueSensorOn', 4000),
            "step": info.get('stepsSensorOn', 50),
        }
    return {"min": 200, "max": 4000, "step": 50}


def _marker_to_pct(marker):
    """Options+ markers encode position as percentages in [0, 100]."""
    x_pct = round(marker.get('x', 0) / 100.0, 3)
    y_pct = round(marker.get('y', 0) / 100.0, 3)
    return max(0.0, min(1.0, x_pct)), max(0.0, min(1.0, y_pct))


def _canonical_sort_key(entry):
    """Sort controls by CID ascending, but push the synthetic thumbwheel
    (CID 0x0000) to the very end.

    The canonical button order matters: ButtonsPage.qml hardcodes
    `buttonId === 7` to mean thumbwheel, so the synthetic entry needs to
    land last. For the MX Master line this sort also happens to produce
    Middle(0x52) / Back(0x53) / Forward(0x56) / Gesture(0xC3) / Shift(0xC4),
    matching the hand-written descriptors — so any profiles persisted
    against the shipped ordering keep working.
    """
    cid = int(entry['_cid'])
    if cid == THUMBWHEEL_CID:
        return (1, 0)
    return (0, cid)


def parse_metadata_hotspots(metadata_path, image_key='device_buttons_image'):
    """Parse button hotspot positions from Options+ core_metadata.json.

    Markers are percentages in [0, 100] relative to the device image, not
    pixel coordinates — the `origin` width/height only describe the source
    image resolution. Controls are returned in canonical CID order with
    the synthetic thumbwheel last; buttonIndex is assigned after sorting.
    """
    try:
        meta = json.load(open(metadata_path))
    except Exception:
        return []

    raw = []

    for img in meta.get('images', []):
        if img.get('key') != image_key:
            continue

        for assignment in img.get('assignments', []):
            slot_id = assignment.get('slotId', '')
            slot_name = assignment.get('slotName', '')
            marker = assignment.get('marker', {})

            x_pct, y_pct = _marker_to_pct(marker)

            cid = parse_cid_from_slot_id(slot_id)
            is_thumb = looks_like_thumbwheel_slot(slot_id, slot_name)

            if cid is None and not is_thumb:
                # Slot has no HID++ CID and isn't a thumbwheel — nothing
                # to emit. Scroll / pointer settings zones hit this
                # branch.
                continue

            if cid is None and is_thumb:
                cid = THUMBWHEEL_CID

            name_info = SLOT_NAME_MAP.get(
                slot_name,
                (slot_name, "default", True),
            )

            raw.append({
                '_cid': cid,
                '_name': name_info[0],
                '_action': name_info[1],
                '_configurable': name_info[2],
                '_x': x_pct,
                '_y': y_pct,
            })
        break

    raw.sort(key=_canonical_sort_key)

    controls = []
    idx = len(DEFAULT_CONTROLS)
    for entry in raw:
        cid_hex = f"0x{entry['_cid']:04X}"
        controls.append({
            'control': {
                # Field names match ControlDescriptor JSON keys parsed by
                # JsonDevice::parseControls() in src/core/devices/JsonDevice.cpp
                # (controlId / buttonIndex / defaultName / defaultActionType).
                "controlId": cid_hex,
                "buttonIndex": idx,
                "defaultName": entry['_name'],
                "defaultActionType": entry['_action'],
                "configurable": entry['_configurable'],
            },
            # Field names must match JsonDevice::parseHotspots()
            # (buttonIndex / xPct / yPct / labelOffsetYPct).
            'hotspot': {
                "buttonIndex": idx,
                "xPct": entry['_x'],
                "yPct": entry['_y'],
                "side": "right" if entry['_x'] > 0.5 else "left",
                "labelOffsetYPct": 0.0,
            },
        })
        idx += 1

    return controls


def parse_scroll_hotspots(metadata_path, after_button_idx):
    """Parse scroll/pointer hotspots from `device_point_scroll_image`.

    Scroll hotspots use negative buttonIndex values (-1, -2, -3…) by
    convention, matching the hand-written descriptors. The image contains
    entries for the scroll wheel, thumb wheel zone, and pointer-settings
    handle; we emit them in metadata order.
    """
    try:
        meta = json.load(open(metadata_path))
    except Exception:
        return []

    hotspots = []
    slot = -1

    for img in meta.get('images', []):
        if img.get('key') != 'device_point_scroll_image':
            continue
        for assignment in img.get('assignments', []):
            marker = assignment.get('marker', {})
            x_pct, y_pct = _marker_to_pct(marker)
            hotspots.append({
                "buttonIndex": slot,
                "xPct": x_pct,
                "yPct": y_pct,
                "side": "right" if x_pct > 0.5 else "left",
                "labelOffsetYPct": 0.0,
            })
            slot -= 1
        break

    return hotspots


def build_descriptor(mouse_info, controls_data, scroll_hotspots,
                     has_side_image, has_back_image):
    """Build a complete descriptor.json for a device."""
    all_controls = list(DEFAULT_CONTROLS)
    button_hotspots = []

    for cd in controls_data:
        all_controls.append(cd['control'])
        button_hotspots.append(cd['hotspot'])

    images = {"front": "front.png"}
    if has_side_image:
        images["side"] = "side.png"
    if has_back_image:
        images["back"] = "back.png"

    caps = mouse_info.get('capabilities', {})

    # Descriptors with real hotspots and a front image meet strict validation
    # requirements, so promote them out of "placeholder" status — they're
    # community-verified by virtue of coming from Logitech's own database.
    status = "community-verified" if button_hotspots else "placeholder"

    descriptor = {
        "name": mouse_info['name'],
        "status": status,
        "version": 1,
        "productIds": sorted(mouse_info['pids']),
        "features": features_from_capabilities(caps),
        "dpi": dpi_from_capabilities(caps),
        "controls": all_controls,
        "hotspots": {
            "buttons": button_hotspots,
            "scroll": scroll_hotspots,
        },
        "images": images,
        "easySwitchSlots": [],
        "defaultGestures": {},
    }

    return descriptor


def main():
    parser = argparse.ArgumentParser(description='Generate descriptors from Options+ data')
    parser.add_argument('--devices-dir', required=True, help='Extracted depot devices directory')
    parser.add_argument('--main-dir', required=True, help='Extracted main depot directory')
    parser.add_argument('--output-dir', required=True, help='Output directory for descriptors')
    parser.add_argument('--dry-run', action='store_true')
    parser.add_argument('--skip-existing', action='store_true', default=True)
    args = parser.parse_args()

    mice = load_options_device_db(args.main_dir)
    print(f"Found {len(mice)} mice in Options+ database")

    generated = 0
    skipped = 0
    no_images = 0

    for depot_name, mouse_info in sorted(mice.items(), key=lambda x: x[1]['name']):
        slug = slugify(mouse_info['name'])
        depot_dir = os.path.join(args.devices_dir, depot_name)
        out_dir = os.path.join(args.output_dir, slug)

        if args.skip_existing and os.path.exists(os.path.join(out_dir, 'descriptor.json')):
            skipped += 1
            continue

        # Handle both naming patterns: front.png (old) / front_core.png (new)
        front_img = os.path.join(depot_dir, 'front.png')
        if not os.path.exists(front_img):
            front_img = os.path.join(depot_dir, 'front_core.png')
        if not os.path.exists(front_img):
            no_images += 1
            continue

        side_img = os.path.join(depot_dir, 'side.png')
        if not os.path.exists(side_img):
            side_img = os.path.join(depot_dir, 'side_core.png')
        has_side = os.path.exists(side_img)

        # Back/bottom image (shows Easy-Switch indicators on flip side)
        back_img = os.path.join(depot_dir, 'back.png')
        if not os.path.exists(back_img):
            back_img = os.path.join(depot_dir, 'back_core.png')
        if not os.path.exists(back_img):
            back_img = os.path.join(depot_dir, 'bottom_core.png')
        if not os.path.exists(back_img):
            back_img = os.path.join(depot_dir, 'bottom.png')
        has_back = os.path.exists(back_img)

        # Handle both metadata patterns: metadata.json (old) / core_metadata.json (new)
        metadata_path = os.path.join(depot_dir, 'metadata.json')
        if not os.path.exists(metadata_path):
            metadata_path = os.path.join(depot_dir, 'core_metadata.json')
        controls_data = parse_metadata_hotspots(metadata_path)
        scroll_hotspots = parse_scroll_hotspots(metadata_path, after_button_idx=len(controls_data))

        descriptor = build_descriptor(mouse_info, controls_data, scroll_hotspots,
                                      has_side, has_back)

        if args.dry_run:
            pids = ', '.join(mouse_info['pids'])
            imgs = "front"
            if has_side: imgs += "+side"
            if has_back: imgs += "+back"
            print(f"  {mouse_info['name']:40s} -> {slug}/ "
                  f"({len(descriptor['controls'])} controls, "
                  f"{len(descriptor['hotspots']['scroll'])} scroll, imgs={imgs})")
            generated += 1
            continue

        os.makedirs(out_dir, exist_ok=True)
        with open(os.path.join(out_dir, 'descriptor.json'), 'w') as f:
            json.dump(descriptor, f, indent=2)
            f.write('\n')
        shutil.copy2(front_img, os.path.join(out_dir, 'front.png'))
        if has_side:
            shutil.copy2(side_img, os.path.join(out_dir, 'side.png'))
        if has_back:
            shutil.copy2(back_img, os.path.join(out_dir, 'back.png'))

        print(f"  {slug}: {len(descriptor['controls'])} controls, "
              f"{len(descriptor['hotspots']['buttons'])} hotspots, "
              f"{len(descriptor['hotspots']['scroll'])} scroll")
        generated += 1

    print(f"\nDone: {generated} generated, {skipped} skipped, {no_images} missing images")


if __name__ == '__main__':
    main()
