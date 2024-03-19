//////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Namespace: BatRootTypes
//Authors: L. Hsu
//Description: A Namespace for BatRoot constants and typedefs.     
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//Jan. 2018 - NM: adding UMN5Q detector type
//Oct. 2017 - NM: adding UMN external trigger detector type
//Oct. 2012 - adding Soudan CDMSlite detector types and noise monitors
//
//Nov. 22, 2010 - adding new record type kDetectorConfig, which stores
//the detector channel information that was previously obtained from 
//the external text files
//
//Aug. 2010 - added iZIP detector types and a later ENDCAP detector type.
//Also added the expanded pulse record type, kPulseRecordExpandedCodeID
//
//Jul. 2009 - added kAdminRecordID64 for expanded series format 
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef BATROOTTYPES_H
#define BATROOTTYPES_H

#include "stdint.h"
#include <cstdlib>
#include <stdio.h>
#include <string>





namespace BatRootTypes
{   
  using namespace std;
  typedef unsigned int uint;
  typedef uint32_t WORD;
   //============= Default value for empty variable ===================//
   static const long kEmptyVariable                 = -999999;
   //=========== rules for reading raw data binaries ============

   //CDMS raw data Logical Record ID
   static const WORD kAdminRecordID             = 0x01;
   static const WORD kAdminRecordID64           = 0x02;  //replaces older 0x01 type with expanded series number
   static const WORD kPulseRecordID             = 0x10;
   static const WORD kPulseRecordExpandedCodeID = 0x11;  //replaces older 0x10 type with expanded detector code
   static const WORD kSUFHistoryRecordID        = 0x20;
   static const WORD kSoudanHistoryRecordID     = 0x21;
   static const WORD kVetoADC                   = 0x40;   
   static const WORD kExternalRecordID          = 0x60;
   static const WORD kTriggerRecordID           = 0x80;
   static const WORD kTLBTriggerMaskID          = 0x81;
   static const WORD kDetectorConfigID          = 0x10000; 
   static const WORD kPhononConfigID            = 0x10001;
   static const WORD kChargeConfigID            = 0x10002;

   //misc raw data constants
   static const int  kWordSize = sizeof(WORD); //in bytes (i.e. 32 bits)
   static const int  kVetoBufferOverflow = 16129;  //taken from hardcoded value in Darkpipe_v11;
   static const uint  kPhononConfigRecordSize = 11; //in words
   static const uint  kChargeConfigRecordSize = 8;  //in words

   //=========== detector types ============

   static const int kBLIPDetType        = 1; 
   static const int kFLIPDetType        = 2; 
   static const int kVetoDetType        = 3; 

   static const int kZIPDetType         = 4;   //CDMSII
   static const int kMZIPDetType        = 5;   //mercedes
   static const int kDualEndcapDetType  = 6;   //ganged top and bottom endcaps   
   static const int kEndcapDetType      = 7;   //separate endcaps

   //static const int kiZIPSoudanBiFold   = 10;  //never installed in Soudan - iZIP test run, semi-circle inner phonons 
   static const int kiZIPSoudan         = 11;  //installed in Soudan iZIP test run, mercedes inner phonons
   static const int kCDMSliteSoudanI    = 21;  //converted detector type 11 to 1-DIB CDMSlite
   static const int kCDMSliteSoudanII   = 22;  //converted detector type 11 to 2-DIB CDMSlite - not a real detector yet

   static const int kMonitorNoiseFast   = 90;  //AC phase monitor at Soudan (short pulse length, similar to phonon channels)
   static const int kMonitorNoiseSlow   = 91;  //cryocooler noise monitor (long pulse length)
   static const int kExternalTriggerUMN	= 92;  //External trigger fed into a single phonon channel. Used at UMN for triggering based on external detector.

   // 100 mm devices
   static const int kiZIPSNOlab         = 700; //100 mm 16 channels (12 phonons, 4 charges)
   static const int kHVSNOlab           = 710; //100 mm 12 channels (12 phonons)
   static const int kHVUMN              = 1710; //converted detector type 710 for UMN HV board, 6/12 channels read out
   
   //150 mm devices
   static const int kUMN5Q		= 800; //150mm 5 charge channels


	
   //=========== misc pulse types ==========


   //=========== channel name mappings ============

   //cdmslite, 1-sided

   static const string kCDMSliteSoudanIChannelNames[6] = {"QI", "QO", "PA", "PB", "PC", "PD"};
   static const string kCDMSliteSoudanIPhononChan[4] = {"PA", "PB", "PC", "PD"};
   static const string kCDMSliteSoudanIChargeChan[2] = {"QI", "QO"};
   static const string kCDMSliteSoudanICrossTalk[2]  = {"QIX", "QOX"};

   static const int kCDMSliteSoudanINAllChan = 6;
   static const int kCDMSliteSoudanINPhononChan = 4;
   static const int kCDMSliteSoudanINChargeChan = 2;
   static const int kCDMSliteSoudanINCrossTalk  = 2;

   //tri-fold symmetry iZIP
   static const string kiZIPSoudanChannelNames[12] = {"QIS1", "QOS1", "PAS1", "PBS1", "PCS1", "PDS1",
						      "QIS2", "QOS2", "PAS2", "PBS2", "PCS2", "PDS2"};
   static const string kiZIPSoudanPhononChan[8] = {"PAS1", "PBS1", "PCS1", "PDS1", "PAS2", "PBS2", "PCS2", "PDS2"};
   static const string kiZIPSoudanChargeChan[4] = {"QIS1", "QOS1", "QIS2", "QOS2"};
 
   static const int kiZIPSoudanNAllChan = 12;
   static const int kiZIPSoudanNPhononChan = 8;
   static const int kiZIPSoudanNChargeChan = 4;
   static const int kiZIPSoudanNCrossTalk  = 4;

   // iZIP (bi or tri-fold) phonon list by side - this is dictated by geometry (no dib mapping implied here)
   static const string kiZIPSoudanPhononChanBySide[8] = {"PAS1", "PBS1", "PCS1", "PDS1", "PAS2", "PBS2", "PCS2", "PDS2"};
   static const string kiZIPSoudanChargeChanBySide[4] = {"QIS1", "QOS1", "QIS2", "QOS2"};
   static const string kiZIPSoudanCrossTalk[4] = {"QIS1X", "QOS1X", "QIS2X", "QOS2X"};

   //Soudan R130/R131 endcaps (top and bottom ganged together) - these have the same mapping the same as CDMSII zips

   //Soudan R132+ endcaps - single set of channels per detector, as it should be

   static const string kEndcapChannelNames[3] = {"Q", "PA", "PB"};
   static const string kEndcapPhononChan[2] = {"PA", "PB"};
   static const string kEndcapChargeChan[1] = {"Q"};

   static const int kEndcapNAllChan = 3;
   static const int kEndcapNPhononChan = 2;
   static const int kEndcapNChargeChan = 1;
   static const int kEndcapNCrossTalk  = 0;


   //(cdmsII) zips and flips (this includes mzips too)
   static const string kZipFlipChannelNames[6] = {"QI", "QO", "PA", "PB", "PC", "PD"};
   static const string kZipFlipPhononChan[4] = {"PA", "PB", "PC", "PD"};
   static const string kZipFlipChargeChan[2] = {"QI", "QO"};
   static const string kZipFlipCrossTalk[2]  = {"QIX", "QOX"};

   static const int kZipFlipNAllChan = 6;
   static const int kZipFlipNPhononChan = 4;
   static const int kZipFlipNChargeChan = 2;
   static const int kZipFlipNCrossTalk  = 2;

   //blips - RQ writout not fully hooked up for this type yet (LLH)
   static const string kBLIPChannelNames[4] = {"QI", "QO", "PS1", "PS2"};
   static const string kBLIPPhononChan[4] = {"PS1", "PS2"};
   static const string kBLIPChargeChan[2] = {"QI", "QO"};
   static const string kBLIPCrossTalk[2]  = {"QIX", "QOX"};

   static const int kBLIPNAllChan = 4;
   static const int kBLIPNPhononChan = 2;
   static const int kBLIPNChargeChan = 2;   
   static const int kBLIPNCrossTalk  = 2; 

   // 100 mm devices
   static const string kiZIPSNOlabChannelNames[16] = {"QIS1", "QOS1", "QIS2", "QOS2",
						      "PAS1", "PBS1", "PCS1",
						      "PDS1", "PES1", "PFS1", 
						      "PAS2", "PBS2", "PCS2",
						      "PDS2","PES2", "PFS2"};
   static const string kiZIPSNOlabPhononChan[12] = {"PAS1", "PBS1", "PCS1", "PDS1", "PES1", "PFS1",
						    "PAS2", "PBS2", "PCS2", "PDS2", "PES2", "PFS2"};
   static const string kiZIPSNOlabChargeChan[4] = {"QIS1", "QOS1", "QIS2", "QOS2"};
   static const string kiZIPSNOlabCrossTalk[4] = {"QIS1X", "QOS1X", "QIS2X", "QOS2X"};
   static const int kiZIPSNOlabNAllChan = 16;
   static const int kiZIPSNOlabNPhononChan = 12;
   static const int kiZIPSNOlabNChargeChan = 4;
   static const int kiZIPSNOlabNCrossTalk  = 4;

   //HV detector in 2-sided readout mode
   // MF - change the order here to the standard A1, B1, C1... F2
   //static const string kHVSNOlabChannelNames[12] = {"PFS1", "PCS1", "PDS1",
						    //"PBS1", "PES1", "PAS1",
						    //"PFS2", "PCS2", "PBS2",
						    //"PES2","PDS2", "PAS2"};
   //static const string kHVSNOlabPhononChan[12] = {"PFS1", "PCS1", "PDS1", "PBS1", "PES1", "PAS1",
						  //"PFS2", "PCS2", "PBS2", "PES2", "PDS2", "PAS2"};
   static const string kHVSNOlabChannelNames[12] = {"PAS1", "PBS1", "PCS1", "PDS1", "PES1", "PFS1", "PAS2", "PBS2", "PCS2", "PDS2","PES2", "PFS2"};
   static const string kHVSNOlabPhononChan[12] = {"PAS1", "PBS1", "PCS1", "PDS1", "PES1", "PFS1", "PAS2", "PBS2", "PCS2", "PDS2","PES2", "PFS2"};

   static const int kHVSNOlabNAllChan = 12;
   static const int kHVSNOlabNPhononChan = 12;
   static const int kHVSNOlabNChargeChan = 0;
   static const int kHVSNOlabNCrossTalk  = 0;

   //HV detector in 1-sided readout mode 
   static const string kHVUMNChannelNames[6] = {"PA", "PB", "PC", "PD", "PE", "PF"};
   static const string kHVUMNPhononChan[12] = {"PA", "PB", "PC", "PD", "PE", "PF"};
   static const int kHVUMNNAllChan = 6;
   static const int kHVUMNNPhononChan = 6;
   static const int kHVUMNNChargeChan = 0;
   static const int kHVUMNNCrossTalk  = 0;

   //External Trigger
   static const string kExternalTriggerUMNChannelNames[1] = {"P"};
   static const string kExternalTriggerUMNPhononChan[1] = {"P"};
   static const int kExternalTriggerUMNNAllChan = 1;
   static const int kExternalTriggerUMNNPhononChan = 1;
   static const int kExternalTriggerUMNNChargeChan = 0;
   static const int kExternalTriggerUMNNCrossTalk= 0;

   //5Q device
   static const string kUMN5QChannelNames[5] = {"QA", "QB", "QC", "QD", "QE"};
   static const string kUMN5QChargeChan[5] = {"QA", "QB", "QC", "QD", "QE"};
   static const string kUMN5QCrossTalk[5] = {"QAX", "QBX", "QCX", "QDX", "QEX"};
   static const int kUMN5QNAllChan = 5;
   static const int kUMN5QNPhononChan = 0;
   static const int kUMN5QNChargeChan = 5;
   static const int kUMN5QNCrossTalk = 5;

   //=========== detector mapping ============

   static const int kSoudanNZipsPerTower = 6; //FIXME - this is defunct! (LLH)
}

#endif /* BATROOTTYPES_H */
