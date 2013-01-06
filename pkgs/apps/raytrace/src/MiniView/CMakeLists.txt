ADD_EXECUTABLE(rtview rtview ../RTTL/common/MapOptions)
TARGET_LINK_LIBRARIES(rtview RTTL LRT ${GLUT_glut_LIBRARY} ${GL_LIBS} $ENV{LIBS} stdc++ rt)

INSTALL(TARGETS rtview RUNTIME DESTINATION bin)

