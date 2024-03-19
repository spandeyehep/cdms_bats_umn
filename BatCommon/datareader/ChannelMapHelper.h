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
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////

#ifndef CHANNELMAPHELPER_H
#define CHANNELMAPHELPER_H

#include "stdint.h"
#include <vector>

using namespace std;

//!very small helper class for raw data reading
class ChannelMapHelper 
{
   public:

     //get a channel name based on detector type and detector number
     static string GetChannelName(int detType, int channelNum);

     //get a channel name based on detector code (which includes channel number)
     static string GetChannelName(uint32_t detCode);

     //fill the channel list based on the detector type - all channels, phonons and charge
     static void FillAllChannelList(int detType, vector<string>& channelNameList);

     //fill the channel list based on the detector type - phonon channel names only
     static void FillPhononChannelList(int detType, vector<string>& channelNameList);

     //fill the channel list based on the detector type - charge channel names only
     static void FillChargeChannelList(int detType, vector<string>& channelNameList);
  
     //returns number of cross talk channels - some detectors, like iZIPs have none so vector will be empty (!)
     static void  FillCrossTalkNameList(int detType, vector<string>& channelNameList);


     //get number of channels of a given type

     static int  GetNAllChannels(int detType);
     static int  GetNPhononChannels(int detType);
     static int  GetNChargeChannels(int detType);
     static int  GetNCrossTalkChannels(int detType);


     //calculate the base for the detector code, channel number is to be added by user

     static int CalcDetCodeBase(int detType, int detNum);
     static int GetDetNumFromCode(uint32_t detCode);
     static int GetDetTypeFromCode(uint32_t detCode);
     static int GetChannelNumFromCode(uint32_t detCode);

     //other utilities - channel-specific manipulations

     static string GetChannelNameBase(string& chanName) { return chanName.substr(0,1); }  //returns first char, should be "P" or "Q"
     static string GetChannelNameBaseByType(string& chanType);   //returns "P" or "Q" for "phonon" or "charge"
   
     static string GetChannelType(const string& chanName);    //returns "phonon" or "charge"
     static int GetChannelIndexBySide(int detType,string& chanName); // return index 
     static int GetChannelIndexByType(int detType,string& chanName); // return index within charge or phonon grouping 
     static int GetChannelOverallIndexByType(int detType,string& chanName); // return index within all channels grouping


     static int GetQTIndex(int detType); //total charge trace is the first after all physical channels
     static int GetPTIndex(int detType); //total phonon trace follows the QT index
     static int GetPS1Index(int detType); //sum S1 phonon trace follows the QT,PT index (iZIP only)
     static int GetPS2Index(int detType); //sum S2 phonon trace follows the QT,PT,S1 index (iZIP only)

     static bool IsPhysicalChannel(string& chanName); //does the channel correspond to a real channel on the physical detetor?


   private:
                 
};

#endif /* CHANNELMAPHELPER_H */
