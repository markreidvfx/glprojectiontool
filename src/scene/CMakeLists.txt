

add_library(rc_helper
rc_helper.cpp
)

add_library(projectionscene
scene.cpp
mesh.cpp
triangle.cpp
camera.cpp
shaders.cpp
scenereader.h
imageplane.cpp
abcscenereader.cpp
abcmesh.cpp
abccamera.cpp
imagereader.cpp
framebuffer.cpp
templaterenderer.cpp
contourrender.cpp
)

target_include_directories(projectionscene PRIVATE
${ALEMBIC_INCLUDE_DIR}
${EXR_INCLUDE_DIR}
${EXR_INCLUDE_DIR}/OpenEXR
${ImageMagick_INCLUDE_DIRS}
${Boost_INCLUDE_DIRS}
${OPENSUBDIV_INCLUDE_DIR}
${ZLIB_INCLUDE_DIRS}
)

target_compile_definitions(projectionscene PRIVATE
-DMAGICKCORE_HDRI_ENABLE=1
-DMAGICKCORE_QUANTUM_DEPTH=16
)

set_property(TARGET projectionscene PROPERTY CXX_STANDARD 11)
set_property(TARGET projectionscene PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(rc_helper
${Qt5Widgets_LIBRARIES}
)

target_link_libraries(projectionscene
rc_helper
${GLEW_LIBRARY}
${OPENGL_LIBRARY}
${SCENE_LIBRARIES}
${ImageMagick_LIBRARIES}
${ZLIB_LIBRARIES}
)

set_target_properties(rc_helper PROPERTIES AUTOMOC FALSE)
set_target_properties(projectionscene PROPERTIES AUTOMOC FALSE)

