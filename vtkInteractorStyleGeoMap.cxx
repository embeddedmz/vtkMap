/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInteractorStyleMap.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

   This software is distributed WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkInteractorStyleGeoMap.h"
#include "vtkMap.h"

//#include "vtkVgRendererUtils.h"

// VTK includes.
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCommand.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkUnsignedCharArray.h>

vtkStandardNewMacro(vtkInteractorStyleGeoMap);

//-----------------------------------------------------------------------------
vtkInteractorStyleGeoMap::vtkInteractorStyleGeoMap() :
  vtkInteractorStyleRubberBand2D()
{
  this->Map = NULL;
  this->AllowPanning = 1;
  this->RubberBandMode = ZoomMode;
  this->RubberBandSelectionWithCtrlKey = 0;
  this->LeftButtonIsMiddleButton = false;
  this->RubberBandActor = 0;
  this->RubberBandPoints = 0;
}

//-----------------------------------------------------------------------------
vtkInteractorStyleGeoMap::~vtkInteractorStyleGeoMap()
{
  if (this->RubberBandActor)
    {
    this->RubberBandActor->Delete();
    }
}

//-----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnKeyPress()
{
  switch (this->Interactor->GetKeyCode())
    {
    case 'a':
    case 'A':
      this->InvokeEvent(KeyPressEvent_A);
      break;
    case 'r':
    case 'R':
      this->InvokeEvent(KeyPressEvent_R);
      break;
    case 'z':
    case 'Z':
      this->InvokeEvent(KeyPressEvent_Z);
      break;
    case 's':
    case 'S':
      this->InvokeEvent(KeyPressEvent_S);
      break;
    case 'p':
    case 'P':
      this->InvokeEvent(KeyPressEvent_P);
      break;
    default:
      if (strcmp(this->Interactor->GetKeySym(), "Delete") == 0)
        {
        this->InvokeEvent(KeyPressEvent_Delete);
        break;
        }
    }
}

//-----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnChar()
{
  // FIXME - may break vsPlay
  // hide ResetCamera
  switch (this->Interactor->GetKeyCode())
    {
    case 'r' :
    case 'R' :
      return;
    default:
      this->Superclass::OnChar();
    }
}

//-----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnLeftButtonDown()
{
  if (this->Interactor->GetShiftKey())
    {
    this->LeftButtonIsMiddleButton = true;
    this->Superclass::OnMiddleButtonDown();
    return;
    }
  this->LeftButtonIsMiddleButton = false;

  if (this->RubberBandMode == vtkInteractorStyleGeoMap::DisabledMode &&
      !this->Interactor->GetControlKey())
    {
    return;
    }

  // fall back to built-in rubberband drawing if no renderer was given
  if (!this->Map->GetRenderer())
    {
    this->Superclass::OnLeftButtonDown();
    return;
    }

  if (this->Interaction != NONE)
    {
    return;
    }

  this->Interaction = SELECTING;
  vtkRenderer *renderer = this->Map->GetRenderer();

  // initialize the rubberband actor on first use
  if (!this->RubberBandActor)
    {
    this->RubberBandActor = vtkActor2D::New();
    this->RubberBandPoints = vtkPoints::New();
    vtkPolyData* PD  = vtkPolyData::New();
    vtkCellArray* CA = vtkCellArray::New();
    vtkCellArray* CA2 = vtkCellArray::New();
    vtkPolyDataMapper2D* PDM = vtkPolyDataMapper2D::New();
    vtkUnsignedCharArray* UCA = vtkUnsignedCharArray::New();

    this->RubberBandPoints->SetNumberOfPoints(4);

    vtkIdType ids[] = { 0, 1, 2, 3, 0 };
    CA2->InsertNextCell(5, ids);
    CA->InsertNextCell(4, ids);

    UCA->SetNumberOfComponents(4);
    UCA->SetName("Colors");

    unsigned char color[]     = { 200, 230, 250,  50 };
    unsigned char edgeColor[] = {  60, 173, 255, 255 };
    UCA->InsertNextTupleValue(edgeColor);
    UCA->InsertNextTupleValue(color);

    PD->GetCellData()->SetScalars(UCA);
    PD->SetPoints(this->RubberBandPoints);
    PD->SetPolys(CA);
    PD->SetLines(CA2);
    PDM->SetInputData(PD);

    this->RubberBandActor->SetMapper(PDM);

    renderer->AddViewProp(this->RubberBandActor);

    CA->FastDelete();
    CA2->FastDelete();
    UCA->FastDelete();
    PD->FastDelete();
    PDM->FastDelete();
    this->RubberBandPoints->FastDelete();
    }
  else
    {
    this->RubberBandActor->VisibilityOn();

    // Our actor may have been removed since it isn't in the scene graph.
    // Don't bother checking if it has already been added, since the renderer
    // will do that anyways.
    renderer->AddViewProp(this->RubberBandActor);
    }

  this->StartPosition[0] = this->Interactor->GetEventPosition()[0];
  this->StartPosition[1] = this->Interactor->GetEventPosition()[1];
  this->EndPosition[0] = this->StartPosition[0];
  this->EndPosition[1] = this->StartPosition[1];

  double pos[] =
    {
    this->StartPosition[0] + 0.5,
    this->StartPosition[1] + 0.5,
    0.0
    };

  this->RubberBandPoints->SetPoint(0, pos);
  this->RubberBandPoints->SetPoint(1, pos);
  this->RubberBandPoints->SetPoint(2, pos);
  this->RubberBandPoints->SetPoint(3, pos);

  vtkPolyDataMapper2D::SafeDownCast(this->RubberBandActor->GetMapper())
  ->GetInput()->Modified();

  this->SetCurrentRenderer(renderer);
  this->InvokeEvent(vtkCommand::StartInteractionEvent);
  this->GetInteractor()->Render();
}

//-----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnLeftButtonUp()
{
  if (this->LeftButtonIsMiddleButton)
    {
    this->LeftButtonIsMiddleButton = false;
    this->Superclass::OnMiddleButtonUp();
    return;
    }

  if (!this->RubberBandActor)
    {
    this->Superclass::OnLeftButtonUp();
    return;
    }

  if (this->Interaction != SELECTING)
    {
    return;
    }

  this->RubberBandActor->VisibilityOff();

  int area = (this->EndPosition[0] - this->StartPosition[0]) *
             (this->EndPosition[1] - this->StartPosition[1]);

  // just a left click?
  if (this->RubberBandMode == DisabledMode ||
      this->RubberBandMode == DisplayOnlyMode ||
      area == 0)
    {
    this->InvokeEvent(LeftClickEvent);
    }
  // don't zoom or select for small rubberband; probably unintentional
  else if (abs(area) > 25)
    {
    // ZoomMode and NOT selection instead because of Ctrl modifier
    if (this->RubberBandMode == ZoomMode &&
        !(this->RubberBandSelectionWithCtrlKey &&
          this->Interactor->GetControlKey()))
      {
      this->Zoom();
      this->InvokeEvent(vtkCommand::EndInteractionEvent);
      }
    else
      {
      this->InvokeEvent(SelectionCompleteEvent);
      }
    }

  this->GetInteractor()->Render();
  this->Interaction = NONE;
}

//--------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnRightButtonDown()
{
  this->StartPosition[0] = this->Interactor->GetEventPosition()[0];
  this->StartPosition[1] = this->Interactor->GetEventPosition()[1];
  this->Superclass::OnRightButtonDown();
}

//--------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnRightButtonUp()
{
  int pos[2];
  this->Interactor->GetEventPosition(pos);

  if (abs(pos[0] - this->StartPosition[0]) < 5 &&
      abs(pos[1] - this->StartPosition[1]) < 5)
    {
    this->InvokeEvent(RightClickEvent);
    }

  this->Superclass::OnRightButtonUp();
}

//--------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnMiddleButtonDown()
{
  if (!this->AllowPanning)
    {
    // Do nothing.
    }
  else
    {
    this->Superclass::OnMiddleButtonDown();
    }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnMiddleButtonUp()
{
  if (!this->AllowPanning)
    {
    // Do nothing.
    }
  else
    {
    this->Superclass::OnMiddleButtonUp();
    }
}

//--------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnMouseMove()
{
  if (this->RubberBandMode == DisabledMode)
    {
    return;
    }

  if (this->Interaction != SELECTING || !this->RubberBandActor)
    {
    this->Superclass::OnMouseMove();
    return;
    }

  this->EndPosition[0] = this->Interactor->GetEventPosition()[0];
  this->EndPosition[1] = this->Interactor->GetEventPosition()[1];

  int* size = this->Interactor->GetRenderWindow()->GetSize();
  if (this->EndPosition[0] > (size[0] - 1))
    {
    this->EndPosition[0] = size[0] - 1;
    }
  if (this->EndPosition[0] < 0)
    {
    this->EndPosition[0] = 0;
    }
  if (this->EndPosition[1] > (size[1] - 1))
    {
    this->EndPosition[1] = size[1] - 1;
    }
  if (this->EndPosition[1] < 0)
    {
    this->EndPosition[1] = 0;
    }

  double pos1[] =
    {
    this->EndPosition[0] + 0.5,
    this->StartPosition[1] + 0.5,
    0.0
    };

  double pos2[] =
    {
    this->EndPosition[0] + 0.5,
    this->EndPosition[1] + 0.5,
    0.0
    };

  double pos3[] =
    {
    this->StartPosition[0] + 0.5,
    this->EndPosition[1] + 0.5,
    0.0
    };

  this->RubberBandPoints->SetPoint(1, pos1);
  this->RubberBandPoints->SetPoint(2, pos2);
  this->RubberBandPoints->SetPoint(3, pos3);

  vtkPolyDataMapper2D::SafeDownCast(this->RubberBandActor->GetMapper())
  ->GetInput()->Modified();

  this->InvokeEvent(vtkCommand::InteractionEvent);
  this->GetInteractor()->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnMouseWheelForward()
{
  if (this->Map)
    {
    int zoom = this->Map->GetZoom();
    if (zoom < 19)
      {
      zoom++;
      this->Map->SetZoom(zoom);
      this->SetCurrentRenderer(this->Map->GetRenderer());

      vtkCamera *camera = this->Map->GetRenderer()->GetActiveCamera();

      // Get current mouse coordinates (to make that screen position constant)
      int *pos = this->Interactor->GetEventPosition();

      // Get corresponding world coordinates
      double zoomCoords[4];
      this->ComputeDisplayToWorld(pos[0], pos[1], 0.0, zoomCoords);

      // Get camera coordinates before zooming in
      double cameraCoords[3];
      camera->GetPosition(cameraCoords);

      // Apply the dolly operation (move closer to focal point)
      camera->Dolly(2.0);

      // Get new camera coordinates
      double nextCameraCoords[3];
      camera->GetPosition(nextCameraCoords);

      // Adjust xy position to be proportional to change in z
      // That way, the zoom point remains stationary
      const double f = 0.5;   // fraction that camera moved closer to origin
      double losVector[3];  // line-of-sight vector, from camera to zoomCoords
      vtkMath::Subtract(zoomCoords, cameraCoords, losVector);
      vtkMath::Normalize(losVector);
      vtkMath::MultiplyScalar(losVector, f * cameraCoords[2]);
      nextCameraCoords[0] = cameraCoords[0] + losVector[0];
      nextCameraCoords[1] = cameraCoords[1] + losVector[1];
      camera->SetPosition(nextCameraCoords);

      // Set same xy coords for the focal point
      nextCameraCoords[2] = 0.0;
      camera->SetFocalPoint(nextCameraCoords);

      // Redraw the map
      this->Map->Draw();
      }
    }
  this->Superclass::OnMouseWheelForward();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::OnMouseWheelBackward()
{
  if (this->Map)
    {
    int zoom = this->Map->GetZoom();
    if (zoom > 0)
      {
      zoom--;

      this->Map->SetZoom(zoom);
      this->SetCurrentRenderer(this->Map->GetRenderer());

      vtkCamera *camera = this->Map->GetRenderer()->GetActiveCamera();

      // Get current mouse coordinates (to make that screen position constant)
      int *pos = this->Interactor->GetEventPosition();

      // Get corresponding world coordinates
      double zoomCoords[4];
      this->ComputeDisplayToWorld(pos[0], pos[1], 0.0, zoomCoords);

      // Get camera coordinates before zooming out
      double cameraCoords[3];
      camera->GetPosition(cameraCoords);

      // Apply the dolly operation (move away from focal point)
      camera->Dolly(0.5);

      // Get new camera coordinates
      double nextCameraCoords[3];
      camera->GetPosition(nextCameraCoords);

      // Adjust xy position to be proportional to change in z
      // That way, the zoom point remains stationary
      double losVector[3];  // line-of-sight vector, from camera to zoomCoords
      vtkMath::Subtract(zoomCoords, cameraCoords, losVector);
      vtkMath::Normalize(losVector);
      vtkMath::MultiplyScalar(losVector, -1.0 * cameraCoords[2]);
      nextCameraCoords[0] = cameraCoords[0] + losVector[0];
      nextCameraCoords[1] = cameraCoords[1] + losVector[1];
      camera->SetPosition(nextCameraCoords);

      // Set same xy coords for the focal point
      nextCameraCoords[2] = 0.0;
      camera->SetFocalPoint(nextCameraCoords);

      // Redraw the map
      this->Map->Draw();
      }
    }
  this->Superclass::OnMouseWheelBackward();
}

//-----------------------------------------------------------------------------
// void vtkInteractorStyleGeoMap::ZoomToExtents(vtkRenderer* ren,
//     double extents[4])
// {
//   // vtkVgRendererUtils::ZoomToExtents2D(ren, extents);
//   vtkWarningMacro("Sorry - ZoomToExtents not implemented");
//   this->InvokeEvent(ZoomCompleteEvent);
// }

//-----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::SetMap(vtkMap *map)
{
  this->Map = map;
  this->SetCurrentRenderer(map->GetRenderer());
}

//-----------------------------------------------------------------------------
void vtkInteractorStyleGeoMap::Zoom()
{
  int width, height;
  width = abs(this->EndPosition[0] - this->StartPosition[0]);
  height = abs(this->EndPosition[1] - this->StartPosition[1]);

  // compute world position of lower left corner
  double rbmin[3];
  rbmin[0] = this->StartPosition[0] < this->EndPosition[0] ?
             this->StartPosition[0] : this->EndPosition[0];
  rbmin[1] = this->StartPosition[1] < this->EndPosition[1] ?
             this->StartPosition[1] : this->EndPosition[1];
  rbmin[2] = 0.0;

  this->CurrentRenderer->SetDisplayPoint(rbmin);
  this->CurrentRenderer->DisplayToView();
  this->CurrentRenderer->ViewToWorld();

  double invw;
  double worldRBMin[4];
  this->CurrentRenderer->GetWorldPoint(worldRBMin);
  invw = 1.0 / worldRBMin[3];
  worldRBMin[0] *= invw;
  worldRBMin[1] *= invw;

  // compute world position of upper right corner
  double rbmax[3];
  rbmax[0] = rbmin[0] + width;
  rbmax[1] = rbmin[1] + height;
  rbmax[2] = 0.0;

  this->CurrentRenderer->SetDisplayPoint(rbmax);
  this->CurrentRenderer->DisplayToView();
  this->CurrentRenderer->ViewToWorld();

  double worldRBMax[4];
  this->CurrentRenderer->GetWorldPoint(worldRBMax);
  invw = 1.0 / worldRBMax[3];
  worldRBMax[0] *= invw;
  worldRBMax[1] *= invw;

  double extents[] =
    {
    worldRBMin[0],
    worldRBMax[0],
    worldRBMin[1],
    worldRBMax[1]
    };

  // zoom
  //this->ZoomToExtents(this->CurrentRenderer, extents);
}
