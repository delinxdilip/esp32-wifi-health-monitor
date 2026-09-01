import os
from pathlib import Path

Import("env")


# ============================================================
# LOAD .ENV FILE
# ============================================================

project_dir = Path(env.get("PROJECT_DIR"))
env_file = project_dir / ".env"

if not env_file.exists():
    raise SystemExit(
        "[ENV] ERROR: .env file not found."
    )

print("[ENV] Loading Firebase configuration...")


values = {}


with open(env_file, "r") as file:

    for line in file:

        line = line.strip()

        # Ignore empty lines
        if not line:
            continue

        # Ignore comments
        if line.startswith("#"):
            continue

        # Ignore malformed lines
        if "=" not in line:
            continue

        key, value = line.split("=", 1)

        key = key.strip()
        value = value.strip()

        # Remove surrounding quotes if present
        if len(value) >= 2:

            if (
                value.startswith('"')
                and value.endswith('"')
            ):
                value = value[1:-1]

            elif (
                value.startswith("'")
                and value.endswith("'")
            ):
                value = value[1:-1]

        values[key] = value


# ============================================================
# REQUIRED VARIABLES
# ============================================================

required = [
    "FIREBASE_API_KEY",
    "FIREBASE_DATABASE_URL",
    "FIREBASE_USER_EMAIL",
    "FIREBASE_USER_PASSWORD",
]


for key in required:

    if key not in values:

        raise SystemExit(
            f"[ENV] ERROR: Missing {key}"
        )

    if not values[key]:

        raise SystemExit(
            f"[ENV] ERROR: {key} is empty"
        )


# ============================================================
# ESCAPE VALUES FOR C++ STRING LITERALS
# ============================================================

def escape_cpp_string(value):

    value = value.replace("\\", "\\\\")
    value = value.replace('"', '\\"')

    return value


# ============================================================
# INJECT FIREBASE VALUES
# ============================================================

for key in required:

    value = escape_cpp_string(
        values[key]
    )

    env.Append(
        CPPDEFINES=[
            f'{key}=\\"{value}\\"'
        ]
    )


print("[ENV] Firebase configuration loaded.")
print("[ENV] API key: loaded")
print("[ENV] Database URL: loaded")
print("[ENV] Email: loaded")
print("[ENV] Password: loaded")