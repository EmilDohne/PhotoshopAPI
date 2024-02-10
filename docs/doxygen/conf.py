# The sphinx template and setup was mostly adopted from openimageios docs to mimic their styling
import subprocess, os

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
    input_dir = '../../PhotoshopAPI'
    output_dir = 'build'
    configureDoxyfile(input_dir, output_dir)
    subprocess.call('doxygen', shell=True)
    breathe_projects['PhotoshopAPI'] = output_dir + '/build'


# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'PhotoshopAPI'
copyright = '2024, Emil Dohne'
author = 'Emil Dohne'
release = '2024'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['breathe']


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_theme_options = {
    "dark_css_variables": {
        "color-api-background" : "#212121",
        "color-api-background-hover" : "#313131",
        "color-api-keyword" : "#0000FF"
    },
}


# -- Breathe configuration ---------------------------------------------------
breathe_default_project = "PhotoshopAPI"