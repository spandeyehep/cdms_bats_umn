///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: DetectorConfigData
//Author: L. Hsu
//Description:  This is a container class which stores data from the External (GPS) record in the raw data file.   
//The external data are filled directly by this class, which knows how to parse the raw record.
//
//File Import By: L. Hsu
//Creation Date: Nov. 22, 2010
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DETECTORCONFIGDATA_H
#define DETECTORCONFIGDATA_H

#include "zlib.h"
#include <map>
#include <vector>

#include "BatRootTypes.h"
#include "EndianHelper.h"

//!This class reads the external record and stores the info
class DetectorConfigData 
{
   public:

      DetectorConfigData();  //constructor

      ~DetectorConfigData(); //destructor

      //Get Data Members

      //returns the whole map of detector config records
      map< int, map<string, double> > GetDetectorConfigMap() { return fDetectorConfigMap; }
      bool                       IsFilled() { return fMapIsFilled; }

      // Allow modification of detector config
      bool                       IsModified() { return fMapIsModified; }
      void SetDetectorConfigMap(map< int, map<string, double> > newConfigMap, bool mapIsModified) 
                      {fDetectorConfigMap = newConfigMap; fMapIsModified = mapIsModified; return;};


      //For reading raw data
      void Reset();             //clears data members for next filling
      void ReadDetectorConfigRecord(gzFile& localRawDataPtr, uint32_t recordLength, bool dispHeader);        
      
      //flipping bytes for opposite endianness
      void SetFlipBytes(bool flipCheck) { fFlipBytes = flipCheck; return; }
      
   private:
      
      //member data

      //key is detector code, vector is list of detector configuration parameters
      map< int, map<string, double> > fDetectorConfigMap;   //key1 = detector code, key2 = config param name
      bool                       fMapIsFilled;              //initilized to false
      bool                       fMapIsModified;            //initilized to false

      //endian reading and bitwise manipulations
      bool        fFlipBytes;              //initilized to false


};

#endif /* DETECTORCONFIGDATA_H */
