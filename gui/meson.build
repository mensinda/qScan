gui_cpp_src = [
    'main.cpp',

    'config/QScanConfig.cpp',
]

gui_hpp_src = []
gui_ui_src  = []

widgets = [
    'MainWindow',
    'DeviceElement',
    'DeviceSelection',
    'ScanRoot',
]

foreach w : widgets
    gui_cpp_src += [f'widgets/@w@.cpp']
    gui_hpp_src += [f'widgets/@w@.hpp']
    gui_ui_src  += [f'ui/@w@.ui']
endforeach

gui_ui  = qt.compile_ui(sources: gui_ui_src)
gui_moc = qt.compile_moc(sources: [], headers: gui_hpp_src)

gui_exe = executable(
    'qScan', [gui_cpp_src, gui_hpp_src, gui_ui, gui_moc],
    dependencies: [qt_dep, qscan_lib_dep],
    include_directories: ['widgets'],
)
