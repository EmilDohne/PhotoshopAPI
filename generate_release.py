'''
This python script generates our release artifacts, this would be upgraded to a github action in the future for multiple platform support
'''
import subprocess
import os
import shutil


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
    raise FileNotFoundError(f"Could not find file {file_name}")


def build_cmake(out_path_rel: str = "build_tmp") -> str:
    '''
    Builds the PhotoshopAPI with CMake in Release mode.

    :returns: the full path to the build files 
    '''
    base_path = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(base_path, out_path_rel)
    if not os.path.exists(build_dir):
        os.makedirs(build_dir, exist_ok=True)

    # Run cmake in our temporary dir
    subprocess.run(["cmake", "-B", build_dir, "-DCMAKE_BUILD_TYPE=Release", "-DPSAPI_BUILD_DOCS=OFF", "-DPSAPI_BUILD_BENCHMARKS=OFF", "-DPSAPI_BUILD_TESTS=OFF"], cwd=base_path)
    # run the build process
    subprocess.run(["cmake", "--build", build_dir, "--config", "Release"])

    return build_dir


def copy_headers(out_path_rel: str = "build_tmp/headers") -> str:
    '''
    Copy all the header files from the PhotoshopAPI to a tmp directory
    where it can be collected from after

    :returns: the full path to the headers
    '''
    base_path = os.path.dirname(os.path.abspath(__file__))
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
    Extract the relevant files from the build_dir and header_dir after which we delete both of those directories 
    '''
    if not os.path.exists(out_path):
        os.makedirs(out_path, exist_ok=True)
    lib_path = os.path.abspath(os.path.join(out_path, "lib"))
    if not os.path.exists(lib_path):
        os.makedirs(lib_path, exist_ok=True)
    # Copy over our PhotoshopAPI.lib, zlibstatic-ng.lib and libblosc2.lib
    _find_file_and_copy("PhotoshopAPI.lib",  build_dir, lib_path)
    _find_file_and_copy("zlibstatic-ng.lib", build_dir, lib_path)
    _find_file_and_copy("libblosc2.lib",     build_dir, lib_path)

    # Copy over the headers which are already pre-sorted
    shutil.copytree(header_dir, os.path.join(out_path, "include"), dirs_exist_ok=True)

    # Clean up the temporary directory
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    if os.path.exists(header_dir):
        shutil.rmtree(header_dir)


if __name__ == "__main__":
    build_dir = build_cmake()
    header_dir = copy_headers()
    generate_clean_release("release", build_dir, header_dir)