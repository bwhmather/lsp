project('lsp', 'c')

sources = [
  'src/builtins.c',
  'src/env.c',
  'src/eval.c',
  'src/interpreter.c',
  'src/parser.c',
  'src/vm.c',
]

test_suites = {
  'cons': [
    'car',
    'cdr',
    'int_is_not_cons',
    'is_cons_pops',
    'null_is_not_cons',
    'nulls',
    'overflow_1',
    'push',
    'set_car',
    'set_cdr',
  ],
  'reverse': [
    'null',
    'pair',
    'triple',
  ],
}

includes = include_directories('include')

lib = both_libraries(
  'lsp', sources,
  include_directories : includes,
  install : true,
)

executable(
  'lsp', 'main.c',
  include_directories : includes,
  link_with : lib,
  install : true,
)

test_includes = include_directories('include', 'tests')

foreach suite, tests : test_suites
  foreach test_name : tests
    test_exe = executable(
      'test_' + suite + '_' + test_name,
      'tests/' + suite + '/test_' + test_name + '.c',
      include_directories : test_includes,
      link_with : lib,
      install : false,
    )
    test(suite + ':' + test_name, test_exe)
  endforeach
endforeach
