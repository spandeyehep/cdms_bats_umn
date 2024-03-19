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

#include <iostream>
#include <cstdio>

#include "EndianHelper.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////

int EndianHelper::GuessMyEndianness()
{
   short int testWord = 0x4321; //a short word

   char* firstByte = (char*) &testWord; //assign char ptr to point to the first byte 

   printf("First Byte = %#x \n", firstByte[0]); 

   //What is the first word?
   if(firstByte[0] == 0x21) return kLittle; //little endian (i.e. Intel)
    
   if(firstByte[0] == 0x43) return kBig;    //big endian (i.e. G4 Mac)

   return 0; //unknown result!
}

//////////////////////////////////////////////////////////////////////////////////////////////

uint32_t EndianHelper::Swap4ByteWord(const uint32_t& word)
{
   return (((word&0x000000FF)<<24) + ((word&0x0000FF00)<<8) +
	   ((word&0x00FF0000)>>8 ) + ((word&0xFF000000)>>24));
}
