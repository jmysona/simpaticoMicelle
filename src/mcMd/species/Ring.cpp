/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2012, The Regents of the University of Minnesota
* Distributed under the terms of the GNU General Public License.
*/

#include "Ring.h"

namespace McMd
{

   using namespace Util;

   /*
   * Constructor.
   */
   Ring::Ring() 
   : Species()
   {}

   /*
   * Build the chemical structure of a flexible loop molecule.
   */
   void Ring::buildRing() 
   {
      // Preconditions
      if (nAtom() < 3) 
         UTIL_THROW("nAtom < 3");
      if (nBond() != nAtom()) 
         UTIL_THROW("nBond != nAtom");
      #ifdef INTER_ANGLE
      if (nAngle() != nAtom()) 
         UTIL_THROW("nAngle != nAtom");
      #endif
      #ifdef INTER_DIHEDRAL
      if (nDihedral() != nAtom()) 
         UTIL_THROW("nDihedral != nAtom");
      #endif


      int i; 

      allocate();

      // Set Atom Type Ids
      for (i=0; i < nAtom(); ++i) {
         setAtomType(i, calculateAtomTypeId(i));
      }

      // Build Bonds.
      for (i=0; i < nBond(); ++i) {
         makeBond(i, i, (i+1)%nAtom(), calculateBondTypeId(i));
      }

      #ifdef INTER_ANGLE
      // Build Angles.
      for (i = 0; i < nAngle(); ++i) {
         makeAngle(i, i, (i+1)%nAtom(), (i+2)%nAtom(), calculateAngleTypeId(i));
      }
      #endif

      #ifdef INTER_DIHEDRAL
      // Build Dihedrals. 
      for (i = 0; i < nDihedral(); ++i) {
         makeDihedral(i, i, (i+1)%nAtom(), (i+2)%nAtom(), (i+3)%nAtom(),
                        calculateDihedralTypeId(i));
      }
      #endif

   }
   
} 
