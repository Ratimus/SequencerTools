import os
import json
import re

LIB_ROOT = "lib"
DEFAULT_VERSION = "1.0.0"


def get_includes_from_file(filepath):
    includes = []
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            match = re.match(r'#include\s*[<"]([^">]+)[">]', line)
            if match:
                includes.append(match.group(1))
    return includes


def create_library_json(lib_path, name, includes):
    library_json_path = os.path.join(lib_path, "library.json")
    internal_deps = sorted(
        {
            inc.split("/")[0]
            for inc in includes
            if "/" in inc
            and inc.startswith(tuple(os.listdir(LIB_ROOT)))
            and inc.split("/")[0] != name
        }
    )

    if os.path.exists(library_json_path):
        with open(library_json_path, "r") as f:
            try:
                data = json.load(f)
            except json.JSONDecodeError:
                print(f"[ERROR] Couldn't parse existing library.json in {name}")
                return

        old_deps = sorted(data.get("dependencies", []))
        if old_deps != internal_deps:
            data["dependencies"] = internal_deps
            with open(library_json_path, "w") as f:
                json.dump(data, f, indent=2)
            print(f"[UPDATE] Updated dependencies in {library_json_path}")
        else:
            print(f"[SKIP] {name}: dependencies unchanged")
    else:
        data = {
            "name": name,
            "version": DEFAULT_VERSION,
            "description": f"{name} component of the SequencerTools library",
            "build": {"srcDir": "src", "includeDir": "include"},
            "dependencies": internal_deps,
        }
        with open(library_json_path, "w") as f:
            json.dump(data, f, indent=2)
        print(f"[WRITE] Created library.json in {lib_path}")


def main():
    lib_names = set(os.listdir(LIB_ROOT))
    for entry in lib_names:
        sub_path = os.path.join(LIB_ROOT, entry)
        if not os.path.isdir(sub_path):
            continue

        includes = []
        # Scan include dir headers
        include_dir = os.path.join(sub_path, "include")
        if os.path.isdir(include_dir):
            for root, _, files in os.walk(include_dir):
                for file in files:
                    if file.endswith((".h", ".hpp")):
                        filepath = os.path.join(root, file)
                        includes += get_includes_from_file(filepath)

        # Scan src dir sources
        src_dir = os.path.join(sub_path, "src")
        if os.path.isdir(src_dir):
            for root, _, files in os.walk(src_dir):
                for file in files:
                    if file.endswith((".h", ".hpp", ".c", ".cpp")):
                        filepath = os.path.join(root, file)
                        includes += get_includes_from_file(filepath)

        # Filter includes to internal deps
        internal_deps = sorted(
            {
                inc.split("/")[0]
                for inc in includes
                if "/" in inc
                and inc.split("/")[0] in lib_names
                and inc.split("/")[0] != entry
            }
        )

        create_library_json(sub_path, entry, internal_deps)


if __name__ == "__main__":
    main()
