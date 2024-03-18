//////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Namespace: BatCalibTypes
//Authors: L. Hsu
//Description: A Namespace for BatCalib constants and typedefs.     
//
//File Import By: L. Hsu
//Creation Date: Dec. 23, 2009
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef BATCALIBTYPES_H
#define BATCALIBTYPES_H

#include "stdint.h"

using namespace std;

typedef unsigned int uint;

namespace BatCalibTypes
{   

   // =========== detector types ============

   static const int kBlipDetType = 1; //blips 
   static const int kFLIPDetType = 2; //flips 
   static const int kVetoDetType = 3; //veto 
   static const int kZIPDetType  = 4; //zips 
   static const int kmZIPDetType  = 5; //mzips
   static const int kDualEndcapDetType  = 6; //ganged top and bottom endcaps 
   static const int kEndcapDetType  = 7; //separate endcaps 

   static const int kiZIPSoudanBiFold   = 10;  //installed in Soudan iZIP test run, semi-circle inner phonons 
   static const int kiZIPSoudanTriFold  = 11;  //installed in Soudan iZIP test run, mercedes inner phonons

   static const int kCDMSliteIDetType  = 21;  //CDMSLITE I detector


   // =========== channel name mappings ============
   

   // === Soudan iZIPs ===

   static const string kiZIPSoudanChannelNames[12] = {"QIS1", "QOS1", "PAS1", "PBS1", "PCS1", "PDS1",
							     "QIS2", "QOS2", "PAS2", "PBS2", "PCS2", "PDS2"};
   static const string kiZIPSoudanPhononChan[8] = {"PAS1", "PBS1", "PCS1", "PDS1", "PAS2", "PBS2", "PCS2", "PDS2"};
   static const string kiZIPSoudanChargeChan[4] = {"QIS1", "QOS1", "QIS2", "QOS2"};

   static const int kiZIPSoudanNAllChan = 12;
   static const int kiZIPSoudanNPhononChan = 8;
   static const int kiZIPSoudanNChargeChan = 4;
   static const int kiZIPSoudanNCrossTalk  = 0;

   //the names after calibration
   static const string kiZIPSoudanPhononCal[8] = {"pa1", "pb1", "pc1", "pd1",
						  "pa2", "pb2", "pc2", "pd2"};


   // === Soudan R132+ endcaps - single set of channels per detector ===

   static const string kEndcapChannelNames[3] = {"Q", "PA", "PB"};
   static const string kEndcapPhononChan[2] = {"PA", "PB"};
   static const string kEndcapChargeChan[1] = {"Q"};

   static const int kEndcapNAllChan = 3;
   static const int kEndcapNPhononChan = 2;
   static const int kEndcapNChargeChan = 1;
   static const int kEndcapNCrossTalk  = 0;

   //the names after calibration
   static const string kEndcapPhononCal[8] = {"pa", "pb"};

                                                 

   // === zips and flips (mzips and dual type endcaps too), cdmslite===

   static const string kZIPFLIPChannelNames[6] = {"QI", "QO", "PA", "PB", "PC", "PD"};
   static const string kZIPFLIPPhononChan[4] = {"PA", "PB", "PC", "PD"};
   static const string kZIPFLIPChargeChan[2] = {"QI", "QO"};
   static const string kZIPFLIPCrossTalk[2]  = {"QIX", "QOX"};

   static const int kZIPFLIPNAllChan = 6;
   static const int kZIPFLIPNPhononChan = 4;
   static const int kZIPFLIPNChargeChan = 2;
   static const int kZIPFLIPNCrossTalk  = 2;

   //the names after calibration
   static const string kZIPFLIPPhononCal[4] = {"pa", "pb", "pc", "pd"};



}

#endif /* BATCALIBTYPES_H */
