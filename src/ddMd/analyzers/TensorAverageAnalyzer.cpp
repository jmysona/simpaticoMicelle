/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2010 - 2014, The Regents of the University of Minnesota
* Distributed under the terms of the GNU General Public License.
*/

#include "TensorAverageAnalyzer.h"
#include <ddMd/simulation/Simulation.h>
#include <util/accumulators/TensorAverage.h>   
#include <util/format/Int.h>
#include <util/format/Dbl.h>
#include <util/mpi/MpiLoader.h>
#include <util/misc/ioUtil.h>

#include <sstream>

namespace DdMd
{

   using namespace Util;

   /*
   * Constructor.
   */
   TensorAverageAnalyzer::TensorAverageAnalyzer(Simulation& simulation) 
    : Analyzer(simulation),
      outputFile_(),
      accumulatorPtr_(0),
      nSamplePerBlock_(1),
      isInitialized_(false)
   {  setClassName("TensorAverageAnalyzer"); }

   /*
   * Destructor.
   */
   TensorAverageAnalyzer::~TensorAverageAnalyzer() 
   {  
      if (accumulatorPtr_) {
         delete accumulatorPtr_;
      }
   }

   /*
   * Read interval and outputFileName. 
   */
   void TensorAverageAnalyzer::readParameters(std::istream& in) 
   {
      readInterval(in);
      readOutputFileName(in);
      read<int>(in,"nSamplePerBlock", nSamplePerBlock_);

      if (simulation().domain().isMaster()) {
         accumulatorPtr_ = new TensorAverage;
         accumulatorPtr_->setNSamplePerBlock(nSamplePerBlock_);
      }

      isInitialized_ = true;
   }

   /*
   * Load internal state from an archive.
   */
   void TensorAverageAnalyzer::loadParameters(Serializable::IArchive &ar)
   {
      loadInterval(ar);
      loadParameter<int>(ar, "nSamplePerBlock", nSamplePerBlock_);

      if (simulation().domain().isMaster()) {
         accumulatorPtr_ = new TensorAverage;
         accumulatorPtr_->loadParameters(ar);
      }

      if (nSamplePerBlock_ != accumulatorPtr_->nSamplePerBlock()) {
         UTIL_THROW("Inconsistent values of nSamplePerBlock");
      }

      isInitialized_ = true;
   }

   /*
   * Save internal state to an archive.
   */
   void TensorAverageAnalyzer::save(Serializable::OArchive &ar)
   {
      saveInterval(ar);
      saveOutputFileName(ar);

      if (simulation().domain().isMaster()){
         ar << *accumulatorPtr_;
      }
   }

   /*
   * Clear accumulator (do nothing on slave processors).
   */
   void TensorAverageAnalyzer::clear() 
   {   
      if (simulation().domain().isMaster()){ 
         accumulatorPtr_->clear();
      }
   }
 
   /*
   * Open outputfile
   */ 
   void TensorAverageAnalyzer::setup()
   {
      if (simulation().domain().isMaster()) {
         if (nSamplePerBlock_) {
            std::string filename  = outputFileName(".dat");
            simulation().fileMaster().openOutputFile(filename, outputFile_);
         }
      }
   }

   /*
   * Compute value.
   */
   void TensorAverageAnalyzer::sample(long iStep) 
   {
      if (isAtInterval(iStep))  {
         compute();
         if (simulation().domain().isMaster()) {
            double data = value();
            accumulatorPtr_->sample(data);
            if (nSamplePerBlock_ > 0 && accumulatorPtr_->isBlockComplete()) {
               int beginStep = iStep - (nSamplePerBlock_ - 1)*interval();
               outputFile_ << Int(beginStep) << "  ";
               double value;
               int i, j, k;
               k = 0;
               for (i = 0; i < Dimension; ++i) {
                  for (j = 0; j < Dimension; ++j) {
                     value << (*accumulatorPtr_)(i, j).blockAverage();
                     outputFile_ << Dbl(value) << "  ";
                  }
               }
               outputFile_ << "\n";
            }
         }
      }
   }

   /*
   * Output results to file after simulation is completed.
   */
   void TensorAverageAnalyzer::output()
   {
      if (simulation().domain().isMaster()) {
         // Close data (*.dat) file, if any
         if (outputFile_.is_open()) {
            outputFile_.close();
         }

         // Write parameter (*.prm) file
         FileMaster& fileMaster = simulation().fileMaster();
         fileMaster.openOutputFile(outputFileName(".prm"), outputFile_);
         ParamComposite::writeParam(outputFile_);
         outputFile_.close();

         // Write average (*.ave) file
         fileMaster.openOutputFile(outputFileName(".ave"), outputFile_);
         int i, j, k;
         k = 0;
         for (i = 0; i < Dimension; ++i) {
            for (j = 0; j < Dimension; ++j) {
               (*accumulatorPtr_)(i, j).output(outputFile_);
               outputFile_ << "\n";
            }
         }
         outputFile_.close();
      }
   }

}