@SET_MAKE@

bin_PROGRAMS = bodytrack
bodytrack_SOURCES = DMatrix.h \
                    GeneralUtil.h \
                    ParticleFilter.h \
                    SmallVectors.h \
                    Vector3.h \
                    system.h \
                    AnnealingFactor.h \
                    AnnealingFactor.cpp \
                    BodyGeometry.h \
                    BodyGeometry.cpp \
                    BodyPose.h \
                    BodyPose.cpp\
                    CameraModel.h \
                    CameraModel.cpp \
                    CovarianceMatrix.h \
                    CovarianceMatrix.cpp \
                    ImageMeasurements.h \
                    ImageMeasurements.cpp\
                    ImageProjection.h \
                    ImageProjection.cpp \
                    RandomGenerator.h \
                    RandomGenerator.cpp \
                    TrackingModel.h \
                    TrackingModel.cpp \
                    main.cpp

if ENABLE_OPENMP
  bodytrack_SOURCES += ParticleFilterOMP.h \
                       TrackingModelOMP.h \
                       TrackingModelOMP.cpp
endif

if ENABLE_THREADS
  bodytrack_SOURCES += ParticleFilterPthread.h \
                       TrackingModelPthread.h \
                       TrackingModelPthread.cpp \
                       WorkPoolPthread.h \
                       AsyncIO.h \
                       AsyncIO.cpp
endif

if ENABLE_TBB
  bodytrack_SOURCES += TBBtypes.h \
                       ParticleFilterTBB.h \
                       TrackingModelTBB.h \
                       TrackingModelTBB.cpp
endif

AM_CPPFLAGS = -I$(VPATH)/../FlexImageLib
bodytrack_LDADD = ../FlexImageLib/libflex.la -lm

if ENABLE_THREADS
  bodytrack_LDADD += threads/libthreads.la
  SUBDIRS = threads
endif

if ENABLE_TBB
  bodytrack_LDADD += -ltbb
endif

