'''
This python script generates our release artifacts, this is called by the cmake-build.yml github action and generates release artifacts that are
attached to the build.

Expects a --build-dir argument which specifies where the build files are to differentiate between release and debug builds. This path is relative
to the project source dir

A sample call of this script would look like this (run from the PhotoshopAPI dir):

py scripts/generate_release.py --build-dir bin-int/PhotoshopAPI/x64-release
'''
import os
import sys
import shutil
import argparse


def _find_file_and_copy(file_name: str, base_dir: str, out_dir: str) -> str:
    '''
    Find a file recursively given a base directory and copy it over to the given out directory

    :returns: the output path to the file
    '''
    for root, _, files in os.walk(base_dir):
        if file_name in files:
            source_file = os.path.join(root, file_name)
            output_file = os.path.join(out_dir, file_name)
            shutil.copy(source_file, output_file)
            print(f"File '{file_name}' copied to '{out_dir}'")
            return output_file
    raise FileNotFoundError(f"Could not find file '{file_name}' in dir '{base_dir}'")


def copy_headers(out_path_rel: str = "build_tmp/headers") -> str:
    '''
    Copy all the header files from the PhotoshopAPI to a tmp directory
    where it can be collected from after

    :returns: the full path to the extracted headers
    '''
    base_path = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    header_dir = os.path.join(base_path, out_path_rel)
    if not os.path.exists(header_dir):
        os.makedirs(header_dir, exist_ok=True)

    photoshop_api_path = os.path.join(base_path, "PhotoshopAPI", "src")

    # Copy all the header files from the src dir
    for root, dirs, files in os.walk(photoshop_api_path):
        # Create corresponding subdirectories in the output directory
        relative_path = os.path.relpath(root, photoshop_api_path)
        output_subdir = os.path.join(header_dir, relative_path)
        os.makedirs(output_subdir, exist_ok=True)
        
        # Iterate through files in the current directory
        for file in files:
            if file.endswith(".h"):  # Filter header files
                # Copy header file to the corresponding subdirectory in the output directory
                source_file = os.path.join(root, file)
                output_file = os.path.join(output_subdir, file)
                shutil.copyfile(source_file, output_file)
    
    # Now copy over the include/PhotoshopAPI.h file separately
    shutil.copyfile(os.path.join(base_path, "PhotoshopAPI", "include", "PhotoshopAPI.h"), os.path.join(header_dir, "PhotoshopAPI.h"))

    return header_dir


def generate_clean_release(out_path: str, build_dir: str, header_dir: str) -> None:
    '''
    Extract the relevant files from the build_dir and header_dir and sort them into a 
    "release" folder with "lib" and "include" subfolders. build_dir is relative to
    the root directory of the PhotoshopAPI. Same goes for header_dir
    '''
    build_dir = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", build_dir))

    if not os.path.exists(out_path):
        os.makedirs(out_path, exist_ok=True)
    lib_path = os.path.abspath(os.path.join(out_path, "lib"))
    if not os.path.exists(lib_path):
        os.makedirs(lib_path, exist_ok=True)


    # Copy over our PhotoshopAPI.lib, zlibstatic-ng.lib and libblosc2.lib
        
    psapi_platform_mapping = {
        "win32": "PhotoshopAPI.lib",
        "linux": "libPhotoshopAPI.a",
        "darwin": "libPhotoshopAPI.a"
    }

    zlib_platform_mapping = {
        "win32": "zlibstatic-ng.lib",
        "linux": "libz-ng.a",
        "darwin": "libz-ng.a"
    }

    blosc2_platform_mapping = {
        "win32": "libblosc2.lib",
        "linux": "libblosc2.a",
        "darwin": "libblosc2.a"
    }

    _find_file_and_copy(psapi_platform_mapping.get(sys.platform, psapi_platform_mapping["linux"]),   build_dir, lib_path)
    _find_file_and_copy(zlib_platform_mapping.get(sys.platform, zlib_platform_mapping["linux"]),     build_dir, lib_path)
    _find_file_and_copy(blosc2_platform_mapping.get(sys.platform, blosc2_platform_mapping["linux"]), build_dir, lib_path)

    # Copy over the headers which are already pre-sorted
    shutil.copytree(header_dir, os.path.join(out_path, "include"), dirs_exist_ok=True)

    # Clean up the temporary directory
    if os.path.exists(header_dir):
        shutil.rmtree(header_dir)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Copy headers and library files to a clean release folder .')
    parser.add_argument('--build-dir', required=True, help='Path to the build binaries.')
    args = parser.parse_args()

    header_dir = copy_headers()
    generate_clean_release("release", args.build_dir, header_dir)