PROJECT (Raytrace)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

ADD_DEFINITIONS(-DTHIS_IS_CMAKE)

# =============================================================================
SET(RTTL_DEFAULT_BVH_BUILDER "default" CACHE STRING "Default BVH builder")
SET(RTTL_STATS_BVH "default" CACHE STRING "Track stats for BVH traversal")
SET(NEEDS_PTHREAD_BARRIER_T_WRAPPER ON CACHE BOOL "Whether we need to write out own wrappers for pthread_barrier_t ")
IF (APPLE)
   SET(NEEDS_PTHREAD_BARRIER_T_WRAPPER ON)
ENDIF (APPLE)

# =============================================================================
# support for PBOs on or off.
# by default, we enable it. if your GL version doesn't support it
# you'll notice early enough....

# passed on via config.h

SET(USE_PBOS 1 CACHE BOOL "Whether compiling with or a/o PBO support")
SET(NEED_ARB_WRAPPERS 0 CACHE BOOL "Whether we need a special ARB string for old gl libs")

# =============================================================================
# will read file from source_dir, 
# replace all cmake variables with their current cmake content, 
# and write modified file to binary_dir
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/RTTL/common/cmake_autoconfig_template.h
  ${CMAKE_BINARY_DIR}/RTTL/common/cmake_autoconfig.h
  )

# =============================================================================
# X11,OpenGL, GLU, GLUT

IF (APPLE)
  SET (OPENGL_INCLUDE_DIR               /usr/X11R6/include)
  SET (OPENGL_gl_LIBRARY                /usr/X11R6/lib/libGL.dylib)
  SET (OPENGL_glu_LIBRARY               /usr/X11R6/lib/libGLU.dylib)
  SET (GLUT_glut_LIBRARY              /opt/local/lib/libglut.dylib)
#  ADD_DEFINITIONS(-DTHIS_IS_APPLE)
ENDIF (APPLE)

IF (CYGWIN)
  SET (OPENGL_INCLUDE_DIR               /usr/X11R6/include)
  IF (BUILD_SHARED_LIBS)
    SET (OPENGL_gl_LIBRARY              /usr/X11R6/bin/libGL.dll)
    SET (OPENGL_glu_LIBRARY             /usr/X11R6/bin/libGLU.dll)
  ELSE (BUILD_SHARED_LIBS)
    SET (OPENGL_gl_LIBRARY              /usr/X11R6/lib/libGL.dll.a)
    SET (OPENGL_glu_LIBRARY             /usr/X11R6/lib/libGLU.dll.a)
  ENDIF (BUILD_SHARED_LIBS)
ENDIF (CYGWIN)

INCLUDE (${CMAKE_ROOT}/Modules/FindX11.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindThreads.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindGLU.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindGLUT.cmake)

IF (CYGWIN)
  # this is windows, have our own glut lib in this project...


# --------------------------------------------
# all cygwin libs (GL,GLUT)
  SET(GLUT_INCLUDE_DIR "/usr/include")
  SET (OPENGL_gl_LIBRARY              /lib/w32api/libopengl32.a)
  SET (OPENGL_glut_LIBRARY            /lib/w32api/libglut32.a)
  SET (OPENGL_glu_LIBRARY             "")
  ADD_DEFINITIONS(-DWIN32)

#  SET(GLUT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Utils/glut-win32")
#  SET (OPENGL_gl_LIBRARY              "") #/lib/w32api/libopengl32.a)
#  SET (OPENGL_glut_LIBRARY            "") #/lib/w32api/libglut32.a)
#  SET (OPENGL_glut_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/Utils/glut-win32/glut32.lib")
#  SET (OPENGL_glu_LIBRARY             "")
##/usr/X11R6/bin/cygGLU-1.dll)
#  ADD_DEFINITIONS(-DWIN32)
ENDIF (CYGWIN)

SET(GL_LIBS ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})

MARK_AS_ADVANCED(
  GL_LIBS
)
# =============================================================================


# =============================================================================
# whether system supports glMapBuffer() etc, or old-style
# glMapBufferARB(). by default, we assume you have a new opengl
# installation that supports pixel buffer objects natively. if not
# you'll notice early enough....
# =============================================================================

  

SET(DEFAULT_BUILD_SHARED_LIBS ON)
SET(BUILD_SHARED_LIBS ${DEFAULT_BUILD_SHARED_LIBS} CACHE BOOL "Building shared or static libraries")

SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")

INCLUDE_DIRECTORIES(
  ${GLUT_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/RTTL/
  ${CMAKE_CURRENT_SOURCE_DIR}/RTTL/common/
  ${CMAKE_CURRENT_BINARY_DIR}/RTTL/common/
  # for the auto-generated header file ...
  ${CMAKE_CURRENT_SOURCE_DIR}/RTTL/BVH/
  ${CMAKE_CURRENT_SOURCE_DIR}/SG
  ${CMAKE_CURRENT_SOURCE_DIR}/LRT/include
  ${CMAKE_CURRENT_SOURCE_DIR}/LRT/
  )




IF(NOT WIN32)
 SET(ICC_PROCESSOR_FLAG "-xT")
# FIND_PROGRAM(uname_executable NAMES uname PATHS /bin /usr/bin /usr/local/bin)
# IF(uname_executable)
#   EXEC_PROGRAM(${uname_executable} ARGS -p OUTPUT_VARIABLE uname_output)
#   IF   (${uname_output} MATCHES 'Core2')
#      SET(ICC_PROCESSOR_FLAG "-xT")
#   ELSE (${uname_output} MATCHES 'Core2')
#      SET(ICC_PROCESSOR_FLAG "-xT")
#   ENDIF(${uname_output} MATCHES 'Core2')
# ENDIF(uname_executable)
ENDIF(NOT WIN32)


#SET(CMAKE_CXX_COMPILER "g++")
#SET(CMAKE_CXX_FLAGS " -Os -fno-strict-aliasing -fno-align-labels -DDEBUG -D_MM_NO_ALIGN_CHECK")

#ADD_LIBRARY(OpenRT SHARED ${OSE_OpenRT_SRCS})

SET(PASSED_FIRST_CONFIGURE ON CACHE INTERNAL "Already Configured once?")

ADD_DEFINITIONS(-D__$(USER)__)

SUBDIRS(
	RTTL     # core library
	LRT      # low level ray tracer
	MiniView  # frontends
	)

ADD_EXECUTABLE(test_glut Testing/testglut)
TARGET_LINK_LIBRARIES(test_glut ${GLUT_glut_LIBRARY} ${GL_LIBS} $ENV{LIBS})

#ADD_EXECUTABLE(test_pbo Testing/testpbo)
#TARGET_LINK_LIBRARIES(test_pbo ${GLUT_glut_LIBRARY} ${GL_LIBS} $ENV{LIBS} stdc++)

INSTALL(TARGETS test_glut RUNTIME DESTINATION bin)
#INSTALL(TARGETS test_pbo RUNTIME DESTINATION bin)

