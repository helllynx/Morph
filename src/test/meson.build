googletest_dep = dependency('googletest', method: 'pkg-config')
thread_dep = dependency('threads')

morph_test_src = ['immutabletest.cpp']

morph_test = executable(
    'morphtest',
    morph_test_src,
    include_directories: root_include_dir,
    dependencies: [googletest_dep, thread_dep]
)

test('morphtest', morph_test)
