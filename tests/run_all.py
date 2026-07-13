#!/usr/bin/env python3
"""
Master test runner with coverage and reporting.
Usage: python tests/run_all.py
"""

import subprocess
import sys
import argparse
from pathlib import Path


def build_server():
    """Compile server binary."""
    print("[BUILD] Compiling server...")
    result = subprocess.run(["make"], capture_output=True, text=True)
    if result.returncode != 0:
        print(f"[ERROR] Build failed:\n{result.stderr}")
        return False
    print("[✓] Build successful")
    return True


def run_tests(cov=False, verbose=False):
    """Execute pytest suite."""
    print(f"\n[TESTS] Running {'verbose ' if verbose else ''}test suite{' with coverage' if cov else ''}...")
    
    cmd = [sys.executable, "-m", "pytest", "tests/", "-v" if verbose else "-q"]
    if cov:
        cmd.extend([
            "--cov=src",
            "--cov-report=html:htmlcov",
            "--cov-report=xml:coverage.xml",
            "--cov-report=term-missing",
        ])
    
    result = subprocess.run(cmd, capture_output=cov, text=True)
    
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)
    
    return result.returncode == 0


def generate_badge():
    """Create CI badge README snippet."""
    print("\n[INFO] Add this to your README:")
    print("""
### CI Status
![Tests](https://github.com/YOUR_USER/YOUR_REPO/actions/workflows/ci.yml/badge.svg)
""")


def main():
    parser = argparse.ArgumentParser(description="IRC Server Test Suite")
    parser.add_argument("--build", action="store_true", help="Build server first")
    parser.add_argument("--cov", action="store_true", help="Enable coverage")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
    args = parser.parse_args()
    
    if args.build:
        if not build_server():
            sys.exit(1)
    
    success = run_tests(cov=args.cov, verbose=args.verbose)
    
    if success:
        print("\n[✓] All tests passed!")
        generate_badge()
    else:
        print("\n[✗] Some tests failed")
        sys.exit(1)


if __name__ == "__main__":
    main()