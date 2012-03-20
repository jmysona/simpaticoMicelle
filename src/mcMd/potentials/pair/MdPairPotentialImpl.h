#ifndef INTER_NOPAIR
#ifndef MD_PAIR_POTENTIAL_IMPL_H
#define MD_PAIR_POTENTIAL_IMPL_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010, David Morse (morse@cems.umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include <mcMd/potentials/pair/MdPairPotential.h>
#include <mcMd/potentials/pair/McPairPotentialImpl.h>
#include <util/global.h>

namespace Util
{
   class Vector;
   class Tensor;
}

namespace McMd
{

   using namespace Util;

   class System;

   /**
   * Implementation template for an MdPairPotential.
   *
   * \ingroup Pair_Module
   */
   template <class Interaction>
   class MdPairPotentialImpl : public MdPairPotential
   {

   public:

      /** 
      * Constructor.
      */
      MdPairPotentialImpl(System& system);

      /** 
      * Constructor (copied from McPairPotentialImpl)
      */
      MdPairPotentialImpl(McPairPotentialImpl<Interaction>& other);

      /** 
      * Destructor.
      */
      virtual ~MdPairPotentialImpl();

      /**
      * Read pair potential interaction and pair list blocks.
      * 
      * This method reads the maxBoundary, PairList and pair potential 
      * Interaction parameter blocks, in  that order, and initializes an
      * internal PairList. Before calling the Interaction::readParam method,
      * it passes simulation().nAtomType() to Interaction::setNAtomType().
      *
      * \param in input parameter stream.
      */
      virtual void readParam(std::istream& in);

      /**
      * Return pair energy for a single pair.
      * 
      * \param rsq       square distance between atoms in pair
      * \param iAtomType atom type index of 1st atom
      * \param jAtomType atom type index of 2nd atom
      * \return energy of pair
      */
      virtual 
      double energy(double rsq, int iAtomType, int jAtomType) const;

      /**
      * Return force / separation for a single pair.
      *
      * \param rsq       square distance between atoms in pair
      * \param iAtomType atom type index of 1st atom
      * \param jAtomType atom type index of 2nd atom
      * \return repulsive force (< 0 if attractive) over distance
      */
      virtual 
      double forceOverR(double rsq, int iAtomType, int jAtomType) const;

      /**
      * Return maximum cutoff.
      */
      virtual double maxPairCutoff() const;

      /**
      * Return pair interaction class name (e.g., "LJPair").
      */
      virtual std::string interactionClassName() const;

      /**
      * Calculate the total nonBonded pair energy for this System.
      * 
      * Rebuilds the PairList if necessary before calculating energy.
      */
      virtual double energy();

      /**
      * Calculate non-bonded pair forces for all atoms in this System.
      *
      * Adds non-bonded pair forces to the current values of the
      * forces for all atoms in this system. Before calculating 
      * forces, the method checks if the pair list is current, 
      * and rebuilds it if necessary.
      */
      virtual void addForces();

      /**
      * Compute total nonbonded pressure
      *
      * \param stress (output) pressure.
      */
      virtual void computeStress(double& stress) const;

      /**
      * Compute x, y, z nonbonded pressures.
      *
      * \param stress (output) pressures.
      */
      virtual void computeStress(Util::Vector& stress) const;

      /**
      * Compute nonbonded stress tensor.
      *
      * \param stress (output) pressures.
      */
      virtual void computeStress(Util::Tensor& stress) const;

   protected:

      Interaction& interaction() const
      {  return *interactionPtr_; }

   private:
  
      Interaction* interactionPtr_;

      bool       isCopy_;
 
      template <typename T>
      void computeStressImpl(T& stress) const;

   };

}

#include <mcMd/simulation/System.h> 
#include <mcMd/simulation/Simulation.h> 
#include <mcMd/simulation/stress.h>
#include <mcMd/neighbor/PairIterator.h> 
#include <util/boundary/Boundary.h> 

#include <util/space/Dimension.h>
#include <util/space/Vector.h>
#include <util/space/Tensor.h>
#include <util/accumulators/setToZero.h>

#include <fstream>

namespace McMd
{

   using namespace Util;

   /* 
   * Default constructor.
   */
   template <class Interaction>
   MdPairPotentialImpl<Interaction>::MdPairPotentialImpl(System& system)
    : MdPairPotential(system),
      interactionPtr_(0),
      isCopy_(false)
   {  interactionPtr_ = new Interaction; }

   /* 
   * Constructor, copy from McPairPotentialImpl<Interaction>.
   */
   template <class Interaction>
   MdPairPotentialImpl<Interaction>::MdPairPotentialImpl(
                         McPairPotentialImpl<Interaction>& other)
    : MdPairPotential(other.system()),
      interactionPtr_(&other.interaction()),
      isCopy_(true)
   {}
 
   /* 
   * Destructor. 
   */
   template <class Interaction>
   MdPairPotentialImpl<Interaction>::~MdPairPotentialImpl() 
   {
      if (interactionPtr_ && !isCopy_) {
         delete interactionPtr_;
         interactionPtr_ = 0;
      }
   }

   template <class Interaction>
   void MdPairPotentialImpl<Interaction>::readParam(std::istream& in)
   {
      readBegin(in, "MdPairPotential");

      // Read pair potential parameters only if not a copy.
      // This block is not indented or surrounded by brackets.
      if (!isCopy_) {
         interaction().setNAtomType(simulation().nAtomType());
         bool nextIndent = false;
         readParamComposite(in, interaction(), nextIndent);
      }

      read<Boundary>(in, "maxBoundary", maxBoundary_);
      readParamComposite(in, pairList_);

      // Allocate the PairList 
      double cutoff = interaction().maxPairCutoff();
      pairList_.allocate(simulation().atomCapacity(), 
                         maxBoundary_, cutoff);

      readEnd(in);
   }

   /*
   * Return pair energy for a single pair.
   */
   template <class Interaction> double 
   MdPairPotentialImpl<Interaction>::energy(double rsq, 
                                        int iAtomType, int jAtomType) const
   { return interaction().energy(rsq, iAtomType, jAtomType); }

   /*
   * Return force / separation for a single pair.
   */
   template <class Interaction> double 
   MdPairPotentialImpl<Interaction>::forceOverR(double rsq, 
                                        int iAtomType, int jAtomType) const
   { return interaction().forceOverR(rsq, iAtomType, jAtomType); }

   /*
   * Return maximum cutoff.
   */
   template <class Interaction>
   double MdPairPotentialImpl<Interaction>::maxPairCutoff() const
   { return interaction().maxPairCutoff(); }

   /*
   * Return pair interaction class name.
   */
   template <class Interaction>
   std::string MdPairPotentialImpl<Interaction>::interactionClassName() const
   {  return interaction().className(); }

   /* 
   * Return nonBonded Pair interaction energy
   */
   template <class Interaction>
   double MdPairPotentialImpl<Interaction>::energy()
   {

      // Update PairList if necessary
      if (!isPairListCurrent()) {
         buildPairList();
      }

      // Loop over pairs
      PairIterator iter;
      Atom *atom0Ptr;
      Atom *atom1Ptr;
      double rsq;
      double energy=0;
      for (pairList_.begin(iter); iter.notEnd(); ++iter) {
         iter.getPair(atom0Ptr, atom1Ptr);
         rsq = boundary().
               distanceSq(atom0Ptr->position(), atom1Ptr->position());
         energy += interaction().
                   energy(rsq, atom0Ptr->typeId(), atom1Ptr->typeId());
      }

      return energy;
   } 
 
   /* 
   * Add nonBonded pair forces to atomic forces.
   */
   template <class Interaction>
   void MdPairPotentialImpl<Interaction>::addForces()
   {
      // Update PairList if necessary
      if (!isPairListCurrent()) {
         buildPairList();
      }

      // Loop over nonbonded neighbor pairs
      PairIterator iter;
      Vector       force;
      double       rsq;
      Atom        *atom0Ptr;
      Atom        *atom1Ptr;
      for (pairList_.begin(iter); iter.notEnd(); ++iter) {
         iter.getPair(atom0Ptr, atom1Ptr);
         rsq = boundary().
               distanceSq(atom0Ptr->position(), atom1Ptr->position(), force);
         force *= interaction().
                  forceOverR(rsq, atom0Ptr->typeId(), atom1Ptr->typeId());
         atom0Ptr->force() += force;
         atom1Ptr->force() -= force;
      }
   } 

   /* 
   * Add nonBonded pair forces to atomic forces.
   */
   template <class Interaction>
   template <typename T>
   void MdPairPotentialImpl<Interaction>::computeStressImpl(T& stress) const
   {

      Vector       dr;
      Vector       force;
      double       rsq;
      PairIterator iter;
      Atom        *atom1Ptr;
      Atom        *atom0Ptr;

      setToZero(stress);

      // Loop over nonbonded neighbor pairs
      for (pairList_.begin(iter); iter.notEnd(); ++iter) {
         iter.getPair(atom0Ptr, atom1Ptr);
         rsq = boundary().
               distanceSq(atom0Ptr->position(), atom1Ptr->position(), dr);
         force  = dr;
         force *= interaction().
                  forceOverR(rsq, atom0Ptr->typeId(), atom1Ptr->typeId());
         incrementPairStress(force, dr, stress);
      }

      // Normalize by volume 
      stress /= boundary().volume();
      normalizeStress(stress);

   }

   template <class Interaction>
   void MdPairPotentialImpl<Interaction>::computeStress(double& stress) 
        const
   {  computeStressImpl(stress); }

   template <class Interaction>
   void MdPairPotentialImpl<Interaction>::computeStress(Util::Vector& stress)
        const
   {  computeStressImpl(stress); }

   template <class Interaction>
   void MdPairPotentialImpl<Interaction>::computeStress(Util::Tensor& stress)
        const
   {  computeStressImpl(stress); }

}
#endif
#endif
