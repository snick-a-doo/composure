composure_sources = [
  'main.cc',
]

inc = include_directories('../libcomposure')

composure_app = executable('composure',
                           composure_sources,
                           include_directories: inc,
                           link_with: [composure_lib],
                           install: true)
