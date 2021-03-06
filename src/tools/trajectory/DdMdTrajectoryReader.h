#ifndef TOOLS_DDMD_TRAJECTORY_READER_H
#define TOOLS_DDMD_TRAJECTORY_READER_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2014, The Regents of the University of Minnesota
* Distributed under the terms of the GNU General Public License.
*/

#include <tools/trajectory/TrajectoryReader.h>  // base class

namespace Tools
{

   class Configuration;
   using namespace Util;

   /**
   * Reader for lammps dump trajectory file format.
   *
   * \ingroup Tools_Trajectory_Module
   */
   class DdMdTrajectoryReader  : public TrajectoryReader
   {

   public:

      /**
      * Constructor.
      *
      * \param configuration parent Configuration object
      */
      DdMdTrajectoryReader(Configuration& configuration);

      /**
      * Destructor.
      */
      virtual ~DdMdTrajectoryReader();

      /**
      * Read the header.
      *
      * \param file input file 
      */
      virtual void readHeader(std::ifstream& file);

      /**
      * Read a frame.
      *
      * \param file input file 
      * \return true if a frame was found, false if end of file
      */
      virtual bool readFrame(std::ifstream& file);

   private:

      // Number of atoms
      int nAtom_;

   };

}
#endif
