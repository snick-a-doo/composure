project('composure', 'cpp',
        version: '0.1.0',
        default_options: ['cpp_std=c++2a'],
        license: 'GPL3')

add_global_arguments('-Dwarning_level=3', '-std=c++2a', language: 'cpp')

compose_sources = ['compose.cc', 'midi.cc', 'phrase.cc', 'random.cc']
compose_lib = library('compose', compose_sources)

composure_sources = ['main.cc']
composure_app = executable('composure', composure_sources, link_with: [compose_lib])

test_sources = ['test.cc', 'test-midi.cc', 'test-phrase.cc', 'test-random.cc']
test_app = executable('test_app', test_sources, link_with: compose_lib)
test('composure test', test_app)
