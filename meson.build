project('player', 'cpp',
        version: '0.1.0',
        default_options: ['cpp_std=c++2a'],
        license: 'GPL3')

add_global_arguments('-Dwarning_level=3', language: 'cpp')

composer_sources = ['composer.cc', 'phrase.cc', 'random.cc']
composer_lib = library('composer', composer_sources)

csound_dep = dependency('csound')

session_sources = ['session.cc']
session_lib = library('session', session_sources,
                      dependencies: [csound_dep])

composure_sources = ['main.cc']
composure_app = executable('composure', composure_sources,
                           dependencies: [csound_dep],
                           link_with: [composer_lib, session_lib])

test_sources = ['test.cc', 'test-phrase.cc', 'test-random.cc']
test_app = executable('test_app', test_sources, link_with: composer_lib)
test('composure test', test_app)
