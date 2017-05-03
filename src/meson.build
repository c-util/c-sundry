#
# target: libcsundry.so
#

install_headers(
        [
                'c-bitmap.h',
                'c-macro.h',
                'c-ref.h',
                'c-string.h',
                'c-syscall.h',
                'c-usec.h'
       ],
)

libcsundry_dep = declare_dependency(
        include_directories: include_directories('.'),
        version: meson.project_version()
)

mod_pkgconfig.generate(
        version: meson.project_version(),
        name: 'libcsundry',
        filebase: 'libcsundry',
        description: 'Collection of C sundries'
)

#
# target: test-*
#

test_api = executable('test-api', ['test-api.c'], dependencies: libcsundry_dep)
test('API Symbol Visibility', test_api)

test_bitmap = executable('test-bitmap', ['test-bitmap.c'], dependencies: libcsundry_dep)
test('Bitmap Functionality', test_bitmap)

test_macro = executable('test-macro', ['test-macro.c'], dependencies: libcsundry_dep, link_args: '-ldl')
test('Utility Macros', test_macro)

test_string = executable('test-string', ['test-string.c'], dependencies: libcsundry_dep)
test('String Manipulators', test_string)
