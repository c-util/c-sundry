#
# target: libcsundry.so
# Right now, c-sundry has no built sources, so there is no library to build.
# Instead, we just install the header plus pkg-config file. In case we ever add
# real code, both the pkg-config and libcsundry_dep ways allow backwards
# compatible addition of linker arguments.
#

install_headers(['c-bitmap.h',
                 'c-macro.h',
                 'c-ref.h',
                 'c-string.h',
                 'c-syscall.h',
                 'c-usec.h'])
libcsundry_dep = declare_dependency(include_directories: include_directories('.'),
                                    version: meson.project_version())
mod_pkgconfig.generate(version: meson.project_version(),
                       name: 'libcsundry',
                       filebase: 'libcsundry',
                       description: 'Collection of C sundries')

#
# target: test-*
# Symbol visibility testing, unit testing, and limited stress tests for the
# c-sundry API.
#

test_api = executable('test-api', ['test-api.c'], dependencies: libcsundry_dep)
test('API Symbol Visibility', test_api)

test_bitmap = executable('test-bitmap', ['test-bitmap.c'], dependencies: libcsundry_dep)
test('Bitmap Functionality', test_bitmap)

test_macro = executable('test-macro', ['test-macro.c'], dependencies: libcsundry_dep, link_args: '-ldl')
test('Utility Macros', test_macro)

test_string = executable('test-string', ['test-string.c'], dependencies: libcsundry_dep)
test('String Manipulators', test_string)
