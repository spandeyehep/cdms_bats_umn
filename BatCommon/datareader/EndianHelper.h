//////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: EndianHelper
//Authors: L. Hsu
//Description: Simple class to perform endian checks and byte flips.     
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef ENDIANHELPER_H
#define ENDIANHELPER_H

#include "stdint.h"

using namespace std;

//!very small helper class for raw data reading
class EndianHelper 
{
   public:

      static const int kLittle = -1;
      static const int kBig = 1;

      //for byte manipulation
      static int GuessMyEndianness();
      static uint32_t Swap4ByteWord(const uint32_t& word);

   private:
                 
};

#endif /* ENDIANHELPER_H */
