#ifndef BOND_FACTORY_CPP
#define BOND_FACTORY_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include <ddMd/potentials/bond/BondFactory.h>
#include <ddMd/system/System.h>

// BondPotential interface and implementation classes
#include <ddMd/potentials/bond/BondPotential.h>
#include <ddMd/potentials/bond/BondPotentialImpl.h>

// Bond interaction classes
#include <ddMd/potentials/bond/HarmonicBond.h>
#include <ddMd/potentials/bond/HarmonicL0Bond.h>
//#include <ddMd/potentials/bond/FeneBond.h>
//#include <ddMd/potentials/bond/CompositeBond.h>

//#include <ddMd/potentials/pair/DpdPair.h>
//#include <ddMd/potentials/pair/LJPair.h>

namespace DdMd
{

   /**
   * Default constructor.
   */
   BondFactory::BondFactory(System& system)
    : Factory<BondPotential>(),
      systemPtr_(&system)
   {}

   /*
   * Return a pointer to a new BondPotential, if possible.
   */
   BondPotential* 
   BondFactory::factory(const std::string& name) const
   {
      BondPotential* ptr = 0;

      // Try subfactories first.
      ptr = trySubfactories(name);
      if (ptr) return ptr;

      if (name == "HarmonicBond") {
         ptr = new BondPotentialImpl<HarmonicBond>(*systemPtr_);
      } else
      if (name == "HarmonicL0Bond") {
         ptr = new BondPotentialImpl<HarmonicL0Bond>(*systemPtr_);
      } //else
      //if (name == "FeneBond") {
      //   ptr = new BondPotentialImpl<FeneBond>(*systemPtr_);
      //} else
      //if (name == "CompositeBond<HarmonicL0Bond,DpdPair>") {
      //   ptr = new BondPotentialImpl< CompositeBond<HarmonicL0Bond, DpdPair> >(*systemPtr_);
      //}
      return ptr;
   }

}
#endif
