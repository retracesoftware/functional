# meson.build - Project configuration + shared build logic

project(
  '_retracesoftware_functional',
  'cpp',
  version: run_command('python', '-m', 'setuptools_scm', check: true, capture: true).stdout().strip(),
  default_options: ['cpp_std=c++20', 'strip=false'])

# Project-specific configuration
rs_extension_name = '_retracesoftware_functional'
rs_cpp_source_dir = 'cpp'
rs_ext_install_subdir = ''
rs_py_sources = ['src/retracesoftware/functional/__init__.py', 'src/retracesoftware/functional/_pure.py']
rs_py_install_subdir = 'retracesoftware/functional'

# Include shared build logic (sets up py, builds release/debug modules)
subdir('common-headers/meson')
