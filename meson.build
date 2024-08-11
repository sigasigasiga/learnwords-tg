# macos cmdline
# ```
# CC=clang CXX=clang++ LDFLAGS="-L/opt/homebrew/opt/llvm/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++" meson setup bin
# ```
project(
  'learnwords-tg',
  'cpp',
  default_options: ['cpp_std=c++23']
)

g_includes = include_directories('src/')
g_shared_pch = files('src/shared_pch.hpp')[0].full_path() # TODO: what the fuck
g_deps = [
  dependency('tl-optional'),
  dependency('spdlog'),
  dependency('boost', modules: ['charconv', 'json', 'program_options']),
  dependency('openssl'),
]

# subdir('src/lw/database')

executable(
  meson.project_name(),
  sources: [
    'src/main.cpp',

    'src/lw/application/application.cpp',
    'src/lw/application/inventory.cpp',
    'src/lw/application/inventory.cpp',
    'src/lw/application/service/user_dialog.cpp',

    'src/lw/telegram/exception.cpp',
    'src/lw/telegram/process_response.cpp',
    'src/lw/telegram/update/allowed_updates.cpp',
    'src/lw/telegram/update/long_polling.cpp',
    'src/lw/telegram/update/polymorphic_update.cpp',

    'src/lw/error/code.cpp',
  ],
  include_directories: g_includes,
  dependencies: g_deps,
  cpp_pch: g_shared_pch
)
