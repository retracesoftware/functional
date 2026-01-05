# meson.build
# Define a custom option for version (overridable via -Dversion=...)

project(
  'retracesoftware_functional',
  'cpp',
  version: '0.3.4',
  #version: run_command('versioningit').stdout().strip(),
  default_options: ['cpp_std=c++20'])
  
# Bind to the interpreter Meson/meson-python wants us to use.
py = import('python').find_installation()
fs = import('fs')

py.install_sources(
  'src/functional.py',
  subdir: 'retracesoftware')

py.extension_module(
  'retracesoftware_functional',
  ['src/functional.cpp',
   'src/callall.cpp',
   'src/sideeffect.cpp',
   'src/compose.cpp',
   'src/repeatedly.cpp',
   'src/notpredicate.cpp',
   'src/andpredicate.cpp',
   'src/orpredicate.cpp',
   'src/whenpredicate.cpp',
    'src/casepredicate.cpp',
    'src/typepredicate.cpp',
    'src/first.cpp',
    'src/advice.cpp',
    'src/memoize.cpp',
    'src/repr.cpp',
    'src/manypredicate.cpp',
    'src/cache.cpp',
    'src/threadlocalproxy.cpp',
    'src/partial.cpp',
    'src/instancetest.cpp',
    'src/methodinvoker.cpp',
    'src/intercept.cpp',
    'src/indexer.cpp',
    'src/param.cpp',
    'src/ternarypredicate.cpp',
    'src/ifthenelse.cpp',
    'src/anyargs.cpp',
    'src/transformargs.cpp',
    'src/walker.cpp',
    'src/when.cpp',
    'src/firstof.cpp',
    'src/always.cpp',
    'src/selfapply.cpp',
    'src/spread.cpp',
    'src/constantly.cpp',
    'src/either.cpp',
    'src/compose2.cpp',
    'src/dropargs.cpp',
    'src/vector.cpp',
    'src/usewith.cpp',
    'src/deepwrap.cpp',
    'src/whennotnone.cpp'
    ],
  install: true,
  link_args: ['-g'],
  
  # If you want extra flags:
  # cpp_args: ['-O3', '-fvisibility=hidden'],
  include_directories: include_directories('common-headers/include'),
)