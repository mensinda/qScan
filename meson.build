project(
    'qScan', ['cpp'],
    license: 'Apache-2.0',
    version: '0.0.1',
    default_options: [
        'optimization=2',
        'warning_level=3',
        'werror=true',
        'cpp_std=c++20',
    ]
)

add_global_arguments('-Wno-pedantic', language: 'cpp')

sane_dep = dependency('sane-backends')
json_dep = dependency('nlohmann_json')

cm = import('cmake')
spdlog_opt = cm.subproject_options()
spdlog_opt.add_cmake_defines({
    'SPDLOG_BUILD_SHARED':     'OFF',
    'SPDLOG_BUILD_PIC':        'ON',
    'SPDLOG_BUILD_EXAMPLE':    'OFF',
    'SPDLOG_BUILD_EXAMPLE_HO': 'OFF',
    'SPDLOG_BUILD_TESTS':      'OFF',
    'SPDLOG_BUILD_TESTS_HO':   'OFF',
    'SPDLOG_BUILD_BENCH':      'OFF',
    'SPDLOG_BUILD_WARNINGS':   'OFF',
    'SPDLOG_INSTALL':          'OFF',
    'SPDLOG_USE_STD_FORMAT':   'OFF',
    'SPDLOG_FMT_EXTERNAL':     'OFF',
    'SPDLOG_FMT_EXTERNAL_HO':  'OFF',
    'SPDLOG_NO_EXCEPTIONS':    'OFF',
})
spdlog_sp  = cm.subproject('spdlog', options: spdlog_opt)
spdlog_dep = spdlog_sp.dependency('spdlog')

qt = import('qt5')
qt_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'], include_type: 'system')

subdir('lib')
subdir('gui')
subdir('test')
