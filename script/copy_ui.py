#!/usr/bin/env python3
"""
Copy UI assets from ui/dist to the html directory.
This script runs as a pre-build step in PlatformIO to ensure assets are in place.
"""

import shutil
import sys
from pathlib import Path

def copy_ui_assets():
    """Copy UI assets from ui/dist to html/."""
    project_root = Path(__file__).parent.parent
    ui_dir = project_root / 'ui'
    html_dir = project_root / 'html'
    
    # Copy assets directly
    print("Copying UI assets to html/...")
    try:
        css_src = ui_dir / 'dist' / 'assets' / 'style.css.gz'
        js_src = ui_dir / 'dist' / 'assets' / 'index.js.gz'
        
        if not css_src.exists():
            print(f"Error: {css_src} not found", file=sys.stderr)
            print("Run 'cd ui && npm run build' first", file=sys.stderr)
            return False
        if not js_src.exists():
            print(f"Error: {js_src} not found", file=sys.stderr)
            print("Run 'cd ui && npm run build' first", file=sys.stderr)
            return False
        
        shutil.copy2(css_src, html_dir / 'app.css.gz')
        shutil.copy2(js_src, html_dir / 'app.js.gz')
        print(f"Copied app.css.gz and app.js.gz to html/")
    except Exception as e:
        print(f"Error: Failed to copy UI assets: {e}", file=sys.stderr)
        return False
    
    return True

if __name__ == '__main__':
    success = copy_ui_assets()
    sys.exit(0 if success else 1)
