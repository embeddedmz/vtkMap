# ==============================================================================
# Source
# ==============================================================================
set(vtkMapCore_SOURCE
    Timer.h
    vtkFeature.cxx
    vtkFeatureLayer.cxx
    vtkGeoMapFeatureSelector.cxx
    vtkGeoMapLayerPass.cxx
    vtkGeoMapSelection.cxx
    vtkInteractorStyleGeoMap.cxx
    vtkMapMarkerSet.cxx
    vtkMapPointSelection.cxx
    vtkMapTile.cxx
    vtkMap.cxx
    vtkMultiThreadedBingLayer.cxx
    vtkMultiThreadedOsmLayer.cxx
    vtkLayer.cxx
    vtkBingLayer.cxx
    vtkOsmLayer.cxx
    vtkPolydataFeature.cxx
    vtkRasterFeature.cxx
    vtkTeardropSource.cxx
    vtkScalarBarFeature.cxx
    vtkParaViewScalarBarFeature.cxx
    vtkBoundingRectContextDevice2D.cxx
    vtkContext2DScalarBarActor.cxx
    )

set (vtkMapCore_HEADERS
    vtkFeature.h
    vtkFeatureLayer.h
    vtkGeoMapFeatureSelector.h
    vtkGeoMapLayerPass.h
    vtkGeoMapSelection.h
    vtkInteractorStyleGeoMap.h
    vtkMapMarkerSet.h
    vtkMapPointSelection.cxx
    vtkMapTile.h
    vtkMapTileSpecInternal.h
    vtkMap.h
    vtkMap_typedef.h
    vtkMercator.h
    vtkLayer.h
    vtkMultiThreadedBingLayer.h
    vtkMultiThreadedOsmLayer.h
    vtkBingLayer.h
    vtkOsmLayer.h
    vtkPolydataFeature.h
    vtkRasterFeature.h
    vtkTeardropSource.h
    vtkScalarBarFeature.h
    vtkParaViewScalarBarFeature.h
    vtkBoundingRectContextDevice2D.h
    vtkContext2DScalarBarActor.h
    ${CMAKE_CURRENT_BINARY_DIR}/vtkmapcore_export.h
    )

# ==============================================================================
# Target
# ==============================================================================
add_library(vtkMapCore ${vtkMapCore_SOURCE})

set(CMAKE_CXX_VISIBILITY_PRESET)
set(CMAKE_VISIBILITY_INLINES_HIDDEN)

target_link_libraries(vtkMapCore
                      LINK_PUBLIC
                        ${VTK_LIBRARIES}
                      LINK_PRIVATE
                        ${CURL_LIBRARIES}
                        ${OPENGL_LIBRARIES}
                      )

target_include_directories(vtkMapCore
                           PUBLIC
                             ${CMAKE_BINARY_DIR}
                             ${CMAKE_CURRENT_BINARY_DIR}
                             ${CMAKE_CURRENT_SOURCE_DIR}
                           PRIVATE
                             ${CURL_INCLUDE_DIRS}
                             ${OPENGL_INCLUDE_DIRS}
                          )

generate_export_header(vtkMapCore)

vtkmap_install_target(vtkMapCore)
install (FILES ${vtkMapCore_HEADERS} DESTINATION include)
