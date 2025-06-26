import os
import json

LIB_ROOT = "lib"
DEFAULT_VERSION = "1.0.0"


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
    for entry in os.listdir(LIB_ROOT):
        sub_path = os.path.join(LIB_ROOT, entry)
        if os.path.isdir(sub_path):
            has_src = os.path.isdir(os.path.join(sub_path, "src"))
            has_include = os.path.isdir(os.path.join(sub_path, "include"))
            if has_src or has_include:
                # Collect header includes for this sublib
                includes = []
                include_dir = os.path.join(sub_path, "include")
                if os.path.isdir(include_dir):
                    for root, _, files in os.walk(include_dir):
                        for file in files:
                            if file.endswith(".h") or file.endswith(".hpp"):
                                rel_path = os.path.relpath(
                                    os.path.join(root, file), start=include_dir
                                )
                                includes.append(rel_path.replace("\\", "/"))

                create_library_json(sub_path, entry, includes)


if __name__ == "__main__":
    main()
