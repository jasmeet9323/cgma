# Don't require GNU-standard files (Changelog, README, etc.)
AUTOMAKE_OPTIONS = foreign

DEFS = $(TEMPLATE_DEFS_INCLUDED) $(LITTLE_ENDIAN)
AM_CPPFLAGS = -I$(top_srcdir)/util -I$(top_srcdir)/geom

# The name of the library to build
noinst_LTLIBRARIES = 
if BUILD_CGM
  noinst_LTLIBRARIES += libCholla.la
endif

# The directory where headers will be installed
libCholla_la_includedir = $(includedir)

LDADD = ../../util/libcubit_util.la ../../geom/libcubit_geom.la

# The non-template sources
libCholla_la_SOURCES = \
    AllocMemManagersCholla.cpp \
    Cholla.cpp \
    ChollaCurve.cpp \
    ChollaEngine.cpp \
    ChollaEntity.cpp \
    ChollaPoint.cpp \
    ChollaSkinTool.cpp \
    ChollaSurface.cpp \
    ChollaVolume.cpp \
    ChordalAxis.cpp \
    CubitFacet.cpp \
    CubitFacetData.cpp \
    CubitFacetEdge.cpp \
    CubitFacetEdgeData.cpp \
    CubitPoint.cpp \
    CubitPointData.cpp \
    CubitQuadFacet.cpp \
    CubitQuadFacetData.cpp \
    CurveFacetEvalTool.cpp \
    debug.cpp \
    FacetDataUtil.cpp \
    FacetEntity.cpp \
    FacetEvalTool.cpp \
    GeoNode.cpp \
    GeoTet.cpp \
    LoopParamTool.cpp \
    PointGridSearch.cpp \
    PointLoopFacetor.cpp \
    TDChordal.cpp \
    TDFacetboolData.cpp \
    TDFacetBoundaryEdge.cpp \
    TDFacetBoundaryPoint.cpp \
    TDGeomFacet.cpp 

# Headers to be installed.  If any file in this list should
# not be installed, move it to the _SOURCES list above.
libCholla_la_include_HEADERS = \
    BoundaryConstrainTool.hpp \
    Cholla.h \
    ChollaCurve.hpp \
    ChollaEngine.hpp \
    ChollaEntity.hpp \
    ChollaPoint.hpp \
    ChollaSkinTool.hpp \
    ChollaSurface.hpp \
    ChollaVolume.hpp \
    ChordalAxis.hpp \
    CubitFacet.hpp \
    CubitFacetData.hpp \
    CubitFacetEdge.hpp \
    CubitFacetEdgeData.hpp \
    CubitPoint.hpp \
    CubitPointData.hpp \
    CubitQuadFacet.hpp \
    CubitQuadFacetData.hpp \
    CurveFacetEvalTool.hpp \
    FacetDataUtil.hpp \
    FacetEntity.hpp \
    FacetEvalTool.hpp \
    FacetorTool.hpp \
    FacetorUtil.hpp \
    GeoNode.hpp \
    GeoTet.hpp \
    LoopParamTool.hpp \
    PointGridSearch.hpp \
    PointLoopFacetor.hpp \
    TDChordal.hpp \
    TDFacetBoundaryEdge.hpp \
    TDFacetBoundaryPoint.hpp \
    TDFacetboolData.hpp \
    TDGeomFacet.hpp \
    TDDelaunay.hpp \
    TetFacetorTool.hpp \
    debug.hpp 

# If template defs are included, then the template definitions
# need to be installed with the headers.  Otherwise they need
# to be compiled.
if INCLUDE_TEMPLATE_DEFS
  libCholla_la_include_HEADERS += \
    BoundaryConstrainTool.cpp \
    FacetorTool.cpp \
    TetFacetorTool.cpp \
    FacetorUtil.cpp \
    TDDelaunay.cpp 
else
  libCholla_la_SOURCES += \
    BoundaryConstrainTool.cpp \
    FacetorTool.cpp \
    TetFacetorTool.cpp \
    FacetorUtil.cpp \
    TDDelaunay.cpp 
endif

libCholla_la_LIBADD=${CGM_EXT_LDFLAGS}
