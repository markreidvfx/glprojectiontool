include_directories(
scene
widget
)


add_subdirectory(scene)
add_subdirectory(widget)

qt5_add_resources(RESOURCES shaders.qrc)
qt5_wrap_ui(UI_HEADERS projector.ui)

add_executable(glprojectiontool
main.cpp
projector.cpp
${UI_HEADERS}
${RESOURCES}
)

target_link_libraries(glprojectiontool projectionwidget Qt5::Widgets)

install(TARGETS glprojectiontool DESTINATION bin)
