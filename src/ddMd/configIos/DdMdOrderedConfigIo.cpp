#ifndef DDMD_DDMD_CONFIG_IO_CPP
#define DDMD_DDMD_CONFIG_IO_CPP

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2012, David Morse (morse012@umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include "DdMdOrderedConfigIo.h"

#include <ddMd/simulation/Simulation.h>                 
#include <ddMd/communicate/Domain.h>   

#include <ddMd/storage/AtomStorage.h>               
#include <ddMd/storage/BondStorage.h>               
#ifdef INTER_ANGLE
#include <ddMd/storage/AngleStorage.h>               
#endif
#ifdef INTER_DIHEDRAL
#include <ddMd/storage/DihedralStorage.h>               
#endif

#include <ddMd/communicate/GroupCollector.tpp> 
#include <ddMd/communicate/GroupDistributor.tpp> 

#include <ddMd/communicate/Buffer.h> 
#include <ddMd/chemistry/Atom.h>
#include <ddMd/chemistry/Bond.h>
#include <ddMd/chemistry/MaskPolicy.h>
#include <util/space/Vector.h>
#include <util/mpi/MpiSendRecv.h>
#include <util/format/Int.h>
#include <util/format/Dbl.h>

namespace DdMd
{

   using namespace Util;

   /*
   * Constructor.
   */
   DdMdOrderedConfigIo::DdMdOrderedConfigIo()
    : ConfigIo()
   {  setClassName("DdMdOrderedConfigIo"); }

   /*
   * Constructor.
   */
   DdMdOrderedConfigIo::DdMdOrderedConfigIo(Simulation& simulation)
    : ConfigIo(simulation)
   {  setClassName("DdMdOrderedConfigIo"); }

   /*
   * Private method to read Group<N> objects.
   */
   template <int N>
   int DdMdOrderedConfigIo::readGroups(std::istream& file, 
                  const char* sectionLabel,
                  const char* nGroupLabel,
                  GroupDistributor<N>& distributor) 
   {
      int nGroup;  // Total number of groups in file
      if (domain().isMaster()) {  
         file >> Label(sectionLabel);
         file >> Label(nGroupLabel) >> nGroup;
         Group<N>* groupPtr;
         int i, j, k;
         distributor.setup();
         for (i = 0; i < nGroup; ++i) {
            groupPtr = distributor.newPtr();
            file >> *groupPtr;
            for (j = 0; j < 2; ++j) {
               k = groupPtr->atomId(j);
            }
            distributor.add();
         }
         // Send any groups not sent previously.
         distributor.send();
      } else { // If I am not the master processor
         // Receive all groups into BondStorage
         distributor.receive();
      }
      return nGroup;
   }

   /*
   * Read a configuration file.
   */
   void DdMdOrderedConfigIo::readConfig(std::istream& file, MaskPolicy maskPolicy)
   {
      // Precondition
      if (atomStorage().nAtom()) {
         UTIL_THROW("Atom storage is not empty (has local atoms)");
      }
      if (atomStorage().nGhost()) {
         UTIL_THROW("Atom storage is not empty (has ghost atoms)");
      }
      if (UTIL_ORTHOGONAL) {
         if (!atomStorage().isCartesian()) {
            UTIL_THROW("Atom storage must use Cartesian coordinates");
         }
      } else {
         if (atomStorage().isCartesian()) {
            UTIL_THROW("Atom storage must use generalized coordinates");
         }
      }

      // Read and broadcast boundary
      if (domain().isMaster()) {  
         file >> Label("BOUNDARY");
         file >> boundary();
      }
      #if UTIL_MPI
      bcast(domain().communicator(), boundary(), 0);
      #endif

      // Atoms 
      int nAtom;  // Total number of atoms in file
      if (domain().isMaster()) {  

         // Read and distribute atoms
         file >> Label("ATOMS");

         // Read number of atoms
         file >> Label("nAtom") >> nAtom;

         int totalAtomCapacity = atomStorage().totalAtomCapacity();

         #if UTIL_MPI
         //Initialize the send buffer.
         atomDistributor().setup();
         #endif

         // Read atoms
         Vector r;
         Atom*  atomPtr;
         int id;
         int typeId;
         int rank;
         for(int i = 0; i < nAtom; ++i) {

            // Get pointer to new atom in distributor memory.
            atomPtr = atomDistributor().newAtomPtr();

            file >> id >> typeId;
            if (id < 0 || id >= totalAtomCapacity) {
               UTIL_THROW("Invalid atom id");
            }
            atomPtr->setId(id);
            atomPtr->setTypeId(typeId);
            file >> r;
            if (UTIL_ORTHOGONAL) {
               atomPtr->position() = r;
            } else {
               boundary().transformCartToGen(r, atomPtr->position());
            }
            file >> atomPtr->velocity();

            // Add atom to list for sending.
            rank = atomDistributor().addAtom();

         }

         // Send any atoms not sent previously.
         atomDistributor().send();

      } else { // If I am not the master processor
         atomDistributor().receive();
      }

      // Check that all atoms are accounted for after distribution.
      {
         int nAtomLocal = atomStorage().nAtom();
         int nAtomAll;
         #ifdef UTIL_MPI
         domain().communicator().Reduce(&nAtomLocal, &nAtomAll, 1, 
                                        MPI::INT, MPI::SUM, 0);
         #else
         nAtomAll = nAtomLocal;
         #endif
         if (domain().isMaster()) {
            if (nAtomAll != nAtom) {
               UTIL_THROW("nAtomAll != nAtom after distribution");
            }
         }
      }

      bool hasGhosts = false;

      if (bondStorage().capacity()) {
         readGroups<2>(file, "BONDS", "nBond", bondDistributor());
         bondStorage().isValid(atomStorage(), domain().communicator(), hasGhosts);
         // Set atom "mask" values
         if (maskPolicy == MaskBonded) {
            setAtomMasks();
         }
      }

      #ifdef INTER_ANGLE
      if (angleStorage().capacity()) {
         readGroups<3>(file, "ANGLES", "nAngle", angleDistributor());
         angleStorage().isValid(atomStorage(), domain().communicator(), 
                                hasGhosts);
      }
      #endif

      #ifdef INTER_DIHEDRAL
      if (dihedralStorage().capacity()) {
         readGroups<4>(file, "DIHEDRALS", "nDihedral", dihedralDistributor());
         dihedralStorage().isValid(atomStorage(), domain().communicator(), 
                                   hasGhosts);
      }
      #endif

   }

   /*
   * Private method to write Group<N> objects.
   */
   template <int N>
   int DdMdOrderedConfigIo::writeGroups(std::ostream& file, 
                  const char* sectionLabel,
                  const char* nGroupLabel,
                  GroupStorage<N>& storage,
                  GroupCollector<N>& collector) 
   {
      Group<N>* groupPtr;
      int       nGroup;
      storage.computeNTotal(domain().communicator());
      nGroup = storage.nTotal();
      if (domain().isMaster()) {  
         file << std::endl;
         file << sectionLabel << std::endl;
         file << nGroupLabel << Int(nGroup, 10) << std::endl;
         collector.setup();
         groupPtr = collector.nextPtr();
         while (groupPtr) {
            file << *groupPtr << std::endl;
            groupPtr = collector.nextPtr();
         }
      } else { 
         collector.send();
      }
      return nGroup;
   }

   /* 
   * Write the configuration file.
   */
   void DdMdOrderedConfigIo::writeConfig(std::ostream& file)
   {

      // Write Boundary dimensions
      if (domain().isMaster()) {
         file << "BOUNDARY" << std::endl << std::endl;
         file << boundary() << std::endl;
         file << std::endl;
      }

      // Atoms
      atomStorage().computeNAtomTotal(domain().communicator());
      if (domain().isMaster()) { 
         int nAtom = atomStorage().nAtomTotal();

         file << "ATOMS" << std::endl;
         file << "nAtom" << Int(nAtom, 10) << std::endl;

         // Set up array of nAtom default-constructed elements.
         IoAtom atom;
         atoms_.reserve(nAtom);
         atoms_.clear();
         atoms_.insert(atoms_.end(), nAtom, atom);

         // Collect unordered atoms and store in order in atoms_ .
         atomCollector().setup();
         Atom* atomPtr = atomCollector().nextPtr();
         int id;
         int n = 0;
         Vector r;
         while (atomPtr) {
            id = atomPtr->id();
            if (UTIL_ORTHOGONAL) {
               atoms_[id].position = atomPtr->position();
            } else {
               boundary().transformGenToCart(atomPtr->position(), atoms_[id].position);
            }
            atoms_[id].velocity = atomPtr->velocity();
            atoms_[id].typeId = atomPtr->typeId();
            atoms_[id].id = id;
            atomPtr = atomCollector().nextPtr();
            ++n;
         } 
         if (n != nAtom) {
            UTIL_THROW("Something is rotten in Denmark");
         }

         // Iterate through atoms_ array to write atoms data.
         for (id = 0; id < nAtom; ++id) {
            if (id != atoms_[id].id) {
               UTIL_THROW("Something is rotten in Denmark");
            }
            file << Int(id, 10) << Int(atoms_[id].typeId, 6)
                 << atoms_[id].position << std::endl
                 << "                " << atoms_[id].velocity;
            file << std::endl;
         }

      } else { 
         atomCollector().send();
      }

      // Write the groups
      if (bondStorage().capacity()) {
         writeGroups<2>(file, "BONDS", "nBond", bondStorage(), bondCollector());
      }
      #ifdef INTER_ANGLE
      if (angleStorage().capacity()) {
         writeGroups<3>(file, "ANGLES", "nAngle", angleStorage(), angleCollector());
      }
      #endif
      #ifdef INTER_DIHEDRAL
      if (dihedralStorage().capacity()) {
         writeGroups<4>(file, "DIHEDRALS", "nDihedral", dihedralStorage(), dihedralCollector());
      }
      #endif

   }
 
}
#endif
