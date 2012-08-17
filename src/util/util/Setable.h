#ifndef SETABLE_H
#define SETABLE_H

/*
* Simpatico - Simulation Package for Polymeric and Molecular Liquids
*
* Copyright 2012, David Morse (morse012@umn.edu)
* Distributed under the terms of the GNU General Public License.
*/

#include <util/global.h>

namespace Util
{

   /**
   * Template for a value that can be declared null.
   *
   * \ingroup Param_Module
   */
   template <class T>
   class Setable
   {

   public:

      /**
      * Default constructor.
      */
      Setable()
       : value_(),
         isSet_(false)
      {}

      /**
      * Copy constructor.
      *
      * \param other Setable object being copied.
      */
      Setable(const Setable<T>& other)
       : value_(other.value_),
         isSet_(other.isSet_)
      {}

      /**
      * Construct from T value (explciit).
      *
      * \param value value of wrapped object.
      */
      explicit Setable(const T& value)
       : value_(value),
         isSet_(true)
      {}

      /**
      * Assignment.
      */ 
      Setable<T>& operator = (const Setable<T>& other)
      {
         if (this != &other) {
            value_ = other.value_;
            isSet_ = other.isSet_;
         }
         return *this;
      }

      /**
      * Assignment from value.
      */ 
      Setable<T>& operator = (const T& value)
      {
         value_ = value;
         isSet_ = true;
         return *this;
      }

      /**
      * Set the value.
      */ 
      void set(const T& value)
      {
         value_ = value;
         isSet_ = true;
      }

      /**
      * Unset the value (mark as unknown).
      */
      void unset()
      {  isSet_ = false; }

      /**
      * Is this object set (is the value known)?
      */
      bool isSet() const
      {  return isSet_; }

      /**
      * Return value (if set).
      *
      * Throws an Exception if value is not set.
      */
      const T& value() const
      {
         if (!isSet_) {
            UTIL_THROW("Attempt to return unknown value.");
         }  
         return value_; 
      }

      #if 0
      /**
      * Conversion to T value, if not null.
      *
      * Throws an exception if this object is null.
      */
      operator T ()
      {
         if (!isSet_) {
            UTIL_THROW("Attempted conversion of unknown value");
         }
         return value_;
      }
      #endif

   private:

      /// Value of associated variable.
      T   value_;

      /// True if value is known (set), false if unknown (unset).
      bool isSet_;

   };

} 
#endif
