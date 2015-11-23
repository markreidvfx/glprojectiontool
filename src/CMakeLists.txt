include_directories(
scene
widget
)


add_subdirectory(scene)
add_subdirectory(widget)

qt5_add_resources(RESOURCES shaders.qrc)
qt5_add_resources(RESOURCES textures.qrc)
qt5_wrap_ui(UI_HEADERS projector.ui)

add_executable(glprojectiontool
application.cpp
main.cpp
projector.cpp
${UI_HEADERS}
${RESOURCES}
)

set_property(TARGET glprojectiontool PROPERTY CXX_STANDARD 11)
set_property(TARGET glprojectiontool PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(glprojectiontool
projectionwidget
Qt5::Widgets
Threads::Threads
)

install(TARGETS glprojectiontool DESTINATION bin)
