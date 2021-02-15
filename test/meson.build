test_sources = [
  'test.cc',
  'test-midi.cc',
  'test-phrase.cc',
  'test-random.cc',
]

inc = include_directories('.', '../libcomposure')

test_app = executable('test_app',
                      test_sources,
                      include_directories: inc,
                      link_with: composure_lib)

test('composure test', test_app)
