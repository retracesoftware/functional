# meson.build
project('retracesoftware_functional', 'cpp', default_options: ['cpp_std=c++20'])
# project('retracesoftware_functional', 'c', default_options: ['c_std=c99'])
# project('retracesoftware_functional', ['c', 'cpp'], default_options: ['c_std=c99', 'cpp_std=c++20'])

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
    'src/compose2.cpp'
    ],
  install: true,
  link_args: ['-g'],
  
  # If you want extra flags:
  # cpp_args: ['-O3', '-fvisibility=hidden'],
  include_directories: include_directories('common-headers/include'),
)