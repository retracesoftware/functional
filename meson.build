# meson.build
project('retracesoftware.functional', 'cpp', default_options: ['cpp_std=c++20'])

# Bind to the interpreter Meson/meson-python wants us to use.
py = import('python').find_installation()
fs = import('fs')

py.extension_module(
  'functional',
  ['src/functional.cpp',
   'src/transformcall.cpp',
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
    'src/transformargs.cpp',
    'src/first.cpp',
    'src/advice.cpp',
    'src/memoize.cpp',
    'src/repr.cpp',
    'src/manypredicate.cpp',
    'src/cache.cpp',
    'src/threadlocalproxy.cpp',
    'src/partial.cpp',
    'src/methodinvoker.cpp',
    'src/intercept.cpp',
    'src/indexer.cpp',
    'src/param.cpp',
    'src/ternarypredicate.cpp'],
  install: true,
  subdir: 'retracesoftware',

  # If you want extra flags:
  # cpp_args: ['-O3', '-fvisibility=hidden'],
  #include_directories: include_directories('.'),
)