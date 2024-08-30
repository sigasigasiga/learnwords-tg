# macos build cmdline
# ```
# meson setup --cross-file brew-cross.ini bin
# ```
project(
  'learnwords-tg',
  'cpp',
  default_options: ['cpp_std=c++23']
)

g_includes = include_directories('src/', 'subprojects/beman-optional26/include/')
g_shared_pch = files('src/shared_pch.hpp')[0].full_path() # TODO: what the fuck
g_deps = [
  dependency('fmt'),
  dependency('spdlog'),
  dependency('magic_enum'),
  dependency('boost', modules: ['charconv', 'iostreams', 'json', 'program_options']),
  dependency('openssl'),
  dependency('siga-util'),
]

# subdir('src/lw/database')

executable(
  meson.project_name(),
  sources: [
    'src/main.cpp',

    'src/lw/application/application.cpp',
    'src/lw/application/inventory.cpp',
    'src/lw/application/inventory.cpp',
    'src/lw/application/service/db.cpp',
    'src/lw/application/service/tg.cpp',
    'src/lw/application/service/user_dialog.cpp',

    'src/lw/telegram/exception.cpp',
    'src/lw/telegram/process_response.cpp',
    'src/lw/telegram/update/allowed_updates.cpp',
    'src/lw/telegram/update/long_polling.cpp',
    'src/lw/telegram/update/polymorphic_update.cpp',

    'src/lw/error/code.cpp',

    'src/lw/util/credentials.cpp',
  ],
  include_directories: g_includes,
  dependencies: g_deps,
  cpp_pch: g_shared_pch
)
