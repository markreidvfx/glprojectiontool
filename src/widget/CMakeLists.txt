
add_library(projectionwidget
openglwindow.cpp
projectionwidget.cpp
glew_helper.cpp
)

target_link_libraries(projectionwidget
${Qt5Widgets_LIBRARIES}
${OPENGL_LIBRARY}
${GLEW_LIBRARY}
projectionscene
)
