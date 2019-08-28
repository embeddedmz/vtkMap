#include <curl/curl.h>

#include "vtkFeatureLayer.h"
#include "vtkGeoMapSelection.h"
#include "vtkInteractorStyleGeoMap.h"
#include "vtkMap.h"
#include "vtkMapMarkerSet.h"
#include "vtkMercator.h"
#include "vtkMultiThreadedOsmLayer.h"
#include "vtkOsmLayer.h"
#include "vtkPolydataFeature.h"

#include "vtkParaViewScalarBarFeature.h"
#include "vtkScalarBarFeature.h"

#include <vtkCellArray.h>
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkIdList.h>
#include <vtkInteractorStyle.h>
#include <vtkLine.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtksys/CommandLineArguments.hxx>

#include <iostream>

// ------------------------------------------------------------
class PickCallback : public vtkCommand
{
public:
  static PickCallback* New() { return new PickCallback; }

  void Execute(vtkObject* caller, unsigned long event, void* data) override
  {
    switch (event)
    {
      case vtkInteractorStyleGeoMap::DisplayClickCompleteEvent:
      {
        double* latLonCoords = static_cast<double*>(data);
        std::cout << "Point coordinates: \n"
                  << "  " << latLonCoords[0] << ", " << latLonCoords[1]
                  << std::endl;
      }
      break;

      case vtkInteractorStyleGeoMap::DisplayDrawCompleteEvent:
      {
        double* latLonCoords = static_cast<double*>(data);
        std::cout << "Rectangle coordinates: \n"
                  << "  " << latLonCoords[0] << ", " << latLonCoords[1]
                  << "\n  " << latLonCoords[2] << ", " << latLonCoords[3]
                  << std::endl;
      }
      break;

      case vtkInteractorStyleGeoMap::SelectionCompleteEvent:
      {
        vtkObject* object = static_cast<vtkObject*>(data);
        vtkGeoMapSelection* selection =
          vtkGeoMapSelection::SafeDownCast(object);

        double* latLonCoords = selection->GetLatLngBounds();
        std::cout << "Selected coordinates: \n"
                  << "  " << latLonCoords[0] << ", " << latLonCoords[1]
                  << "\n  " << latLonCoords[2] << ", " << latLonCoords[3]
                  << std::endl;

        vtkCollection* collection = selection->GetSelectedFeatures();
        std::cout << "Number of features: " << collection->GetNumberOfItems()
                  << std::endl;
        vtkNew<vtkIdList> cellIdList;
        vtkNew<vtkIdList> markerIdList;
        vtkNew<vtkIdList> clusterIdList;
        for (int i = 0; i < collection->GetNumberOfItems(); i++)
        {
          vtkObject* object = collection->GetItemAsObject(i);
          std::cout << "  " << object->GetClassName() << "\n";
          vtkFeature* feature = vtkFeature::SafeDownCast(object);

          // Retrieve polydata cells (if relevant)
          if (selection->GetPolyDataCellIds(feature, cellIdList.GetPointer()))
          {
            if (cellIdList->GetNumberOfIds() > 0)
            {
              std::cout << "    Cell ids: ";
              for (int j = 0; j < cellIdList->GetNumberOfIds(); j++)
              {
                std::cout << " " << cellIdList->GetId(j);
              }
              std::cout << std::endl;
            }
          }

          // Retrieve marker ids (if relevant)
          if (selection->GetMapMarkerIds(
                feature, markerIdList.GetPointer(), clusterIdList.GetPointer()))
          {
            std::cout << "    Marker ids: ";
            for (int j = 0; j < markerIdList->GetNumberOfIds(); j++)
            {
              std::cout << " " << markerIdList->GetId(j);
            }
            std::cout << std::endl;

            std::cout << "    Cluster ids: ";
            for (int j = 0; j < clusterIdList->GetNumberOfIds(); j++)
            {
              std::cout << " " << clusterIdList->GetId(j);
            }
            std::cout << std::endl;
          }
        } // for (i)
      }   // case
      break;

      case vtkInteractorStyleGeoMap::ZoomCompleteEvent:
      {
        double* latLonCoords = static_cast<double*>(data);
        std::cout << "Zoom coordinates: \n"
                  << "  " << latLonCoords[0] << ", " << latLonCoords[1]
                  << "\n  " << latLonCoords[2] << ", " << latLonCoords[3]
                  << std::endl;
      }
      break;

      case vtkInteractorStyleGeoMap::RightButtonCompleteEvent:
      {
        int* coords = static_cast<int*>(data);
        std::cout << "Right mouse click at (" << coords[0] << ", " << coords[1]
                  << ")" << std::endl;
      }
      break;
    } // switch
  }

  void SetMap(vtkMap* map) { this->Map = map; }

protected:
  vtkMap* Map;
};

// ------------------------------------------------------------
int main(int argc, char* argv[])
{
  // Initialize libcurl for vtkMap to avoid bad surprises
  curl_global_init(CURL_GLOBAL_DEFAULT);

  // Setup command line arguments
  bool showHelp = false;
  bool perspective = false;
  bool rubberBandDisplayOnly = false;
  bool rubberBandSelection = false;
  bool drawPolygonSelection = false;
  bool rubberBandZoom = false;
  bool singleThreaded = false;
  int zoomLevel = 2;
  int scalarBarType = 0;
  std::vector<double> centerLatLon;
  std::string tileExtension = "png";
  std::string tileServer;
  std::string tileServerAttribution;

  vtksys::CommandLineArguments arg;
  arg.Initialize(argc, argv);
  arg.StoreUnusedArguments(true);
  arg.AddArgument("-h", vtksys::CommandLineArguments::NO_ARGUMENT, &showHelp,
    "show help message");
  arg.AddArgument("--help", vtksys::CommandLineArguments::NO_ARGUMENT,
    &showHelp, "show help message");
  arg.AddArgument("-a", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &tileServerAttribution, "map-tile server attribution");
  arg.AddArgument("-d", vtksys::CommandLineArguments::NO_ARGUMENT,
    &rubberBandDisplayOnly, "set interactor to rubberband-draw mode");
  arg.AddArgument("-e", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &tileExtension, "map-tile file extension (jpg, png, etc.)");
  arg.AddArgument("-c", vtksys::CommandLineArguments::MULTI_ARGUMENT,
    &centerLatLon, "initial center (latitude longitude)");
  arg.AddArgument("-m", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &tileServer, "map-tile server (tile.openstreetmaps.org)");
  arg.AddArgument("-p", vtksys::CommandLineArguments::NO_ARGUMENT, &perspective,
    "use perspective projection");
  arg.AddArgument("-q", vtksys::CommandLineArguments::NO_ARGUMENT,
    &rubberBandZoom, "set interactor to rubberband zoom mode");
  arg.AddArgument("-r", vtksys::CommandLineArguments::NO_ARGUMENT,
    &rubberBandSelection, "set interactor to rubberband selection mode");
  arg.AddArgument("-P", vtksys::CommandLineArguments::NO_ARGUMENT,
    &drawPolygonSelection, "set interactor to polygon selection mode");
  arg.AddArgument("-s", vtksys::CommandLineArguments::NO_ARGUMENT,
    &singleThreaded, "use single-threaded map I/O");
  arg.AddArgument("-z", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &zoomLevel, "initial zoom level (1-20)");
  arg.AddArgument("-b", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &scalarBarType, "scalar bar (0: vtk - 1: paraview)");

  if (!arg.Parse() || showHelp)
  {
    std::cout << "\n" << arg.GetHelp() << std::endl;
    return -1;
  }

  vtkNew<vtkMap> map;

  // Note: Always set map's renderer *before* adding layers
  vtkNew<vtkRenderer> rend;
  map->SetRenderer(rend.GetPointer());

  // Set viewport
  //if (centerLatLon.size() == 2)
  //{
    //map->SetCenter(centerLatLon[0], centerLatLon[1]);
  //}
  //else
  //{
    map->SetZoom(16);
    map->SetCenter(42.9543, -0.326754);
  //}

  map->SetPerspectiveProjection(perspective);

  // Adjust zoom level to perspective vs orthographic projection
  // Internally, perspective is zoomed in one extra level
  // Do the same here for perspective.
  //zoomLevel += perspective ? 0 : 1;
  //map->SetZoom(zoomLevel);

  vtkOsmLayer* osmLayer;
  if (singleThreaded)
  {
    osmLayer = vtkOsmLayer::New();
  }
  else
  {
    osmLayer = vtkMultiThreadedOsmLayer::New();
  }
  map->AddLayer(osmLayer);

  if (tileServer != "")
  {
    osmLayer->SetMapTileServer(
      tileServer.c_str(), tileServerAttribution.c_str(), tileExtension.c_str());
  }

  vtkNew<vtkRenderWindow> wind;
  wind->AddRenderer(rend.GetPointer());
  //wind->SetSize(1920, 1080);
  wind->SetSize(800, 600);

  vtkNew<vtkRenderWindowInteractor> intr;
  intr->SetRenderWindow(wind.GetPointer());
  map->SetInteractor(intr.GetPointer());

  vtkMapType::Interaction mode = vtkMapType::Interaction::Default;
  if (rubberBandDisplayOnly)
  {
    mode = vtkMapType::Interaction::RubberBandDisplayOnly;
  }
  else if (rubberBandSelection)
  {
    mode = vtkMapType::Interaction::RubberBandSelection;
  }
  else if (drawPolygonSelection)
  {
    mode = vtkMapType::Interaction::PolygonSelection;
  }
  else if (rubberBandZoom)
  {
    mode = vtkMapType::Interaction::RubberBandZoom;
  }
  map->SetInteractionMode(mode);

  intr->Initialize();
  map->Draw();

  // vtkPolyData (Lac d'Uzious Lavedan Anglas)
  const double gpxCoords[][3] = {
    {42.955367, -0.327713, 1388},
    {42.955234, -0.327541, 1391},
    {42.955075, -0.327401, 1398},
    {42.954942, -0.327233, 1410},
    {42.954798, -0.327080, 1414},
    {42.954635, -0.326967, 1416},
    {42.954182, -0.326892, 1421},
    {42.954048, -0.326715, 1432},
    {42.953922, -0.326527, 1439},
    {42.953830, -0.326311, 1443},
    {42.953701, -0.326139, 1445},
    {42.953575, -0.325952, 1448},
    {42.953428, -0.325808, 1456} };
  const unsigned gpxSize = sizeof(gpxCoords) / sizeof(double[3]);

  // store altitude in an array that will become later vtkPolyData's point data scalars
  vtkNew<vtkIntArray> altitudeArray;
  altitudeArray->SetNumberOfComponents(1);
  altitudeArray->SetNumberOfTuples(gpxSize);
  for (auto p = 0u; p < gpxSize; ++p)
  {
      altitudeArray->SetValue(p, int(gpxCoords[p][2]));
  }

  // Convert poly data points from <lat, long> to <x, y>
  vtkNew<vtkPoints> gpxPoints;
  gpxPoints->SetDataTypeToDouble();
  gpxPoints->SetNumberOfPoints(gpxSize);
  for (auto p = 0u; p < gpxSize; ++p)
  {
      gpxPoints->SetPoint(p, gpxCoords[p][1], vtkMercator::lat2y(gpxCoords[p][0]), 0);
  }

  // will represent a colored path on the map
  vtkNew<vtkPolyData> path;

  // Add the points to the dataset
  path->SetPoints(gpxPoints.GetPointer());

  // Create the lines between the points
  // Create a cell array to store the lines in and add the lines to it
  vtkNew<vtkCellArray> lines;
  for (unsigned int i = 0; i < gpxSize - 1; i++)
  {
    vtkNew<vtkLine> line;
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i + 1);

    lines->InsertNextCell(line.GetPointer());
  }

  // Add the lines to the dataset
  path->SetLines(lines.GetPointer());

  // Scalar bar and polydata CTF
  vtkNew<vtkDiscretizableColorTransferFunction> lut;
  lut->SetDiscretize(1);
  lut->SetColorSpace(VTK_CTF_RGB);
  lut->AddRGBPoint(1388, 0.23137254902000001, 0.298039215686,       0.75294117647100001);
  lut->AddRGBPoint(1422, 0.86499999999999999, 0.86499999999999999,  0.86499999999999999);
  lut->AddRGBPoint(1456, 0.70588235294099999, 0.015686274509800001, 0.149019607843);
  lut->Build();

  // Add altitude colors to the dataset
  vtkSmartPointer<vtkUnsignedCharArray> altitudeColorsArray;
  vtkUnsignedCharArray* ret = lut->MapScalars(altitudeArray.GetPointer(),
                                              VTK_COLOR_MODE_MAP_SCALARS, 0);
  altitudeColorsArray.TakeReference(ret);
  path->GetPointData()->SetScalars(altitudeColorsArray);

  // ....
  double latLon[4];
  map->GetVisibleBounds(latLon);
  std::cout << "lat-lon bounds: "
            << "(" << latLon[0] << ", " << latLon[1] << ")"
            << "  "
            << "(" << latLon[2] << ", " << latLon[3] << ")" << std::endl;

  // .............
  vtkNew<vtkFeatureLayer> featureLayer;
  featureLayer->SetName("test-scalarBar");

  // Note: Always add feature layer to the map *before* adding features
  map->AddLayer(featureLayer.GetPointer());

  // Scalar bar
  if (scalarBarType == 0)
  {
      vtkNew<vtkScalarBarFeature> sb;
      sb->SetLookupTable(lut.GetPointer());
      sb->SetTitle("Altitude");
      featureLayer->AddFeature(sb.GetPointer());
  }
  else if (scalarBarType == 1)
  {
      vtkNew<vtkParaViewScalarBarFeature> sb;
      sb->SetLookupTable(lut.GetPointer());
      sb->SetTitle("Altitude");
      featureLayer->AddFeature(sb.GetPointer());
  }

  // Path
  vtkNew<vtkPolydataFeature> pathFeature;
  pathFeature->GetActor()->GetProperty()->SetLineWidth(3.0);
  pathFeature->GetActor()->GetProperty()->SetPointSize(1.0);
  pathFeature->GetMapper()->SetInputData(path.GetPointer());
  //feature->GetActor()->GetProperty()->SetOpacity(0.5);
  featureLayer->AddFeature(pathFeature.GetPointer());

  map->Draw();

  // Set callbacks
  vtkNew<PickCallback> pickCallback;
  pickCallback->SetMap(map.GetPointer());
  /*map->AddObserver(vtkInteractorStyleGeoMap::DisplayClickCompleteEvent, pickCallback.GetPointer());
  map->AddObserver(vtkInteractorStyleGeoMap::DisplayDrawCompleteEvent, pickCallback.GetPointer());
  map->AddObserver(vtkInteractorStyleGeoMap::SelectionCompleteEvent, pickCallback.GetPointer());
  map->AddObserver(vtkInteractorStyleGeoMap::ZoomCompleteEvent, pickCallback.GetPointer());*/
  map->AddObserver(vtkInteractorStyleGeoMap::RightButtonCompleteEvent, pickCallback.GetPointer());

  intr->Start();

  //map->Print(std::cout);
  osmLayer->Delete();

  // tests...
  //featureLayer->RemoveFeature(scalarBar.GetPointer());
  //featureLayer->RemoveFeature(pathFeature.GetPointer());

  // global libcurl cleanup
  curl_global_cleanup();

  return 0;
}
