import re
import os
import json
import glob
import argparse

# Matches the full macro invocation, e.g.:
#   ESPFWK_WEB_ERR("CONFIG_UPDATED")
#   ESPFWK_WEB_ERR(success > 0 ? "TEMPLATE_STORED" : "TEMPLATE_FAILED")
# Assumes no nested parentheses inside the macro arguments (true for all
# current call sites in gravitymon and espframework).
CALL_REGEX = re.compile(r'ESPFWK_WEB_ERR\(([^)]*)\)', re.DOTALL)
STRING_REGEX = re.compile(r'"([A-Za-z0-9_]+)"')


def find_installed_espframework(project_root):
    # Different PlatformIO environments can pin different espframework
    # versions, so scan every installed copy rather than just the first.
    return glob.glob(os.path.join(project_root, '.pio', 'libdeps', '*', 'espframework'))


def extract_keys_to_template(scan_paths, output_file):
    print("--> Scanning firmware and libraries for UI message codes...")

    found_codes = set()

    for path in scan_paths:
        if not os.path.exists(path):
            print(f"warning: path not found, skipping: {path}")
            continue

        print(f"   Scanning path: {path}")
        for root, _, files in os.walk(path):
            for file in files:
                if file.endswith(('.cpp', '.c', '.h', '.hpp')):
                    file_path = os.path.join(root, file)
                    try:
                        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                            content = f.read()
                    except OSError as e:
                        print(f"warning: could not read file {file_path}: {e}")
                        continue

                    for call_args in CALL_REGEX.findall(content):
                        found_codes.update(STRING_REGEX.findall(call_args))

    print(f"--> Found {len(found_codes)} unique UI message codes.")

    template_data = {code: "" for code in sorted(found_codes)}

    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir, exist_ok=True)

    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(template_data, f, indent=2, ensure_ascii=False)
        f.write('\n')

    print(f"Successfully created template file at: {output_file}")


if __name__ == "__main__":
    default_scan = ['./src', './lib'] + find_installed_espframework('.')

    parser = argparse.ArgumentParser(
        description="Extract ESPFWK_WEB_ERR() message codes from firmware source into a vue-i18n locale template."
    )
    parser.add_argument(
        '--scan',
        nargs='+',
        default=default_scan,
        help='Space-separated list of directories to scan for C/C++ files (default: %(default)s)'
    )
    parser.add_argument(
        '--output',
        default='./ui/src/locales/template.json',
        help='Path where the template JSON file should be generated (default: %(default)s)'
    )

    args = parser.parse_args()
    extract_keys_to_template(args.scan, args.output)
