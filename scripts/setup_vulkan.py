import os
import tempfile
import tarfile
import pathlib
import sys
import urllib.request
from dotenv import load_dotenv

load_dotenv(".env")

TEMP_DIR = tempfile.gettempdir()
VULKAN_SDK_VERSION = os.environ.get("VULKAN_SDK_VERSION")
TARGET = os.environ.get("TARGET", sys.platform).upper()

SUPPORTED_TARGETS = ["LINUX"]

def get_link(version):
    if (TARGET == "LINUX"):
        return f"https://sdk.lunarg.com/sdk/download/{version}/linux/vulkansdk-linux-x86_64-{version}.tar.xz"
    
    return ""

def get_download_file_path(ext):
    return os.path.join(TEMP_DIR, f"vulkan_sdk{ext}");

def get_output_path():
    if (TARGET == "LINUX"):
        return os.path.abspath(os.path.join("vendor", "linux", "vulkan_sdk"))

    return ""

def dir_exists(dir):
    return os.path.isdir(dir)

def setup(path):
    if (TARGET == "LINUX"):
        setup_path = os.path.join(path, VULKAN_SDK_VERSION or "", "setup-env.sh")
        print(f"export {setup_path}")

def handle_file():
    extract_dir = get_output_path()

    if dir_exists(extract_dir):
        print(f"Vulkan SDK path already exists, nothing to do.")
        exit(0)

    link = get_link(VULKAN_SDK_VERSION)
    print(link)
    p = pathlib.Path(link)
    ext = "".join(p.suffixes)
    downloaded_file_path = get_download_file_path(ext)

    if dir_exists(downloaded_file_path):
        print(f"No download needed, found Vulkan sdk file in {extract_dir}")
    else:
        print("Downloading Vulkan SDK")
        urllib.request.urlretrieve(link, downloaded_file_path)

    if (TARGET == "LINUX"):
        print(f"Extracting {extract_dir}")
        with tarfile.open(downloaded_file_path, "r:xz") as tar:
            tar.extractall(path=extract_dir)

if __name__ == "__main__":
    if VULKAN_SDK_VERSION is None:
        print("VULKAN_SDK_VERSION not defined", file=sys.stderr)
        exit(1)

    if TARGET is None:
        print("TARGET not defined", file=sys.stderr)
        exit(1)

    if not TARGET in SUPPORTED_TARGETS:
        print(f"Invalid TARGET env variable provided: \"{TARGET}\"", file=sys.stderr)
        exit(1)

    handle_file()
