project('player', 'cpp',
        default_options: ['cpp_std=c++17'],
        version: '0.1.0',
        license: 'GPL3')
# The line below makes 'meson setup builddir' give the warning
#     meson.build:12: WARNING: Consider using the built-in option for language standard
#     version instead of using "-std=c++17".
#
# But setting the project option doesn't seem to get it into the compiler options.
# meson configure gives
#     Compiler options    Current Value ...
#     ----------------    ------------- ...
#     ...
#     cpp_std             none          ...

add_global_arguments('-Dwarning_level=3', '-std=c++17', language: 'cpp')

composer_sources = ['composer.cc', 'random.cc']
composer_lib = library('composer', composer_sources)

csound_dep = dependency('csound')

session_sources = ['session.cc']
session_lib = library('session', session_sources,
                      dependencies: [csound_dep])

gtkmm_dep = dependency('gtkmm-3.0')

play_sources = ['play.cc']
play_app = executable('play', play_sources,
                      dependencies: [csound_dep],
                      link_with: [composer_lib, session_lib])

# Interactive
# player_sources = ['player.cc']
# player_app = executable('player', player_sources,
#                         dependencies: [gtkmm_dep, csound_dep],
#                         link_with: [composer_lib, session_lib])

# test_sources = ['test.cc', 'test_composer.cc']
# test_app = executable('test_app', test_sources, link_with: composer_lib)
# test('composer test', test_app)
