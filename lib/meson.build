qscan_lib_src = files([
    'qscan_log.cpp',
    'ScanBackend.cpp',
    'SaneBackend.cpp',
])

sane_inc_dir = sane_dep.get_variable(pkgconfig: 'includedir')

enumGen_sp  = subproject('enumGen')
enumGen     = enumGen_sp.get_variable('enumGen')
enumGen_cc  = enumGen_sp.get_variable('cc_enumGen')
enumGen_cfg = files(['enumGen.json'])

enumGen_src = [
    sane_inc_dir / 'sane' / 'sane.h',
]

enumGen_extracted = enumGen_cc.process(enumGen_src)
enumGen_src = custom_target(
  'enum2str',
  input: [enumGen_extracted],
  output: ['enum2str.hpp', 'enum2str.cpp'],
  command: [enumGen, '-c', enumGen_cfg, 'generate', 'enum2str', '@OUTPUT@', '@INPUT@']
)

qscan_lib_inc_dirs = include_directories('.')

qscan_lib = static_library(
    'qscan_lib', [qscan_lib_src, enumGen_src],
    include_directories: qscan_lib_inc_dirs,
    dependencies: [sane_dep, spdlog_dep],
)

qscan_lib_dep = declare_dependency(
    include_directories: qscan_lib_inc_dirs,
    link_with: qscan_lib,
    dependencies: [sane_dep, spdlog_dep],
    version: meson.project_version(),
)
