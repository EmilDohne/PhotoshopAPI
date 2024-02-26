# The sphinx template and setup was mostly adopted from openimageios docs to mimic their styling
import subprocess, os, sys

# This is for local development to properly include the paths
sys.path.insert(0, os.path.abspath("../../bin-int/PhotoshopAPI/x64-release/python"))
sys.path.insert(0, os.path.abspath("../../bin-int/PhotoshopAPI/x64-debug/python"))


def buildPhotoshopAPI(build_dir: str) -> str:
    '''
    Build the photoshopAPI with python bindings so we can display them in our docs
    '''
    base_path = os.path.join(os.path.abspath(__file__), "../.." )
    build_dir = os.path.join(os.path.abspath(__file__), build_dir)
    print(f"Building cmake on dir '{base_path}' into dir '{build_dir}'")
    
    subprocess.run(["cmake", "-B", build_dir, "-DCMAKE_BUILD_TYPE=Release", "-DPSAPI_BUILD_DOCS=OFF", "-DPSAPI_BUILD_BENCHMARKS=OFF", "-DPSAPI_BUILD_TESTS=OFF"], cwd=base_path)
    # run the build process
    subprocess.run(["cmake", "--build", build_dir, "--config", "Release"])
    return build_dir


def configureDoxyfile(input_dir, output_dir):
    with open('Doxyfile.in', 'r') as file :
        filedata = file.read()

    filedata = filedata.replace('@DOXYGEN_INPUT_DIR@', input_dir)
    filedata = filedata.replace('@DOXYGEN_OUTPUT_DIR@', output_dir)

    with open('Doxyfile', 'w') as file:
        file.write(filedata)

# Check if we're running on Read the Docs' servers
read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

breathe_projects = {}

if read_the_docs_build:
    print("Detected we are running in Readthedocs")
    input_dir = '../../PhotoshopAPI'
    output_dir = 'build'
    configureDoxyfile(input_dir, output_dir)
    subprocess.call('doxygen', shell=True)
    breathe_projects['PhotoshopAPI'] = output_dir + '/build'
    
    # Build the PSAPI for the python bindings and append the path so we can resolve it
    bin_path = buildPhotoshopAPI("bin")
    py_module_path = os.path.join(bin_path, "PhotoshopAPI/x64-release/python")
    sys.path.insert(0, py_module_path)
    


# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'PhotoshopAPI'
copyright = '2024, Emil Dohne'
author = 'Emil Dohne'
release = '2024'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
    'sphinx.ext.autodoc',
    'sphinx_inline_tabs',
    'numpydoc'
    ]

numpydoc_show_class_members = False
autodoc_typehints = "none"


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_theme_options = {
    "dark_css_variables": {
        "color-api-background" : "#212121",
        "color-api-background-hover" : "#313131"
    },
}

autodoc_member_order = 'bysource'

# -- Breathe configuration ---------------------------------------------------
breathe_default_project = "PhotoshopAPI"