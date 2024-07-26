database_src = [
  'connection.cpp',
]

database_lib = static_library(
  'database',
  sources: database_src,
  include_directories: g_includes,
  dependencies: g_deps,
  cpp_pch: g_shared_pch
)
