gui_cpp_src = [
    'main.cpp',

    'config/QScanConfig.cpp',

    'viewer/ScanImageView.cpp',
    'viewer/SelectionOverlay.cpp',

    'util/ImageEditor.cpp',
    'util/QImageContainer.cpp',
]

gui_hpp_src = [
    'viewer/ScanImageView.hpp',
]

gui_ui_src  = []

widgets = [
    'MainWindow',
    'DeviceElement',
    'DeviceSelection',
    'ImagesTab',
    'ImageViewer',
    'OptionsWidget',
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
    'qscan', [gui_cpp_src, gui_hpp_src, gui_ui, gui_moc],
    dependencies: [qt_dep, qscan_lib_dep, magick_dep],
    include_directories: ['widgets', 'viewer'],
    install: true,
)
