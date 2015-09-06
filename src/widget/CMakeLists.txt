
add_library(projectionwidget
openglwindow.cpp
projectionwidget.cpp
glew_helper.cpp
loader.cpp
)

set_property(TARGET projectionwidget PROPERTY CXX_STANDARD 11)
set_property(TARGET projectionwidget PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(projectionwidget
${Qt5Widgets_LIBRARIES}
${OPENGL_LIBRARY}
${GLEW_LIBRARY}
projectionscene
)
