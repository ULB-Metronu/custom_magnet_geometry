/* 
Beam Delivery Simulation (BDSIM) Copyright (C) Royal Holloway, 
University of London 2001 - 2020.

This file is part of BDSIM.

BDSIM is free software: you can redistribute it and/or modify 
it under the terms of the GNU General Public License as published 
by the Free Software Foundation version 3 of the License.

BDSIM is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BDSIM.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef UDIPOLE_H
#define UDIPOLE_H

#include "BDSAcceleratorComponent.hh"

#ifdef USE_GDML
#include "BDSGeometryFactoryGDML.hh"
#include "G4GDMLParser.hh"
#endif

#include "globals.hh" // geant4 types / globals

// forward declare things that are only needed as pointers
class G4LogicalVolume;
class G4Material;

/**
 * @brief Custom dipole component.
 * 
 * A pipe then dipole yoke (straight) with custom vacuum
 * chamber.
 * 
 * Note all comments are in Doxygen style for later use.
 *
 * @author Someone
 */

class UDipole: public BDSAcceleratorComponent
{
public:
  /// Construct as we wish.
  UDipole(G4String name,
	  G4double bFieldIn,
	  G4String paramsIn);
  
  virtual ~UDipole(); ///< Remember destructor has to be virtual.

protected:
  /// The minimum bit you should implement to build some geometry. This
  /// should reassign the member containerLogicalVolume and containerSolid
  /// and should be a shape with flat incoming / outgoing ends that's big
  /// enough to encompass the whole object.
  virtual void BuildContainerLogicalVolume(){;};

  /// The more detailed build method. The default implementation in BDSIM
  /// calls BuildContainerLogicalVolume() and then sets some visualisation
  /// attributes for the container. We override it here to a) call the base
  /// class versioni then complete our own more detailed construction of
  /// the required geometry. Here, this function calls a series of functions
  /// to do small bits of construction.
  virtual void Build();

private:
  // Remove the default constructor as we only ever want to use our one.
  UDipole() = delete;

  // Split the construction into steps for our own sanity.
  void BuildMagnet();
  void BuildField(G4String);
  void SetExtents();
  void Setvolumesforfields();

  // Parameters that determine how udipole is built that need to get
  // from the constructor to Build which is called later on.
  G4double bField;
  G4double horizontalWidth;

  G4String geometryGdmlPath;
  BDSGeometryFactoryGDML* gdml;

  G4String yokeFieldMap;

  std::vector<G4LogicalVolume*> magnet_pipe_volumes;
  std::vector<G4LogicalVolume*> magnet_yoke_volumes;
  std::vector<G4LogicalVolume*> magnet_exteriors_volumes;
};

#endif
