"""Tests for scripts/optionsplus_extractor/. See spec
docs/superpowers/specs/2026-04-13-optionsplus-extractor-design.md."""

import importlib


def test_package_imports():
    mod = importlib.import_module("optionsplus_extractor")
    assert mod.__doc__ is not None


def test_all_submodules_import():
    for name in ("sources", "capabilities", "slots", "canonicalize",
                 "descriptor", "validate", "cli"):
        importlib.import_module(f"optionsplus_extractor.{name}")
