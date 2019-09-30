project('lsp', 'c')

includes = include_directories('include')

### Library ###
lib_sources = [
  'src/builtins.c',
  'src/env.c',
  'src/eval.c',
  'src/interpreter.c',
  'src/reader.c',
  'src/vm.c',
]

lib = both_libraries(
  'lsp', lib_sources,
  include_directories : includes,
  install : true,
)

### Interpreter ###
executable(
  'lsp', 'main.c',
  include_directories : includes,
  link_with : lib,
  install : true,
)

### Tests ###
test_includes = include_directories('include', 'tests')

test_suites = {
  'cons': [
    'car',
    'cdr',
    'int_is_not_cons',
    'is_cons_pops',
    'null_is_not_cons',
    'nulls',
    'overflow_1',
    'overflow_2',
    'push',
    'set_car',
    'set_cdr',
  ],
  'reverse': [
    'null',
    'pair',
    'triple',
  ],
  'reader': [
    'empty',
    'int_single_digit',
    'int_multiple_digit',
    'empty_list',
    'single_element_list',
    'multi_element_list',
    'nested_list',
    'list_of_nils',
    'string',
    'string_escapes',
  ],
  'env': [
    'push_empty',
    'push_scope',
    'define',
    'lookup',
    'lookup_older',
    'lookup_outer',
    'lookup_shadowed',
    'set',
  ],
  'eval': [
    'int',
    'symbol',
    'if_true',
    'if_false',
    'add',
    'quote_int',
    'quote_lambda',
  ],
}

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
