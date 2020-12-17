test_sources = [
  'test.cc',
  'test-midi.cc',
  'test-phrase.cc',
  'test-random.cc',
]

inc = include_directories('.', '../libcomposure')

test_app = executable('test_app',
                      test_sources,
                      override_options: ['cpp_std=c++2a'],
                      include_directories: inc,
                      link_with: composure_lib)

test('composure test', test_app)
