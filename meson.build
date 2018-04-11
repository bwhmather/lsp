project(
  'lsp', 'c',
  default_options : ['c_std=c11'],
)
add_project_arguments(
  '-pedantic', '-Wextra',
  language : 'c',
)

sources = [
  'src/vm.c', 'src/builtins.c',
  'src/parser.c', 'src/env.c',
  'src/eval.c', 'src/interpreter.c',
]


lib = shared_library(
  'lsp', sources, install : true,
  include_directories : include_directories('include'),
  version : '0.0.1', soversion : '0',
)


executable(
  'lsp', 'main.c', install : true,
  link_with : lib,
  include_directories : include_directories('include'),
)


criterion_dep = dependency('criterion')

executable(
  'lsp-test', 'test/test_cons.c',
  include_directories : include_directories('include'),
  link_with : [lib], dependencies : [criterion_dep],
)
