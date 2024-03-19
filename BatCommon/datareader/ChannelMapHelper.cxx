//////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: ChannelMapHelper
//Authors: L. Hsu
//Description: A class to map channel id to channel names and detector codes
//
//File Import By: L. Hsu
//Creation Date: Aug. 18, 2010
//
//Modifications:
//
// 10/2017 - NM: Added relevant code for detector type kExternalTriggerUMN
// 1/2018 - NM: Added relevant code for detector type kUMN5Q
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#include <iostream>

#include "BatRootTypes.h"
#include "ChannelMapHelper.h"
#include <sstream>

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////

string ChannelMapHelper::GetChannelName(int detType, int channelNum) 
{
  string tempString = "";

  // === Choose name based on channel number and detector type (BLIP, FLIP, ZIP, iZIP ...) ===
  //     Note that mapping from channel number to name is specified by raw data format and
  //     should be exactly matched in the BatRootTypes file in order for this to work!

  int maxChan = 0;


  //BLIPS
  if(detType == BatRootTypes::kBLIPDetType)
  {
     maxChan = BatRootTypes::kBLIPNAllChan;
     
     if(channelNum < maxChan)
	tempString = BatRootTypes::kBLIPChannelNames[channelNum];
  }


  //FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type
  if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
     detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
  {
     maxChan = BatRootTypes::kZipFlipNAllChan;

     if(channelNum < maxChan)
	tempString = BatRootTypes::kZipFlipChannelNames[channelNum];

  }


  //endcaps - single variety
  if(detType == BatRootTypes::kEndcapDetType)
  {
     maxChan = BatRootTypes::kEndcapNAllChan;

     if(channelNum < maxChan)
	tempString = BatRootTypes::kEndcapChannelNames[channelNum];

  }

  //Soudan tri-fold symmetry iZIP
  if(detType == BatRootTypes::kiZIPSoudan)
  {
     maxChan = BatRootTypes::kiZIPSoudanNAllChan;

     if(channelNum < maxChan)
	tempString = BatRootTypes::kiZIPSoudanChannelNames[channelNum];
  }


  //Soudan CDMSlite type I
  if(detType == BatRootTypes::kCDMSliteSoudanI)
  {
     maxChan = BatRootTypes::kCDMSliteSoudanINAllChan;

     if(channelNum < maxChan)
	tempString = BatRootTypes::kCDMSliteSoudanIChannelNames[channelNum];
  }

   
  // Noise monitor

  if(detType == BatRootTypes::kMonitorNoiseFast)
  {
    stringstream ss;
    ss<<"NFast"<<channelNum+1;
    tempString = ss.str();
  }



  if(detType == BatRootTypes::kMonitorNoiseSlow)
  {
    stringstream ss;
    ss<<"NSlow"<<channelNum+1;
    tempString = ss.str();
  }




  //QT and PT are defined based on total number of channels
  // This part in only for ZIP data 
 
  if (detType != BatRootTypes::kMonitorNoiseFast &&
      detType != BatRootTypes::kMonitorNoiseSlow) {

      if(channelNum == GetQTIndex(detType))
         tempString = "QT";

      if(channelNum == GetPTIndex(detType))
         tempString = "PT";
  
      if(channelNum == GetPS1Index(detType))
         tempString = "PS1";

      if(channelNum == GetPS2Index(detType))
         tempString = "PS2";
  
  }


  // 100 mm devices
  
  if(detType == BatRootTypes::kiZIPSNOlab)
  {
	  maxChan = BatRootTypes::kiZIPSNOlabNAllChan;
	  
	  if(channelNum < maxChan)
		  tempString = BatRootTypes::kiZIPSNOlabChannelNames[channelNum];
  }

  if(detType == BatRootTypes::kHVSNOlab)
  {
	  maxChan = BatRootTypes::kHVSNOlabNAllChan;
	  
	  if(channelNum < maxChan)
		  tempString = BatRootTypes::kHVSNOlabChannelNames[channelNum];
  }

  if(detType == BatRootTypes::kHVUMN)
  {
	  maxChan = BatRootTypes::kHVUMNNAllChan;
	  
	  if(channelNum < maxChan)
		  tempString = BatRootTypes::kHVUMNChannelNames[channelNum];
  }


  //External Trigger
  if(detType == BatRootTypes::kExternalTriggerUMN)
  {
  	maxChan = BatRootTypes::kExternalTriggerUMNNAllChan;
	if(channelNum < maxChan)
		tempString = BatRootTypes::kExternalTriggerUMNChannelNames[channelNum];
  }

  //5Q
  if(detType == BatRootTypes::kUMN5Q)
  {
  	maxChan = BatRootTypes::kUMN5QNAllChan;
	if(channelNum < maxChan)
		tempString = BatRootTypes::kUMN5QChannelNames[channelNum];
  }


  //Now check that something was assigned
  if(tempString == "")
  {
    cout <<"ERROR!  ChannelMapHelper::GetChannelName - unknown combination of detectorType "<<detType
	 <<" and channelNum "<<channelNum<<" passed to this function."
	  <<"\nChannelMapHelper failed to assign a name to this channel!" 
	  << endl;

     exit(1);
  }
  
  //cout <<"channelNum = " << channelNum <<", and maxChan = " << maxChan <<", and detectorType = " << detType << endl;

  return tempString;
}


string ChannelMapHelper::GetChannelName(uint32_t detCode)
{
  int detType = GetDetTypeFromCode(detCode);
  int chanNum = GetChannelNumFromCode(detCode);

  return GetChannelName(detType, chanNum);
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ChannelMapHelper::FillAllChannelList(int detType, std::vector<string>& channelNameList)
{

   //BLIPS
   if(detType == BatRootTypes::kBLIPDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kBLIPNAllChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kBLIPChannelNames[nameCtr]);
      }
   }
   
   
   //FLIPs, CDMSII ZIPs, mercedes ZIPs, ganged endcaps
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNAllChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kZipFlipChannelNames[nameCtr]);
      }
   }
   
   
   //endcaps - separate for top and bottom
   if(detType == BatRootTypes::kEndcapDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kEndcapNAllChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kEndcapChannelNames[nameCtr]);
      }      
   }
   
   //Soudan tri-fold symmetry iZIP
   if(detType == BatRootTypes::kiZIPSoudan)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNAllChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kiZIPSoudanChannelNames[nameCtr]);
      }      
   }

   //Soudan CDMSlite type I
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINAllChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kCDMSliteSoudanIChannelNames[nameCtr]);
      }      
   }



   // SNOLAB iZIP 100 mm
   if(detType == BatRootTypes::kiZIPSNOlab)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNAllChan; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kiZIPSNOlabChannelNames[nameCtr]);   
   }
   
   if(detType == BatRootTypes::kHVSNOlab)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kHVSNOlabNAllChan; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kHVSNOlabChannelNames[nameCtr]);   
   }
   
   if(detType == BatRootTypes::kHVUMN)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kHVUMNNAllChan; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kHVUMNChannelNames[nameCtr]);   
   }
   
   //External Trigger
   if(detType == BatRootTypes::kExternalTriggerUMN)
   {
   	for(int nameCtr = 0; nameCtr < BatRootTypes::kExternalTriggerUMNNAllChan; nameCtr++)
		channelNameList.push_back(BatRootTypes::kExternalTriggerUMNChannelNames[nameCtr]);
   }

   //UMN5Q
   if(detType == BatRootTypes::kUMN5Q)
   {
   	for(int nameCtr = 0; nameCtr < BatRootTypes::kUMN5QNAllChan; nameCtr++)
		channelNameList.push_back(BatRootTypes::kUMN5QChannelNames[nameCtr]);
   }

   return;
}


void ChannelMapHelper::FillPhononChannelList(int detType, std::vector<string>& channelNameList)
{

   //BLIPS
   if(detType == BatRootTypes::kBLIPDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kBLIPNPhononChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kBLIPPhononChan[nameCtr]);
      }
   }
   
   
   //FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNPhononChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kZipFlipPhononChan[nameCtr]);
      }
   }
   
   
   //endcaps
   if(detType == BatRootTypes::kEndcapDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kEndcapNPhononChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kEndcapPhononChan[nameCtr]);
      }      
   }
   
   
   //Soudan tri-fold symmetry iZIP
   if(detType == BatRootTypes::kiZIPSoudan)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNPhononChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kiZIPSoudanPhononChan[nameCtr]);
      }      
   }

   //Soudan CDMSlite type I
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINPhononChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kCDMSliteSoudanIPhononChan[nameCtr]);
      }      
   }

   
   // SNOLAB iZIP 100 mm
   if(detType == BatRootTypes::kiZIPSNOlab)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNPhononChan; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kiZIPSNOlabPhononChan[nameCtr]);   
   }
   
   if(detType == BatRootTypes::kHVSNOlab)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kHVSNOlabNPhononChan; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kHVSNOlabPhononChan[nameCtr]);   
   }
   
   if(detType == BatRootTypes::kHVUMN)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kHVUMNNPhononChan; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kHVUMNPhononChan[nameCtr]);   
   }
   

   //External Trigger
   if(detType == BatRootTypes::kExternalTriggerUMN)
   {
   	for(int nameCtr = 0; nameCtr < BatRootTypes::kExternalTriggerUMNNPhononChan; nameCtr++)
		channelNameList.push_back(BatRootTypes::kExternalTriggerUMNPhononChan[nameCtr]);
   }

   
   return;
}


void ChannelMapHelper::FillChargeChannelList(int detType, std::vector<string>& channelNameList)
{
   //BLIPS
   if(detType == BatRootTypes::kBLIPDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kBLIPNChargeChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kBLIPChargeChan[nameCtr]);
      }
   }
   
   
   //FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type 
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNChargeChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kZipFlipChargeChan[nameCtr]);
      }
   }
   
   
   //endcaps
   if(detType == BatRootTypes::kEndcapDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kEndcapNChargeChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kEndcapChargeChan[nameCtr]);
      }      
   }
   
   
   //Soudan tri-fold symmetry iZIP
   if(detType == BatRootTypes::kiZIPSoudan)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNChargeChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kiZIPSoudanChargeChan[nameCtr]);
      }      
   }

   //Soudan CDMSlite type I
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINChargeChan; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kCDMSliteSoudanIChargeChan[nameCtr]);
      }      
   }

   // SNOLAB iZIP 100 mm
   if(detType == BatRootTypes::kiZIPSNOlab)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNChargeChan; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kiZIPSNOlabChargeChan[nameCtr]);   
   }

   //UMN5Q
   if(detType == BatRootTypes::kUMN5Q)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kUMN5QNChargeChan; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kUMN5QChargeChan[nameCtr]);   
   }
   
      
   return;
}


void ChannelMapHelper::FillCrossTalkNameList(int detType, std::vector<string>& channelNameList)
{
   //BLIPS
   if(detType == BatRootTypes::kBLIPDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kBLIPNCrossTalk; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kBLIPCrossTalk[nameCtr]);
      }
   }
   
   
   //FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap det type
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNCrossTalk; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kZipFlipCrossTalk[nameCtr]);
      }
   }
   
   
   //endcaps - no cross talk
   if(detType == BatRootTypes::kEndcapDetType)
   {
      return;
   }
   
   
   //Soudan bi-fold, trifold symmetry iZIP
   if(detType == BatRootTypes::kiZIPSoudan)
   {

     for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNCrossTalk; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kiZIPSoudanCrossTalk[nameCtr]);
      }

      return;
   }

   //CDMSlite SoudanI
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {

     for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINCrossTalk; nameCtr++)
      {
	 channelNameList.push_back(BatRootTypes::kCDMSliteSoudanICrossTalk[nameCtr]);
      }

      return;
   }

   
   // SNOLAB iZIP 100 mm
   if(detType == BatRootTypes::kiZIPSNOlab)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNCrossTalk; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kiZIPSNOlabCrossTalk[nameCtr]);   
   }

   // UMN5Q
   if(detType == BatRootTypes::kUMN5Q)
   { 
	   for(int nameCtr = 0; nameCtr < BatRootTypes::kUMN5QNCrossTalk; nameCtr++)
		   channelNameList.push_back(BatRootTypes::kUMN5QCrossTalk[nameCtr]);   
   }

   
      
   return;
}



//////////////////////////////////////////////////////////////////////////////////////////////

int ChannelMapHelper::GetNAllChannels(int detType)
{

   //BLIPS
   if(detType == BatRootTypes::kBLIPDetType)
   {
      return BatRootTypes::kBLIPNAllChan;
   }
   
   
   //FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap det type
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      return BatRootTypes::kZipFlipNAllChan;
   }
   
   
   //endcaps
   if(detType == BatRootTypes::kEndcapDetType)
   {
      return BatRootTypes::kEndcapNAllChan;
   }
   
   
   //Soudan tri-fold symmetry iZIP
   if(detType == BatRootTypes::kiZIPSoudan)
   {
      return BatRootTypes::kiZIPSoudanNAllChan;
   }

   //Soudan CDMSlite type I
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
      return BatRootTypes::kCDMSliteSoudanINAllChan;
   }


   // SNOLAB iZIP 
   if(detType == BatRootTypes::kiZIPSNOlab)
   {	
	   return 	BatRootTypes::kiZIPSNOlabNAllChan;
   }

   // SNOLAB HV 
   if(detType == BatRootTypes::kHVSNOlab)
   {	
	   return 	BatRootTypes::kHVSNOlabNAllChan;
   }

   // UMN HV 
   if(detType == BatRootTypes::kHVUMN)
   {	
	   return 	BatRootTypes::kHVUMNNAllChan;
   }

   //External Trigger
   if(detType == BatRootTypes::kExternalTriggerUMN)
   	return BatRootTypes::kExternalTriggerUMNNAllChan;

   //UMN5Q
   if(detType == BatRootTypes::kUMN5Q)
   	return BatRootTypes::kUMN5QNAllChan;


   //if you make it to here then ChannelMapHelper failed to find a valid detector type
   cout <<"ERROR!  ChannelMapHelper::GetNAllChannels  Detector type, " << detType
	<<", passed to this function is not known to ChannelMapHelper!"
	<< endl;
   exit(1);

   return 0;
}


int ChannelMapHelper::GetNPhononChannels(int detType)
{

   //BLIPS
   if(detType == BatRootTypes::kBLIPDetType)
   {
      return BatRootTypes::kBLIPNPhononChan;
   }
   
   
   //FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      return BatRootTypes::kZipFlipNPhononChan;
   }
   
   
   //endcaps
   if(detType == BatRootTypes::kEndcapDetType)
   {
      return BatRootTypes::kEndcapNPhononChan;
   }
   
   
   //Soudan tri-fold symmetry iZIP
   if(detType == BatRootTypes::kiZIPSoudan)
   {
      return BatRootTypes::kiZIPSoudanNPhononChan;
   }


   //Soudan CDMSlite type I
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
      return BatRootTypes::kCDMSliteSoudanINPhononChan;
   }

   // SNOLAB iZIP 
   if(detType == BatRootTypes::kiZIPSNOlab)
   {
	   return BatRootTypes::kiZIPSNOlabNPhononChan;
   }
	   
   // SNOLAB HV 
   if(detType == BatRootTypes::kHVSNOlab)
   {
	   return BatRootTypes::kHVSNOlabNPhononChan;
   }
	   
   // UMN HV 
   if(detType == BatRootTypes::kHVUMN)
   {
	   return BatRootTypes::kHVUMNNPhononChan;
   }

   //External Trigger
   if(detType == BatRootTypes::kExternalTriggerUMN)
   	return BatRootTypes::kExternalTriggerUMNNPhononChan;

   //UMN5Q
   if(detType == BatRootTypes::kUMN5Q)
   	return BatRootTypes::kUMN5QNPhononChan;


   //if you make it to here then ChannelMapHelper failed to find a valid detector type
   cout <<"ERROR!  ChannelMapHelper::GetNPhononChannels  Detector type passed to this function is not known to ChannelMapHelper!"
	<< endl;
   exit(1);

   return 0;
}

int ChannelMapHelper::GetNChargeChannels(int detType)
{

   //BLIPS
   if(detType == BatRootTypes::kBLIPDetType)
   {
      return BatRootTypes::kBLIPNChargeChan;
   }
   
   
   //FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      return BatRootTypes::kZipFlipNChargeChan;
   }
   
   
   //endcaps
   if(detType == BatRootTypes::kEndcapDetType)
   {
      return BatRootTypes::kEndcapNChargeChan;
   }
   
   
   //Soudan tri-fold symmetry iZIP
   if(detType == BatRootTypes::kiZIPSoudan)
   {
      return BatRootTypes::kiZIPSoudanNChargeChan;
   }


   //Soudan CDMSlite type I
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
      return BatRootTypes::kCDMSliteSoudanINChargeChan;
   }

   
   // SNOLAB iZIP 
   if(detType == BatRootTypes::kiZIPSNOlab)
   {
	   return BatRootTypes::kiZIPSNOlabNChargeChan;
   }
	 

   // SNOLAB HV 
   if(detType == BatRootTypes::kHVSNOlab)
   {
	   return BatRootTypes::kHVSNOlabNChargeChan;
   }
	 

   // UMN HV 
   if(detType == BatRootTypes::kHVUMN)
   {
	   return BatRootTypes::kHVUMNNChargeChan;
   }
	 
   //External Trigger
   if(detType == BatRootTypes::kExternalTriggerUMN)
   	return BatRootTypes::kExternalTriggerUMNNChargeChan;

   //UMN5Q
   if(detType == BatRootTypes::kUMN5Q)
   	return BatRootTypes::kUMN5QNChargeChan;



   //if you make it to here then ChannelMapHelper failed to find a valid detector type
   cout <<"ERROR!  ChannelMapHelper::GetNChargeChannels  Detector type passed to this function is not known to ChannelMapHelper!"
	<< endl;
   exit(1);

   return 0;
}

int ChannelMapHelper::GetNCrossTalkChannels(int detType)
{

   //BLIPS
   if(detType == BatRootTypes::kBLIPDetType)
   {
      return BatRootTypes::kBLIPNCrossTalk;
   }
   
   
   //FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      return BatRootTypes::kZipFlipNCrossTalk;
   }
   
   
   //endcaps
   if(detType == BatRootTypes::kEndcapDetType)
   {
      return BatRootTypes::kEndcapNCrossTalk;
   }
   
   
   //Soudan tri-fold symmetry iZIP
   if(detType == BatRootTypes::kiZIPSoudan)
   {
      return BatRootTypes::kiZIPSoudanNCrossTalk;
   }


   //CDMSlite SoudanI
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
      return BatRootTypes::kCDMSliteSoudanINCrossTalk;
   }

   
   // SNOLAB iZIP 
   if(detType == BatRootTypes::kiZIPSNOlab)
   {
	   return BatRootTypes::kiZIPSNOlabNCrossTalk;
   }

   
   // SNOLAB HV 
   if(detType == BatRootTypes::kHVSNOlab)
   {
	   return BatRootTypes::kHVSNOlabNCrossTalk;
   }

   
   // UMN HV
   if(detType == BatRootTypes::kHVUMN)
   {
	   return BatRootTypes::kHVUMNNCrossTalk;
   }
	 
   //External Trigger
   if(detType == BatRootTypes::kExternalTriggerUMN)
   	return BatRootTypes::kExternalTriggerUMNNCrossTalk;

   //UMN5Q
   if(detType == BatRootTypes::kUMN5Q)
   	return BatRootTypes::kUMN5QNCrossTalk;


   //if you make it to here then ChannelMapHelper failed to find a valid detector type
   cout <<"ERROR!  ChannelMapHelper::GetNCrossTalkChannels  Detector type passed to this function is not known to ChannelMapHelper!"
	<< endl;
   exit(1);

   return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////////


int ChannelMapHelper::CalcDetCodeBase(int detType, int detNum)
{
   int detCodeBase = detType*1000000 + detNum*1000; 
  
   return detCodeBase;
}


int ChannelMapHelper::GetDetNumFromCode(uint32_t detCode)
{
   int detNum = ((detCode % 1000000) - (detCode % 1000))/1000; 

   return detNum;
}

int ChannelMapHelper::GetDetTypeFromCode(uint32_t detCode)
{
   int detType = (detCode - (detCode % 1000000))/1000000; 

   return detType;
}

int ChannelMapHelper::GetChannelNumFromCode(uint32_t detCode)
{
   
   int chanNum = detCode % 1000; 

   return chanNum;
}


//////////////////////////////////////////////////////////////////////////////////////////////

string ChannelMapHelper::GetChannelType(const string& chanName)
{

   if(chanName[0] == 'P')
   {
      return "phonon";
   }

   if(chanName[0] == 'Q')
   {
      return "charge";
   }


   //if you make it to here then ChannelMapHelper failed to find a valid base for the channel name
   cout <<"ERROR!  ChannelMapHelper::GetChannelType  Channel name: " << chanName <<", passed to this function is unknown!"
	<< endl;
   exit(1);


   return "";
}

//////////////////////////////////////////////////////////////////////////////////////////////

string ChannelMapHelper::GetChannelNameBaseByType(string& chanType)
{

   if(chanType=="phonon") return "P";
   if(chanType=="charge") return "Q";
   


   //if you make it to here then ChannelMapHelper failed to find a valid base for the channel name
   cout <<"ERROR!  ChannelMapHelper::GetChannelNameBaseByType  Channel type: " << chanType <<", passed to this function is unknown!"
	<< endl;
   exit(1);


   return "";
}






//////////////////////////////////////////////////////////////////////////////////////////////

int ChannelMapHelper::GetChannelIndexBySide(int detType,string& chanName)
{ 
     
   // FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      if (chanName[0]== 'P') {
         
	  int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNPhononChan; nameCtr++) {
	    if (BatRootTypes::kZipFlipPhononChan[nameCtr]==chanName)
	 	return index;
            index++;
           }
       } else {
          
          int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNChargeChan; nameCtr++) {
	    if (BatRootTypes::kZipFlipChargeChan[nameCtr]==chanName)
	 	return index;
            index++;
           }  

       }
   }

  
   //endcaps
   if(detType == BatRootTypes::kEndcapDetType)
   {

     if (chanName[0]== 'P') {
         
	  int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kEndcapNPhononChan; nameCtr++) {
	    if (BatRootTypes::kEndcapPhononChan[nameCtr]==chanName)
	 	return index;
            index++;
           }
       } else {
          
          int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kEndcapNChargeChan; nameCtr++) {
	    if (BatRootTypes::kEndcapChargeChan[nameCtr]==chanName)
	 	return index;
            index++;
          }  
       }
   }
   
   
   //Soudan iZIP (same for bi or tri-fold iZIP)
   if(detType == BatRootTypes::kiZIPSoudan)
   {
     if (chanName[0]== 'P') {
         
	  int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNPhononChan; nameCtr++) {
	    if (BatRootTypes::kiZIPSoudanPhononChanBySide[nameCtr]==chanName)
	 	return index;
            index++;
           }
       } else {
          
          int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNChargeChan; nameCtr++) {
	    if (BatRootTypes::kiZIPSoudanChargeChanBySide[nameCtr]==chanName)
	 	return index;
            index++;
          }  
       }
   }

   // CDMSlite SoudanI
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
      if (chanName[0]== 'P') {
         
	  int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINPhononChan; nameCtr++) {
	    if (BatRootTypes::kCDMSliteSoudanIPhononChan[nameCtr]==chanName)
	 	return index;
            index++;
           }
       } else {
          
          int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINChargeChan; nameCtr++) {
	    if (BatRootTypes::kCDMSliteSoudanIChargeChan[nameCtr]==chanName)
	 	return index;
            index++;
           }  

       }
   }

 
   // SNOLAB iZIP 
   if(detType == BatRootTypes::kiZIPSNOlab)
   {

	   if (chanName[0]== 'P') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNPhononChan; nameCtr++) {
			   if (BatRootTypes::kiZIPSNOlabPhononChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {
		   
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNChargeChan; nameCtr++) {
			   if (BatRootTypes::kiZIPSNOlabChargeChan[nameCtr]==chanName)
				   return index;
			   index++;
           }  
		   
       }
	   
   }

 
   // SNOLAB HV 
   if(detType == BatRootTypes::kHVSNOlab)
   {

	   if (chanName[0]== 'P') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kHVSNOlabNPhononChan; nameCtr++) {
			   if (BatRootTypes::kHVSNOlabPhononChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {

		   // HV detectors have only phonon channels, so if you make it here then there has been an error.
		   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexBySide:  Channel name type passed to this function is not valid for this detector type!"
		   << endl;
		   exit(1);
		   return 0;
		   
       }
	   
   }

 
   // UMN HV 
   if(detType == BatRootTypes::kHVUMN)
   {

	   if (chanName[0]== 'P') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kHVUMNNPhononChan; nameCtr++) {
			   if (BatRootTypes::kHVUMNPhononChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {

		   // HV detectors have only phonon channels, so if you make it here then there has been an error.
		   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexBySide:  Channel name type passed to this function is not valid for this detector type!"
		   << endl;
		   exit(1);
		   return 0;
		   
       }
	   
   }

   //External Trigger 
   if(detType == BatRootTypes::kExternalTriggerUMN)
   {

	   if (chanName[0]== 'P') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kExternalTriggerUMNNPhononChan; nameCtr++) {
			   if (BatRootTypes::kExternalTriggerUMNPhononChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {

		   // External Trigger is only connected to phonon channels, so if you make it here then there has been an error.
		   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexBySide:  Channel name type passed to this function is not valid for this detector type!"
		   << endl;
		   exit(1);
		   return 0;
		   
       }
	   
   }

   //UMN5Q 
   if(detType == BatRootTypes::kUMN5Q)
   {

	   if (chanName[0]== 'Q') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kUMN5QNChargeChan; nameCtr++) {
			   if (BatRootTypes::kUMN5QChargeChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {

		   // UMN 5Q is only connected to charge channels, so if you make it here then there has been an error.
		   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexBySide:  Channel name type passed to this function is not valid for this detector type!"<< endl;
		   cout <<"Channel name: " << chanName << ", detector type: kUMN5Q"<<endl;
		   exit(1);
		   return 0;
		   
       }
	   
   }



   //if you make it to here then ChannelMapHelper failed to find a valid index 
   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexBySide:  Channel name passed to this function is not known to ChannelMapHelper!"
	<< endl;
   exit(1);

   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

int ChannelMapHelper::GetChannelOverallIndexByType(int detType,string& chanName)
{ 
     
  // FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type
  if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
     detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
    {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNAllChan; nameCtr++) {
	if (BatRootTypes::kZipFlipChannelNames[nameCtr]==chanName)
	  return index;
	index++;
      }  
    }
  
  
  //endcaps
  if(detType == BatRootTypes::kEndcapDetType)
    {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kEndcapNAllChan; nameCtr++) {
	if (BatRootTypes::kEndcapChannelNames[nameCtr]==chanName)
	  return index;
	index++;
      }  
    }
   
  
  //Soudan iZIP (same for bi or tri-fold iZIP)
  if(detType == BatRootTypes::kiZIPSoudan)
    {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNAllChan; nameCtr++) {
	if (BatRootTypes::kiZIPSoudanChannelNames[nameCtr]==chanName)
	  return index;
	index++;
      }  
    }
  
  //Soudan CDMSlite type I
  if(detType == BatRootTypes::kCDMSliteSoudanI)
    {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINAllChan; nameCtr++) {
	if (BatRootTypes::kCDMSliteSoudanIChannelNames[nameCtr]==chanName)
	  return index;
       index++;
      }  
    }
  
   
  // SNOLAB iZIP 
  if(detType == BatRootTypes::kiZIPSNOlab)
    {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNAllChan; nameCtr++) {
	if (BatRootTypes::kiZIPSNOlabChannelNames[nameCtr]==chanName)
	  return index;
       index++;
      }  
      
    }
  
   
  // SNOLAB HV 
  if(detType == BatRootTypes::kHVSNOlab)
    {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kHVSNOlabNAllChan; nameCtr++) {
	if (BatRootTypes::kHVSNOlabChannelNames[nameCtr]==chanName)
	  return index;
       index++;
      }  
      
    }
  
   
  // UMN HV 
  if(detType == BatRootTypes::kHVUMN)
    {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kHVUMNNAllChan; nameCtr++) {
	if (BatRootTypes::kHVUMNChannelNames[nameCtr]==chanName)
	  return index;
       index++;
      }   
    }
   
  // External Trigger 
  if(detType == BatRootTypes::kExternalTriggerUMN)
  {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kExternalTriggerUMNNAllChan; nameCtr++) {
	if (BatRootTypes::kExternalTriggerUMNChannelNames[nameCtr]==chanName)
	  return index;
       index++;
      }  
  }

  // UMN5Q
  if(detType == BatRootTypes::kUMN5Q)
  {
      int index = 0;
      for(int nameCtr = 0; nameCtr < BatRootTypes::kUMN5QNAllChan; nameCtr++) {
	if (BatRootTypes::kUMN5QChannelNames[nameCtr]==chanName)
	  return index;
       index++;
      }  
  }
  
  
   //if you make it to here then ChannelMapHelper failed to find a valid index 
   cout <<"ERROR!  ChannelMapHelper::GetAllChannelsIndexByType:  Channel name passed to this function is not known to ChannelMapHelper!"
	<< endl;
   exit(1);

   return 0;
}





//////////////////////////////////////////////////////////////////////////////////////////////

int ChannelMapHelper::GetChannelIndexByType(int detType,string& chanName)
{ 

   // FLIPs, CDMSII ZIPs, mercedes ZIPs, dual endcap type
   if(detType == BatRootTypes::kFLIPDetType || detType == BatRootTypes::kZIPDetType || 
      detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kDualEndcapDetType)
   {
      if (chanName[0]== 'P') {
         
	  int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNPhononChan; nameCtr++) {
	    if (BatRootTypes::kZipFlipPhononChan[nameCtr]==chanName)
	 	return index;
            index++;
           }
       } else {
          
          int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kZipFlipNChargeChan; nameCtr++) {
	    if (BatRootTypes::kZipFlipChargeChan[nameCtr]==chanName)
	 	return index;
            index++;
           }  

       }
   }

  
   //endcaps
   if(detType == BatRootTypes::kEndcapDetType)
   {

     if (chanName[0]== 'P') {
         
	  int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kEndcapNPhononChan; nameCtr++) {
	    if (BatRootTypes::kEndcapPhononChan[nameCtr]==chanName)
	 	return index;
            index++;
           }
       } else {
          
          int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kEndcapNChargeChan; nameCtr++) {
	    if (BatRootTypes::kEndcapChargeChan[nameCtr]==chanName)
	 	return index;
            index++;
          }  
       }
   }
   
   
   //Soudan iZIP (same for bi or tri-fold iZIP)
   if(detType == BatRootTypes::kiZIPSoudan)
   {
     if (chanName[0]== 'P') {
         
	  int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNPhononChan; nameCtr++) {
	    if (BatRootTypes::kiZIPSoudanPhononChan[nameCtr]==chanName)
	 	return index;
            index++;
           }
       } else {
          
          int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSoudanNChargeChan; nameCtr++) {
	    if (BatRootTypes::kiZIPSoudanChargeChan[nameCtr]==chanName)
	 	return index;
            index++;
          }  
       }
   }

   //Soudan CDMSlite type I
   if(detType == BatRootTypes::kCDMSliteSoudanI)
   {
     if (chanName[0]== 'P') {
         
	  int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINPhononChan; nameCtr++) {
	    if (BatRootTypes::kCDMSliteSoudanIPhononChan[nameCtr]==chanName)
	 	return index;
            index++;
           }
       } else {
          
          int index = 0;
          for(int nameCtr = 0; nameCtr < BatRootTypes::kCDMSliteSoudanINChargeChan; nameCtr++) {
	    if (BatRootTypes::kCDMSliteSoudanIChargeChan[nameCtr]==chanName)
	 	return index;
            index++;
          }  
       }
   }

   
   // SNOLAB iZIP 
   if(detType == BatRootTypes::kiZIPSNOlab)
   {

	   if (chanName[0]== 'P') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNPhononChan; nameCtr++) {
			   if (BatRootTypes::kiZIPSNOlabPhononChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {
		   
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kiZIPSNOlabNChargeChan; nameCtr++) {
			   if (BatRootTypes::kiZIPSNOlabChargeChan[nameCtr]==chanName)
				   return index;
			   index++;
           }  
		   
       }
	   
   }

   
   // SNOLAB HV 
   if(detType == BatRootTypes::kHVSNOlab)
   {

	   if (chanName[0]== 'P') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kHVSNOlabNPhononChan; nameCtr++) {
			   if (BatRootTypes::kHVSNOlabPhononChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {
		   
		   // HV detectors have only phonon channels, so if you make it here then there has been an error.
		   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexByType:  Channel name type passed to this function is not valid for this detector type!"
		   << endl;
		   exit(1);
		   return 0;
		   
       }
	   
   }

   
   // UMN HV 
   if(detType == BatRootTypes::kHVUMN)
   {

	   if (chanName[0]== 'P') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kHVUMNNPhononChan; nameCtr++) {
			   if (BatRootTypes::kHVUMNPhononChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {
		   
		   // HV detectors have only phonon channels, so if you make it here then there has been an error.
		   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexByType:  Channel name type passed to this function is not valid for this detector type!"
		   << endl;
		   exit(1);
		   return 0;
		   
       }
	   
   }

   // External Trigger 
   if(detType == BatRootTypes::kExternalTriggerUMN)
   {

	   if (chanName[0]== 'P') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kExternalTriggerUMNNPhononChan; nameCtr++) {
			   if (BatRootTypes::kExternalTriggerUMNPhononChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {
		   
		   // External triggers have only phonon channels, so if you make it here then there has been an error.
		   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexByType:  Channel name type passed to this function is not valid for this detector type!"
		   << endl;
		   exit(1);
		   return 0;
		   
       }
	   
   }

   // UMN5Q
   if(detType == BatRootTypes::kUMN5Q)
   {

	   if (chanName[0]== 'Q') {
         
		   int index = 0;
		   for(int nameCtr = 0; nameCtr < BatRootTypes::kUMN5QNChargeChan; nameCtr++) {
			   if (BatRootTypes::kUMN5QChargeChan[nameCtr]==chanName)
				   return index;
			   index++;
           }
		   
       } else {
		   
		   // UMN5Q detectors have only phonon channels, so if you make it here then there has been an error.
		   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexByType:  Channel name type passed to this function is not valid for this detector type!"<< endl;
		   cout <<"Channel name: " << chanName << ", detector type: kUMN5Q"<<endl;
		   exit(1);
		   return 0;
		   
       }
	   
   }



   //if you make it to here then ChannelMapHelper failed to find a valid index 
   cout <<"ERROR!  ChannelMapHelper::GetChannelIndexByType:  Channel name passed to this function is not known to ChannelMapHelper!"
	<< endl;
   exit(1);

   return 0;
}




//////////////////////////////////////////////////////////////////////////////////////////////

int ChannelMapHelper::GetQTIndex(int detType)
{
   //this is the first free index after physical channels
   return GetNAllChannels(detType);
}

int ChannelMapHelper::GetPTIndex(int detType)
{
   //this is the second free index after physical channels
   return (GetNAllChannels(detType)+1);
}


int ChannelMapHelper::GetPS1Index(int detType)
{
   //this is the second free index after physical channels
   return (GetNAllChannels(detType)+2);
}

int ChannelMapHelper::GetPS2Index(int detType)
{
   //this is the second free index after physical channels
   return (GetNAllChannels(detType)+3);
}




//This function expects that there are two types on "nonphysical"
//channels, PT/QT/PS1/PS2 and cross talk.   If we want to make this more general
//then one must search the list of physical channel names and compare
//to the chanName arg.   This will be slower.
bool ChannelMapHelper::IsPhysicalChannel(string& chanName)
{
    bool IsPhysical = true;


   // Get the last character to check T (indicating a summed pulse) or X (indicating a cross-talk pulse)
   char lastChar = chanName[chanName.size()-1];
   if(lastChar == 'T' || lastChar == 'X') IsPhysical =  false;
   
  
   // subtract  first character to check sum of pulses in each side
   string  lastStr = chanName.substr(1);
   if(lastStr.compare("S1")==0 || lastStr.compare("S2")==0) IsPhysical =  false;


   //  check if "slow" or "fast" noise data
   if(chanName.find("slow")!=string::npos || chanName.find("fast")!=string::npos) IsPhysical =  false;
   


   return IsPhysical;
}
