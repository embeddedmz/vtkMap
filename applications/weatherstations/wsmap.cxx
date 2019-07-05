/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMap

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

   This software is distributed WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME wsmap.cxx - Weather station map demo program
// .SECTION Description
//

#include "qtWeatherStations.h"
#include <iostream>
#include <qapplication.h>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
  //std::cout << "Hello from wsmap" << std::endl;

  QApplication app(argc, argv);

  QCommandLineParser parser;
  QCommandLineOption p_opt({"p","provider"}, "Map tile provider ('osm' or 'bing')", "provider");
  parser.addHelpOption();
  parser.addOption(p_opt);
  parser.process(QCoreApplication::arguments());

  QString mapTileProvider = parser.value("provider");

  qtWeatherStations win(nullptr, (mapTileProvider == "bing") ? qtWeatherStations::Bing :
                                                               qtWeatherStations::OpenStreetMap);
  win.show();
  win.resize(1000, 800);
  win.drawMap();

  return app.exec();
}
