project('lsp', 'c')

sources = [
  'src/builtins.c',
  'src/env.c',
  'src/eval.c',
  'src/interpreter.c',
  'src/parser.c',
  'src/vm.c',
]

include = include_directories('include')

lib = both_libraries(
  'lsp', sources,
  include_directories : include,
  install : true,
)

executable(
  'lsp', 'main.c',
  include_directories : include,
  link_with : lib,
  install : true,
)
