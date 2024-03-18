///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: PulseEvtBuilder
//Authors: L. Hsu, M. Kos, B. Serfass
//Description: This class is basically a clone of EventBuilder, for dedicated use with the pulse simulation
// library builder utility. The arrangement is a bit of a hack for now, but works well enough.
//
//File Import By: A. Anderson
//Import date: 19 June 2014
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "zlib.h"


//CDMSBATS math library (inherits from blas sparse matrix of complex nums)
#include "SprseMatrix.h"

#include "PulseTools.h"
#include "ChannelMapHelper.h"
#include "PulseEvtBuilder.h"

//User Analysis Classes - DO NOT modify or copy this comment (for auto_analysis)
#include "SimulateFromRandoms.h"
#include "OptimalFilterPhononNS.h"
#include "OptimalFilterNxN.h"
#include "PSDIntegralPhonon.h"
#include "WedgeFitPhonon.h"
#include "ConstFreqRTFTWalkPhonon.h"
#include "VetoAnalysis.h"
#include "F5ChargeX.h"
#include "InflectionTime.h"
#include "NoiseSelector.h"
#include "PulseIntegral.h"
#include "VarFreqRTFTWalkPhonon.h"
#include "RTFTWalkCharge.h"
#include "BasicPulseCalc.h"
#include "PipeFitPhonon.h"
#include "OptimalFilterPhonon.h"
#include "OptimalFilterChargeX.h"
#include "OptimalFilterCharge.h"
#include "SingleExponentialFit.h"
#include "GenericRQStorage.h"
#include "TRandom3.h"

using namespace std;

////////////////////////////////////////////////////////

//default constructor
PulseEvtBuilder::PulseEvtBuilder(UserDataManager& myUserData, DetectorConfigManager& myDetectorConfigManager,
			   string& inputRawDataFile) :
   fUserData(myUserData),
   fReadIsr(true),
   fReadInfo(true),
   fDetectorConfigManager(myDetectorConfigManager)
{

   // register the data classes so that they will be read 
   // (note: all of these can be made optional through config interface)
   
   fRawReader.RegisterAdminData(&fAdminData);
   fRawReader.RegisterGPSData(&fGPSData);

   //this is optional 
   if(fUserData.DoTriggerProcessing()) 
   {
      fRawReader.RegisterHistoryData(&fHistoryData, fUserData.GetIntParameter("MAX_TOWERS")); 
      fRawReader.RegisterTriggerData(&fTriggerData, fUserData.GetIntParameter("MAX_TOWERS")); 
   }   

   // pulses handled a little differently
   fRawReader.RegisterZipPulseMap(&fMapOfZipPulses); 
   fRawReader.RegisterVetoPulseVector(&fVectorOfVetoPulses); //will read out all veto in the file
   
   // store certain ExtData settings so that we don't have to continually check them - FIXME should this stay here?
   fReadIsr = fUserData.DoRead("ISR_FILE");
   fReadInfo = fUserData.DoRead("INFO_FILE");

   // --- open the file ---

   fRawReader.OpenRawDataFile(fUserData.GetPath("RAW_DATA"), inputRawDataFile);
   
   // reads the file header
   fRawReader.ReadFileHeader(false);
   
   // Allow modification of configuration if needed   
   if (fUserData.DoModifyRawData()) {

      // get detector -> modification map
      map<int, string> modificationMap = fUserData.GetRawDataModificationMap();
   
      if (!modificationMap.empty())
         fRawReader.ModifyRawData(modificationMap);
      else {
        cerr <<"PulseEvtBuilder::ERROR!  Cannot modify raw data without modification type!"  
	    <<"\nNeed to add a MODIFICATION_TYPE parameter in processing settings."
	    << endl;
        exit(1);
      }

   }
}

PulseEvtBuilder::~PulseEvtBuilder()
{
    fRawReader.CloseRawDataFile();
}

uint32_t PulseEvtBuilder::GetEventCategory()
{
  return fRawReader.GetEventCategory();
}

int PulseEvtBuilder::ReadNextEvent()
{
   //First clear data containers of previous event's data
   fRawReader.Clear();

   //Read the event!
   int checkStatus = fRawReader.ReadRawDataRecord();

   // Modify pulse data if needed
   if (checkStatus && fUserData.DoModifyRawData()) {
     
      map<int, string> modificationMap = fUserData.GetRawDataModificationMap();
  
      if (!modificationMap.empty())
         fRawReader.ModifyRawData(modificationMap);
      else {
        cerr <<"PulseEvtBuilder::ERROR!  Cannot modify raw data without modification type!"  
	    <<"\nNeed to add a MODIFICATION_TYPE parameter in processing settings."
	    << endl;
        exit(1);
      }
    }


   return checkStatus;
}


int PulseEvtBuilder::ReadEventN(int eventN)
{
   //First clear data containers of previous event's data
   fRawReader.Clear();

   //Read the event!
   int checkStatus = fRawReader.ReadRawDataRecord(eventN);

   // Modify pulse data if needed
   if (checkStatus && fUserData.DoModifyRawData()) {
     
      map<int, string> modificationMap = fUserData.GetRawDataModificationMap();
   
      if (!modificationMap.empty())
         fRawReader.ModifyRawData(modificationMap);
      else {
        cerr <<"PulseEvtBuilder::ERROR!  Cannot modify raw data without modification type!"  
      <<"\nNeed to add a MODIFICATION_TYPE parameter in processing settings."
      << endl;
        exit(1);
      }
    }


   return checkStatus;
}


void PulseEvtBuilder::SetSimDataManager(string input_filename)
{
    // close file if already open
    if(fSimDataReader.isOpen())
	fSimDataReader.closeDataFile();
    fSimDataReader.setDataFilename(input_filename);
    fSimDataReader.openDataFile();
}


void PulseEvtBuilder::ReadSimEvent()
{
    fSimDataReader.readEvent();
}


/*void PulseEvtBuilder::SetPulseLibManager(string filename)
{
    map<int, int> detectorMap = fDetectorConfigManager.GetDetectorMap();
    fSimLibManager.setLibraryFile(filename, detectorMap);
    }*/

// ================ Interfaces to Basic Pulse Analysis Classes =====================

//This class calculates RMS, MaxADC, Baseline, BaselineSubtraction and checks if pulse is Saturated
void PulseEvtBuilder::DoBasicPulseCalc(int detNum)
{
    int detType = 0; //for calculating sum of pulses
    //retrieve the vector of pulses for this zip 
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);

       //Construct sum of phonon pulses and store as a special PulseData object
       
       //total sum
       vector<double> phononSumPulse;
       vector<double> phononSumBSPulse;
       vector<double> phononSumBSNPulse;


       // S1 sum (iZIP only)
       vector<double> phononSumS1Pulse;
       vector<double> phononSumS1BSPulse;
       vector<double> phononSumS1BSNPulse;

       // S2 sum (iZIP only)
       vector<double> phononSumS2Pulse;
       vector<double> phononSumS2BSPulse;
       vector<double> phononSumS2BSNPulse;


       // Get the status RQ if available
       // this will be used to remove broken channels
       // when constructing pt

       vector<string>  brokenPhononChannels;
       if (fUserData.DoRead("DET_STATUS_FILE"))
            brokenPhononChannels = fUserData.GetBrokenPhononChannelList(detNum);  


       // ===== loop ZIP pulse collection =======
       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]); 
	  string sensorType     = aPulseData->GetChannelType(); 
	  string chanName       = aPulseData->GetChannelName();
	  string parNameBase    = ChannelMapHelper::GetChannelNameBase(chanName);
	  int    detCode        = aPulseData->GetDetectorCode();

	  detType = aPulseData->GetDetectorType();
	  

          // ------ Set BasicPulseCalc parameters ------

	  BasicPulseCalc tempBasicPulseCalc(sensorType);

          tempBasicPulseCalc.SetSaturationVal(fUserData.GetIntParameter(detNum, parNameBase+ "_SATURATION"));
          tempBasicPulseCalc.SetMinADCVal(fUserData.GetIntParameter(detNum, parNameBase+ "_MINADC"));
	  tempBasicPulseCalc.SetBaselineRange(fUserData.GetIntParameter(detNum, parNameBase+ "_BASELINE_MIN"),
					      fUserData.GetIntParameter(detNum, parNameBase+ "_BASELINE_MAX"));
	  tempBasicPulseCalc.SetPostBaselineRange(fUserData.GetIntParameter(detNum, parNameBase+ "_POSTBASELINE"));


	  tempBasicPulseCalc.SetSensorType(sensorType);

	  //In the past, total gain and normalization were not the same for phonons
	  //as of 2011 they are now the same.  We store both for backwards compatibility
	  //Note, to revert to old phonon normalizations, use DetectorConfigManager::GetPNormADCtoWatts()
	  double normalization = 1.;
	  double totalGain = fDetectorConfigManager.GetTotalGain(detCode, fAdminData.GetEventTime());
	  double bias = fDetectorConfigManager.GetBias(detCode, fAdminData.GetEventTime());

	  if(sensorType == "phonon")
	  {
	    normalization = fDetectorConfigManager.GetPNormADCToAmps(detCode, fAdminData.GetEventTime());
	  }

	  if(sensorType == "charge") 	  
	  {
	    normalization = fDetectorConfigManager.GetQNormADCToVolts(detCode, fAdminData.GetEventTime());
	    double biasTime = fDetectorConfigManager.GetBiasTime(detCode, fAdminData.GetEventTime());
	    tempBasicPulseCalc.SetBiasTime(biasTime);
	  }

	  // store channel settings 
	  tempBasicPulseCalc.SetGain(totalGain);
	  tempBasicPulseCalc.SetBias(bias);
	  tempBasicPulseCalc.SetPulseNorm(normalization);          
	 
	  // ----------- Do Basic pulse calculations  -------------
	  tempBasicPulseCalc.DoCalc(aPulseData->GetRawPulse());
	  
 
          // ----------- store results ------------

	  //store these for convenient access in PulseData for additional pulse analysis calls

	  aPulseData->fBSPulseVector  = tempBasicPulseCalc.GetBaselineSubPulse(); //save baseline subtracted pulse
	  aPulseData->fBSNPulseVector = tempBasicPulseCalc.GetBaselineSubNormPulse(); //save baseline subtracted AND normalized pulse (same as BaselineSub if ISR is not read)
	  
	  //store instance of this class so RQs can be read out later
	  aPulseData->StorePulseAnalysis(tempBasicPulseCalc);

          
          // ---------- construct sum of phonon pulses ---------
	  
          //  phonon only
	  if(aPulseData->IsChargePulse())  continue; 

          //  not using broken channels
          if (find(brokenPhononChannels.begin(), brokenPhononChannels.end(), chanName) != brokenPhononChannels.end()) continue;


          // relative calibration          
	  double pulseCalib = fUserData.GetRelativeCalibration(detNum, detType, chanName);


          // TOTAL phonon pulse  (sum of all phonon channels)

	  if(phononSumPulse.size() == 0)
	  {
	     phononSumPulse = PulseTools::Scale(aPulseData->fPulseVector, pulseCalib); //applying rough calibration factor
	     phononSumBSPulse = PulseTools::Scale(aPulseData->fBSPulseVector, pulseCalib); //applying rough calibration factor
	     phononSumBSNPulse = PulseTools::Scale(aPulseData->fBSNPulseVector, pulseCalib); //applying rough calibration factor
	  
	  } else {
	  
	     vector<double> tempPulse = PulseTools::Scale(aPulseData->fPulseVector, pulseCalib); //applying rough calibration factor
	     vector<double> tempBSPulse = PulseTools::Scale(aPulseData->fBSPulseVector, pulseCalib); //applying rough calibration factor
	     vector<double> tempBSNPulse = PulseTools::Scale(aPulseData->fBSNPulseVector, pulseCalib); //applying rough calibration factor

	     phononSumPulse = PulseTools::SumPulses(phononSumPulse, tempPulse); 				    
	     phononSumBSPulse = PulseTools::SumPulses(phononSumBSPulse, tempBSPulse); 				    
	     phononSumBSNPulse = PulseTools::SumPulses(phononSumBSNPulse, tempBSNPulse); 				    
	  }
	  	


          // sum phonon channels on side 1 

          if (chanName.find("S1")!=string::npos) {
          
           if(phononSumS1Pulse.size() == 0)
	     {
	       phononSumS1Pulse = PulseTools::Scale(aPulseData->fPulseVector, pulseCalib); //applying rough calibration factor
	       phononSumS1BSPulse = PulseTools::Scale(aPulseData->fBSPulseVector, pulseCalib); //applying rough calibration factor
	       phononSumS1BSNPulse = PulseTools::Scale(aPulseData->fBSNPulseVector, pulseCalib); //applying rough calibration factor
	  
	     } else {
	  
	       vector<double> tempPulse = PulseTools::Scale(aPulseData->fPulseVector, pulseCalib); //applying rough calibration factor
	       vector<double> tempBSPulse = PulseTools::Scale(aPulseData->fBSPulseVector, pulseCalib); //applying rough calibration factor
	       vector<double> tempBSNPulse = PulseTools::Scale(aPulseData->fBSNPulseVector, pulseCalib); //applying rough calibration factor

	       phononSumS1Pulse = PulseTools::SumPulses(phononSumS1Pulse, tempPulse); 				    
	       phononSumS1BSPulse = PulseTools::SumPulses(phononSumS1BSPulse, tempBSPulse); 				    
	       phononSumS1BSNPulse = PulseTools::SumPulses(phononSumS1BSNPulse, tempBSNPulse); 				    
	     }
	  }


          // sum phonon channels on side 2

          if (chanName.find("S2")!=string::npos) {
          
           if(phononSumS2Pulse.size() == 0)
	     {
	       phononSumS2Pulse = PulseTools::Scale(aPulseData->fPulseVector, pulseCalib); //applying rough calibration factor
	       phononSumS2BSPulse = PulseTools::Scale(aPulseData->fBSPulseVector, pulseCalib); //applying rough calibration factor
	       phononSumS2BSNPulse = PulseTools::Scale(aPulseData->fBSNPulseVector, pulseCalib); //applying rough calibration factor
	  
	     } else {
	  
	       vector<double> tempPulse = PulseTools::Scale(aPulseData->fPulseVector, pulseCalib); //applying rough calibration factor
	       vector<double> tempBSPulse = PulseTools::Scale(aPulseData->fBSPulseVector, pulseCalib); //applying rough calibration factor
	       vector<double> tempBSNPulse = PulseTools::Scale(aPulseData->fBSNPulseVector, pulseCalib); //applying rough calibration factor

	       phononSumS2Pulse = PulseTools::SumPulses(phononSumS2Pulse, tempPulse); 				    
	       phononSumS2BSPulse = PulseTools::SumPulses(phononSumS2BSPulse, tempBSPulse); 				    
	       phononSumS2BSNPulse = PulseTools::SumPulses(phononSumS2BSNPulse, tempBSNPulse); 				    
	     }
	  }


  
       } // == end loop ZIP pulses  ==



       // ====== Construct PT PulseData =======

       PulseData phononSumPulseData;
       phononSumPulseData.fDetType = detType;
       phononSumPulseData.fDetNum = detNum;
       phononSumPulseData.fDetChannel = ChannelMapHelper::GetPTIndex(detType); //depends on number of physical channels in the det
       phononSumPulseData.fDetCode = ChannelMapHelper::CalcDetCodeBase(detType, detNum) + phononSumPulseData.fDetChannel;
       phononSumPulseData.fChannelName = ChannelMapHelper::GetChannelName(detType, phononSumPulseData.fDetChannel);
       phononSumPulseData.fNADCBins = phononSumPulse.size();

       phononSumPulseData.fIsPhonon = true;
       phononSumPulseData.fIsZip = true;
       phononSumPulseData.fIsCharge = false;
       phononSumPulseData.fIsVeto = false;

       phononSumPulseData.fPulseVector = phononSumPulse;
       phononSumPulseData.fBSPulseVector = phononSumBSPulse;
       phononSumPulseData.fBSNPulseVector = phononSumBSNPulse;

       //store the PT PulseData object
       zipPulseList->push_back(phononSumPulseData);
       


      // ====== Construct S1/S2 PulseData (iZIP only)=======

       if (detType==BatRootTypes::kiZIPSoudan)  {  

         // --- S1 PulseData  -- 

         PulseData phononSumS1PulseData;
         phononSumS1PulseData.fDetType = detType;
         phononSumS1PulseData.fDetNum = detNum;
         phononSumS1PulseData.fDetChannel = ChannelMapHelper::GetPS1Index(detType); //depends on number of physical channels in the det
         phononSumS1PulseData.fDetCode = ChannelMapHelper::CalcDetCodeBase(detType, detNum) + phononSumS1PulseData.fDetChannel;
         phononSumS1PulseData.fChannelName = ChannelMapHelper::GetChannelName(detType, phononSumS1PulseData.fDetChannel);
         phononSumS1PulseData.fNADCBins = phononSumS1Pulse.size();

         phononSumS1PulseData.fIsPhonon = true;
         phononSumS1PulseData.fIsZip = true;
         phononSumS1PulseData.fIsCharge = false;
         phononSumS1PulseData.fIsVeto = false;

         phononSumS1PulseData.fPulseVector = phononSumS1Pulse;
         phononSumS1PulseData.fBSPulseVector = phononSumS1BSPulse;
         phononSumS1PulseData.fBSNPulseVector = phononSumS1BSNPulse;

         //store the S1 PulseData object
         zipPulseList->push_back(phononSumS1PulseData);

         // --- S2 PulseData ---

         PulseData phononSumS2PulseData;
         phononSumS2PulseData.fDetType = detType;
         phononSumS2PulseData.fDetNum = detNum;
         phononSumS2PulseData.fDetChannel = ChannelMapHelper::GetPS2Index(detType); //depends on number of physical channels in the det
         phononSumS2PulseData.fDetCode = ChannelMapHelper::CalcDetCodeBase(detType, detNum) + phononSumS2PulseData.fDetChannel;
         phononSumS2PulseData.fChannelName = ChannelMapHelper::GetChannelName(detType, phononSumS2PulseData.fDetChannel);
         phononSumS2PulseData.fNADCBins = phononSumS2Pulse.size();

         phononSumS2PulseData.fIsPhonon = true;
         phononSumS2PulseData.fIsZip = true;
         phononSumS2PulseData.fIsCharge = false;
         phononSumS2PulseData.fIsVeto = false;

         phononSumS2PulseData.fPulseVector = phononSumS2Pulse;
         phononSumS2PulseData.fBSPulseVector = phononSumS2BSPulse;
         phononSumS2PulseData.fBSNPulseVector = phononSumS2BSNPulse;

         //store the S2 PulseData object
         zipPulseList->push_back(phononSumS2PulseData);
       
        }


    }  // == end if detNum found in map ==

    return; 
}




void PulseEvtBuilder::DoNoiseSelector(int detNum, const string& sensorType)
{
   
    //retrieve the vector of pulses for this zip
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);
 
       // Define maps of OF and RTFT walk P*VWKr20 parameters
       map<string,double> POFmap;
       map<string,double> R20map;
       

       // ==== loop ZIP pulse collection ======

       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]);

	  // we need only phonon pulses
 	  if(aPulseData->IsChargePulse()) { continue; } 
          
          // no need PT/PS1/PS2 information for now
          if(aPulseData->GetChannelName() == "PT" || aPulseData->GetChannelName() == "PS1"
                || aPulseData->GetChannelName() == "PS2")  { continue; };

          // get OF
          POFmap.insert(pair<string,double>(aPulseData->GetChannelName(),aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFamps0")));
          
          // get RTFTwalk parameters (check if parameters are ok)
          if ( fUserData.DoAlgorithm(detNum, "phonon", "ConstFreqRTFTWalkPhonon") )
          {
	    R20map.insert(pair<string,double>(aPulseData->GetChannelName(),aPulseData->GetPulseAnalysis("VarFreqRTFTWalkPhonon").GetRQVal("VWKr20")));
	  }

	}


       // ==== noise events selection ====

       // get PT and Rdel threshold from user settings file
       double ptThresh =  fUserData.GetDoubleParameter(detNum,"PT_THRESHOLD");
       double rdelThresh =  fUserData.GetDoubleParameter(detNum,"RDEL_THRESHOLD");
    
       NoiseSelector tempNoiseSelector;
       tempNoiseSelector.DoCalc(POFmap,R20map,ptThresh,rdelThresh); 
     //  bool isNoise = tempNoiseSelector.GetIsNoise();
 

       // ==== loop again zip pulse collection and store NoiseSelector ====

       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {
         PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
         if(aPulseData->GetChannelType() != sensorType) { continue; }
         if(aPulseData->GetChannelName() == "PT" && !fUserData.DoAlgorithm(detNum,"PT","NoiseSelector")) {continue; }
         if(aPulseData->GetChannelName() == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","NoiseSelector")) {continue; }
         if(aPulseData->GetChannelName() == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","NoiseSelector")) {continue; }
        
         aPulseData->StorePulseAnalysis(tempNoiseSelector);
       }
  } //end if detNum found in map

  return; 
}



void PulseEvtBuilder::DoVarFreqRTFTWalkPhonon(int detNum, const string& sensorType, const string& pulseType)
{
    //retreive the vector of pulses for this zip
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);


       // ==== loop ZIP pulse collection ======

       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {	  
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	  
	  int detChan = aPulseData->GetDetectorChannel();
         
          // only phonon pulses
          if(aPulseData->IsChargePulse()) { continue; }

          // check analysis flag for sum of phonon pulses
          if(aPulseData->GetChannelName() == "PT" && !fUserData.DoAlgorithm(detNum,"PT","VarFreqRTFTWalkPhonon")) {continue; }
	  if(aPulseData->GetChannelName() == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","VarFreqRTFTWalkPhonon")) {continue; }
	  if(aPulseData->GetChannelName() == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","VarFreqRTFTWalkPhonon")) {continue; }
		  
          // get baseline subtracted pulse vector (norm = 1, when isr is not read)
	  vector<double> aPulse = aPulseData->GetBaselineSubNormPulse(); 
	      
          // get normalized pulse standard deviation ('noise' in the signal-to-noise)
	  double pulseSTD;
	  if(aPulseData->GetChannelName() == "PT" || aPulseData->GetChannelName() == "PS1" 
                 || aPulseData->GetChannelName() == "PS2")
	  {
	     //because we are not running BasicPulseCalc on PT ir PS1/PS2 for iZIP
	     //note, we are running Std on baseline subtracted normalized pulse to get right norm
	     pulseSTD = PulseTools::Std(aPulseData->GetBaselineSubNormPulse(), 
					fUserData.GetIntParameter(detNum, "P_BASELINE_MIN"), 
					fUserData.GetIntParameter(detNum, "P_BASELINE_MAX"));
	  }
	  else
	  {
	     //for all other pulses, use the values stored in BasicPulseCalc
	     pulseSTD = aPulseData->GetPulseAnalysis("BasicPulseCalc").GetRQVal("std")/aPulseData->GetPulseAnalysis("BasicPulseCalc").GetRQVal("norm");
	  }

          // get optimal filter pulse height ('signal' in the signal-to-noise)
	  double pulseOFamps = aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFamps");
	  
          // get optimal filter template pulse height (for normalizing the 'signal')
	  double templateMax = fFilterData.GetTemplateMax(detNum,aPulseData->GetChannelName());
	  
          // ----- calculate pulse window  ------
	  
	  int    detCode    =  aPulseData->GetDetectorCode();
          string chanName   =  aPulseData->GetChannelName();
        
          double sampleRate = -999999.; 
          double preTrigger = -999999.; 
          
          if(chanName == "PT" || chanName == "PS1"  || chanName == "PS2") {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
           } else {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detCode);
           }

          
	  //for the peak scan
          int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MIN")));
          int pullback  = fUserData.GetIntParameter(detNum, "P_RTFT_PULLBACK");

// 	  cout <<"In VarFreqRTFTWalkPhonon, windowMin = " << windowMin 
// 	       <<", and windowMax = " << aPulse.size() - pullback
// 	       << endl;

          //  ------ set VarFreqRTFTWalkPhonon -------

          VarFreqRTFTWalkPhonon tempVarFreqRTFTWalkPhonon;

	  tempVarFreqRTFTWalkPhonon.SetPeakWindowMin(windowMin);
	  tempVarFreqRTFTWalkPhonon.SetPeakWindowMax(aPulse.size() - pullback);
	  tempVarFreqRTFTWalkPhonon.SetSampleRate(sampleRate);
	  tempVarFreqRTFTWalkPhonon.SetSensorType(sensorType);

	  //butterworth cutoff frequency is calculated inside SetFilterParameters based on pulseOFamps and pulseSTD
	  if(pulseType == "filtered") 
	     tempVarFreqRTFTWalkPhonon.SetVariableFilterParameters(detChan,
							    fUserData.GetVectDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_1"),
							    fUserData.GetVectDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_2"), 
							    fUserData.GetIntParameter(detNum,"P_RTFT_BUTTERWORTH_ORDER"),
							    fUserData.GetDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_CUTOFF_LOWERLIMIT"),
							    fUserData.GetDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_CUTOFF_UPPERLIMIT"),
							    fUserData.GetDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_SCALEFACTOR"),
							    pulseSTD,
							    pulseOFamps,
							    templateMax,
		                                            fUserData.GetDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_CUTOFF_DEFAULT"));

          // ------- Do RTFTwalk Calc -------

	  tempVarFreqRTFTWalkPhonon.DoCalc(aPulse); 
	  
          // ----------- store results  ------------

	  //store instance of this class so RQs can be read out a little later
	  aPulseData->StorePulseAnalysis(tempVarFreqRTFTWalkPhonon);


       }  // ======= end loop ZIP pulses  =======

   } //end if detNum found in map

    return; 
}





void PulseEvtBuilder::DoRTFTWalkCharge(int detNum, const string& sensorType, const string& pulseType)
{
   //retrieve the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);
      
        // ==== loop ZIP pulse collection ======

      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 string chanName = aPulseData->GetChannelName();

	 // charge pulse only	 
	 if(aPulseData->IsPhononPulse()) { continue; }

	 vector<double> aPulse = aPulseData->GetBaselineSubNormPulse();  //norm = 1, when isr is not read

         // ----- calculate pulse window min ------

	 int    detCode    =  aPulseData->GetDetectorCode();

          double sampleRate = -999999.; 
          double preTrigger = -999999.; 
          
          if(chanName == "QT") {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
           } else {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detCode);
           }
 
                     
         int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"Q_PEAK_WINDOW_MIN")));

 
         // ------ set RTFTWalkCharge parameters ----------
        
         RTFTWalkCharge tempRTFTWalkCharge;

	 tempRTFTWalkCharge.SetPeakWindowMin(windowMin);
	 tempRTFTWalkCharge.SetSampleRate(sampleRate);
	 tempRTFTWalkCharge.SetSensorType(sensorType);
          
          // get normalized Std
         double pulseSTD = aPulseData->GetPulseAnalysis("BasicPulseCalc").GetRQVal("std")/aPulseData->GetPulseAnalysis("BasicPulseCalc").GetRQVal("norm");
	 tempRTFTWalkCharge.SetStd(pulseSTD);
 
         if(pulseType == "filtered")
              tempRTFTWalkCharge.SetFilterParameters(fUserData.GetDoubleParameter(detNum,"Q_RTFT_BUTTERWORTH_CUTOFF"), fUserData.GetIntParameter(detNum,"Q_RTFT_BUTTERWORTH_ORDER")); 

       
         // ---------- do RTFTwalk calc ----------

	 tempRTFTWalkCharge.DoCalc(aPulse, pulseType); 
	 

         // ----------- store results ------------

	 //store instance of this class so RQs can be read out a little later
	 aPulseData->StorePulseAnalysis(tempRTFTWalkCharge);
     
     }  // ======= end loop ZIP pulses  =======
      
   } //end if detNum found in map

   return; 
}




//This is the original algorithm ported from DarkPipe
void PulseEvtBuilder::DoConstFreqRTFTWalkPhonon(int detNum, const string& sensorType,  const string& pulseType)
{
    //retreive the vector of pulses for this zip
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);


       // ==== loop ZIP pulse collection ======

       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {	  
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
         
          // only phonon pulses
          if(aPulseData->IsChargePulse()) { continue; }

          // check analysis flag for sum of phonon pulses
          if(aPulseData->GetChannelName() == "PT" && !fUserData.DoAlgorithm(detNum,"PT","ConstFreqRTFTWalkPhonon")) {continue; }
	  if(aPulseData->GetChannelName() == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","ConstFreqRTFTWalkPhonon")) {continue; }
          if(aPulseData->GetChannelName() == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","ConstFreqRTFTWalkPhonon")) {continue; }



          // get baseline subtracted pulse vector (norm = 1, when isr is not read)
	  vector<double> aPulse = aPulseData->GetBaselineSubNormPulse(); 
	  
          // ----- calculate pulse window  ------

	  int    detCode    =  aPulseData->GetDetectorCode();
          string chanName = aPulseData->GetChannelName();
             
          double sampleRate = -999999.; 
          double preTrigger = -999999.; 
          
          if(chanName == "PT" || chanName == "PS1"  || chanName == "PS2") {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
           } else {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detCode);
           }

        
	  //for the peak scan
          int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MIN")));
          int pullback  = fUserData.GetIntParameter(detNum, "P_RTFT_PULLBACK");

// 	  cout <<"In ConstFrequencyRTFTWalkPhonon, windowMin = " << windowMin 
// 	       <<", and windowMax = " << aPulse.size() - pullback
// 	       << endl;
//Temp?
          //  ------ set RTFTWalkPhonon -------

          ConstFreqRTFTWalkPhonon tempConstFreqRTFTWalkPhonon;

	  tempConstFreqRTFTWalkPhonon.SetPeakWindowMin(windowMin);
	  tempConstFreqRTFTWalkPhonon.SetPeakWindowMax(aPulse.size() - pullback);
	  tempConstFreqRTFTWalkPhonon.SetSampleRate(sampleRate);
	  tempConstFreqRTFTWalkPhonon.SetSensorType(sensorType);

	  if(pulseType == "filtered") 
	     tempConstFreqRTFTWalkPhonon.SetFilterParameters(fUserData.GetDoubleParameter(detNum,"P_RTFT_BUTTERWORTH_CUTOFF_DEFAULT"), fUserData.GetIntParameter(detNum,"P_RTFT_BUTTERWORTH_ORDER")); 
	  

          // ------- Do RTFTwalk Calc -------

	  tempConstFreqRTFTWalkPhonon.DoCalc(aPulse, pulseType); 
	  
          // ----------- store results  ------------

	  //store instance of this class so RQs can be read out a little later
	  aPulseData->StorePulseAnalysis(tempConstFreqRTFTWalkPhonon);


       }  // ======= end loop ZIP pulses  =======

    } //end if detNum found in map

    return; 
}




void PulseEvtBuilder::DoPulseIntegral(int detNum, const string& sensorType, const string& pulseType)
{
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);

      // ----  loop ZIP pulse collection ----

      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	PulseData* aPulseData = &((*zipPulseList)[pulseItr]);

	if(aPulseData->GetChannelType() != sensorType) { continue; }

        // check analysis flag for sum of phonon pulses
        if(aPulseData->GetChannelName() == "PT" && !fUserData.DoAlgorithm(detNum,"PT","PulseIntegral")) {continue; }
        if(aPulseData->GetChannelName() == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","PulseIntegral")) {continue; }
        if(aPulseData->GetChannelName() == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","PulseIntegral")) {continue; }


        // =====  initialize PulseIntegral =====

        PulseIntegral tempPulseIntegral; 


        // ======  Get parameters ======
         
	string chanName = aPulseData->GetChannelName();
	string parNameBase = ChannelMapHelper::GetChannelNameBase(chanName);
	int    detCode = aPulseData->GetDetectorCode();


        // sample rate
        double sampleRate = -999999;
        if(chanName == "QT" || chanName == "PT" || chanName == "PS1"  || chanName == "PS2") 
            sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
        else 
            sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
              

          
        // filer cutoff/order

        double butterCutoff;
        
        if (sensorType=="phonon" && fUserData.DoAlgorithm(detNum, "phonon", "VarFreqRTFTWalkPhonon"))
            butterCutoff = aPulseData->GetPulseAnalysis("VarFreqRTFTWalkPhonon").GetRQVal("VWKCutoff");
        else 
            butterCutoff =  fUserData.GetDoubleParameter(detNum, parNameBase+"_RTFT_BUTTERWORTH_CUTOFF_DEFAULT");
        

        int  butterOrder =  fUserData.GetIntParameter(detNum,parNameBase + "_RTFT_BUTTERWORTH_ORDER");
  	 

        // ===== set parameters in PulseIntegrals =====
 
	if(pulseType == "filtered") 
	    tempPulseIntegral.SetFilterParameters(sampleRate,butterCutoff,butterOrder);

         vector<double> aPulse;
	 aPulse = aPulseData->GetBaselineSubNormPulse();
	 
	// =======  Do integral =======

	 tempPulseIntegral.DoCalc(aPulse, pulseType); 


	// ======= store results ========

	 //store instance of this class so RQs can be read out a little later
	 aPulseData->StorePulseAnalysis(tempPulseIntegral);
    
      }  // -------- end loop ZIP pulses ----------

   } //end if detNum found in map

   return; 
}





void PulseEvtBuilder::DoTailFitPhonon(int detNum, const string& sensorType)
{
     

    // get pulses for detector detNum
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);

    
       //loop over zip pulse collection to check PT threshold and 
       // get PT delay

       double delayTime = 0.0;
       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {	  
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
        	  
          // only PT 
          if(aPulseData->GetChannelName() != "PT") { continue; }


	  // Check threshold using PTOFamps if available, otherwise always fit
          // FIXME: could also check using PTmax or other energy estimate

          if(fUserData.DoAlgorithm(detNum,"PT","OptimalFilterPhonon")) { 
	     
             double ptThresh =  fUserData.GetDoubleParameter(detNum,"PT_THRESHOLD");
             double PTOFamps = aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFamps");
             if(PTOFamps*1e8<ptThresh) return;  // no fit done for that detector 
             delayTime = aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFdelay");

          } 

          // stop loop
          break;
        }



	// Get delay from Qdelay instead  (if set by user)  

        if (fUserData.GetIntParameter(detNum,"P_TAIL_FIT_USE_QDELAY")==1) 
         {
 
              // get charge OF algorithm 
              string analysisName;
              if (fUserData.DoAlgorithm(detNum,"charge","OptimalFilterCharge2X2"))
                  analysisName = "OptimalFilterCharge2X2";
              else if(fUserData.DoAlgorithm(detNum,"charge","OptimalFilterChargeX"))
                  analysisName = "OptimalFilterChargeX";
              else {
                cout <<"\nERROR! PulseEvtBuilder::DoTailFitPhonon: No charge OF delay available."
		 <<"You need to set P_TAIL_FIT_USE_QDELAY = 0 or enable charge OF!"
		 << endl;
	         exit(1);
	      }


              // loop
              double ampTemp = -999999; 
              for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
              {	  
	         PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
        	  
                 // only charge pulses
	         if(aPulseData->IsPhononPulse()) continue; 

                 // get start time (not possible if pulses saturated)
                 if (aPulseData->HasPulseAnalysis(analysisName)) 
                  {
                                       
                    double amp = aPulseData->GetPulseAnalysis(analysisName).GetRQVal("OFvolts")
;                   if (amp>ampTemp) {
                       ampTemp = amp;
                       delayTime =  aPulseData->GetPulseAnalysis(analysisName).GetRQVal("OFdelay"); 
                    }
                  }
               } 

          // check that start time is reasonable (might not be if noise trace)
          if (abs(delayTime)> 50e-6) delayTime = 0.0;
        }

       


        // loop pulse again and fit tail using single exponentiol
        for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
         {	  
  	   PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
           string chanName   = aPulseData->GetChannelName();    
           int  detCode    = aPulseData->GetDetectorCode();
           int  detType = aPulseData->GetDetectorType();

           //check whether processing is desired for this sensor type
	   if(aPulseData->GetChannelType() != sensorType) { continue; }
	  
           // check PT/PS1/PS2
           if(aPulseData->GetChannelName() == "PT" && !fUserData.DoAlgorithm(detNum,"PT","TailFitPhonon")) {continue; }
	   if(aPulseData->GetChannelName() == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","TailFitPhonon")) {continue; }
           if(aPulseData->GetChannelName() == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","TailFitPhonon")) {continue; }


           // Get pulse 
	   vector<double> aBSNPulse;	 
	   aBSNPulse = aPulseData->GetBaselineSubNormPulse();  //when ISR is not read, norm = 1    



           // Get digitizer informations
           double sampleRate = -999999.; 
           double preTrigger = -999999.; 
           double traceLength = -999999.;

           if(chanName == "PT" || chanName == "PS1"  || chanName == "PS2") {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
              traceLength =  fDetectorConfigManager.GetTraceLength(detNum,sensorType);
           } else {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detCode);
              traceLength  =  fDetectorConfigManager.GetTraceLength(detCode);
           }


	   // Define fit window
                 
           // 1. Assume pulse start at trigger time
	   int windowMin = (int) floor(sampleRate*(preTrigger+fUserData.GetDoubleParameter(detNum,"P_TAIL_FIT_START")));
           int windowMax = (int) (traceLength - (sampleRate*80e-6)); 
        
           // 2. Adjust with charge OF delay 
           windowMin   = (int) (windowMin + floor(sampleRate*delayTime));
           windowMax   = (int) (windowMax + floor(sampleRate*delayTime));   

           // calculate mean last bins
           double pulseMeanEnd = PulseTools::Baseline(aBSNPulse,traceLength-100,traceLength-50);


           // calculate baseline std (not done in BasicPulseCalc for PT/PS1/PS2
           int baselineMin = fUserData.GetIntParameter(detNum, "P_BASELINE_MIN");
           int baselineMax = fUserData.GetIntParameter(detNum, "P_BASELINE_MAX");
           double pulseStd = PulseTools::Std(aBSNPulse, baselineMin, baselineMax);


           // get fall time (convert in second)
           double tau = fUserData.GetTailFitFallTime(detNum,detType, chanName)*1e-6;

           // First guess fit parameters
           double par1 = aBSNPulse[windowMin];  
           double par2 = -1/tau;
           double par3 = pulseMeanEnd;


           // ==== Single Exponential Fit ====

	   // Define for TailFitPhonon 
 	   SingleExponentialFit myExpFit("TailFitPhonon");

           // initialize fit parameters
	   myExpFit.InitializeFitParameters(par1,par2, par3, pulseStd);
           myExpFit.SetSampleTime(1/sampleRate);
 	   myExpFit.SetFitWindow(windowMin,windowMax);

           // Fall Time constraint  (fit paramter 2)
           myExpFit.SetFitParameterConstraintFlag(2, fUserData.GetIntParameter(detNum,"P_TAIL_FIT_TAU_FLAG"));  
              
           if (fUserData.GetIntParameter(detNum,"P_TAIL_FIT_TAU_FLAG")==1) {
              double tauSigma = fUserData.GetTailFitFallTimeSigma(detNum,detType, chanName)*1e-6;
              double par2Min = -1/(tau-tauSigma);
              double par2Max = -1/(tau+tauSigma);
              myExpFit.SetFitParameterRange(2,par2Min,par2Max); 
           }

   
           // Fit
 	   myExpFit.DoCalc(aBSNPulse);
	 
        

	   //store instance of this class so RQs can be read out a little later
	   aPulseData->StorePulseAnalysis(myExpFit);

        
        }

   } //end if detNum found in map

    return; 
}







// =========== Interfaces to User Analysis Classes Go Here - DO NOT modify or copy this comment (for auto_analysis) ================

//Modify this as needed to correctly call your class


//A class to manage fake pulse simulation using randoms for
//the noise.   A random is passed into the analysis class where
//the fake pulse is constructed from the templates and the noise.
//Then the fake pulse is retrieved and stored in the PulseData objects in
//place of the original trace.  All subsequent analyses run on this event 
//will run on the fake pulse.  The class is meant to only run on randoms
void PulseEvtBuilder::DoSimulatePhononFromRandoms(int detNum)
{
    //skip if not a random triggered event
    //BatRoot main should take care of this check, 
    //repeating here just in case
    if(fRawReader.GetEventCategory() != 0x1)
      return;

    // get the event energy
    map<string, double> eventEnergies = fSimDataReader.getEventEnergies(detNum);

    // do nothing if there is no energy info for this detector
    if(eventEnergies.size() == 0)
        return;
    
    //retrive the vector of pulses for this zip
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
	//Get the pulse list for this zip
	zipPulseList = &(mapItr->second);

	//loop over zip pulse collection
	for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
	{	  
	    PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	    string chanName   = aPulseData->GetChannelName();
    
	    // only consider phonon channels
	    if(chanName.find("P")!=string::npos) 
	    {
		//Setup simulation class
		SimulateFromRandoms tempSimulateFromRandoms;
	    
		//Get the trace (should be a random)
		vector<double> aPulse = aPulseData->GetBaselineSubNormPulse();  
	    
		// skip the injection process if particular channel is turned off
		if(chanName == "PT" && fUserData.GetIntParameter("DO_PTSIM")!=1)
		    continue;
		if((chanName == "PS1" || chanName == "PS2") &&
		   fUserData.GetIntParameter("DO_PSIDESSIM")!=1)
		    continue;
		if(chanName != "PT" && chanName != "PS1" && chanName != "PS2" &&
		   fUserData.GetIntParameter("DO_PCHANSIM")!=1)
		    continue;

		vector<double>   templateTime = fFilterData.GetTemplateTime(detNum,chanName);
		tempSimulateFromRandoms.LoadPTemplate(templateTime);
		
		// get the calibration for this channel
		int detType = aPulseData->GetDetectorType();
		double ptCal = fUserData.GetOverallPTCalibrationTI(detNum, detType);
		double pRelCal;
		if(chanName == "PT" || chanName == "PS1" || chanName == "PS2")
		    pRelCal = 1.0;
		else
		    pRelCal = fUserData.GetRelativeCalibration(detNum, detType, chanName);

		// inject the pulse
		tempSimulateFromRandoms.SimPMonoenergetic(aPulse, eventEnergies[chanName], ptCal * pRelCal); 	   
		
		//Overwrite the baseline subtracted, normalized pulse 
		//with the modified trace.  All fitting routines run 
		//after this point will run on the modified trace.
		aPulseData->fBSNPulseVector = aPulse;
		
		//store instance of this class so RQs can be read out a little later
		aPulseData->StorePulseAnalysis(tempSimulateFromRandoms);    
	    }
	}
   } //end if detNum found in map

    return; 
}


/*void PulseEvtBuilder::DoSimulateFromPulse(int detNum)
{
    //skip if not a random triggered event
    //BatRoot main should take care of this check, 
    //repeating here just in case
    if(fRawReader.GetEventCategory() != 0x1)
	return;

    // get the event energies
    map<string, double> eventEnergies = fSimDataReader.getEventEnergies(detNum);

    // do nothing if there is no energy info for this detector
    if(eventEnergies.size() == 0)
        return;

    // get pulse from the library
    fSimLibManager.readRandomEvent(detNum);
    map<string, vector<double>* > pulseMap = fSimLibManager.getPulseMap();
    map<string, double> chanAmp = fSimLibManager.getChanAmps();
    
    //retrive the vector of pulses for this zip
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
	//Get the pulse list for this zip
	zipPulseList = &(mapItr->second);
	   
	//loop over zip pulse collection
	for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
	{	  
	    PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	    string chanName   = aPulseData->GetChannelName();
	     
	    // check whether simulation is turned on for this channel
	    if(fUserData.GetIntParameter("DO_PHONONSIM") != 1 && chanName[0] == 'P')
		continue;
	    if(fUserData.GetIntParameter("DO_CHARGESIM") != 1 && chanName[0] == 'Q')
		continue;
	    if(fUserData.GetIntParameter("DO_PTSIM") != 1 && chanName == "PT")
		continue;
	    if(fUserData.GetIntParameter("DO_PSIDESSIM") != 1 && (chanName == "PS1" || chanName == "PS2"))
		continue;
	    if(fUserData.GetIntParameter("DO_PCHANSIM") != 1 && chanName[0] == 'P' && chanName[2] == 'S')
		continue;

            // Setup simulation class
	    SimulateFromRandoms tempSimulateFromRandoms;
		
	    // Get the trace (should be a random)
	    vector<double> aPulse = aPulseData->GetBaselineSubNormPulse();  
		
	    // get the calibration for this channel
	    int detType = aPulseData->GetDetectorType();

	    double chanCal;
	    //if(chanName[0] == 'P')
	    chanCal = fUserData.GetOverallPTCalibrationTI(detNum, detType);
	    //else if(chanName[0] == 'Q')
	    //chanCal = fUserData.GetQCalibration(detNum, detType, chanName);
	    //else
	    //std::cout << "PulseEvtBuilder::DoSimulateFromPulse: ERROR! Cannot find calibration info for channel " << chanName << "." << std::endl;
 
	    // inject the pulse
	    tempSimulateFromRandoms.SimPwDataTemplate(aPulse, *(pulseMap[chanName]), eventEnergies["PT"], chanCal, chanAmp, chanName);
		
	    //Overwrite the baseline subtracted, normalized pulse 
	    //with the modified trace.  All fitting routines run 
	    //after this point will run on the modified trace.
	    aPulseData->fBSNPulseVector = aPulse;
		
	    //store instance of this class so RQs can be read out a little later
	    aPulseData->StorePulseAnalysis(tempSimulateFromRandoms);
	}
    } //end if detNum found in map
    
    return; 
    }*/





//Similar to above, a class to manage fake pulse simulation using randoms for
//the noise. The class is meant to only run on randoms
void PulseEvtBuilder::DoSimulateChargeFromRandoms(int detNum)
{
  //skip if not a random triggered event
  //BatRoot main should take care of this check, 
  //repeating here just in case
  if(fRawReader.GetEventCategory() != 0x1)
    return;

  // get energy for this channel
  map<string, double> eventEnergies = fSimDataReader.getEventEnergies(detNum);

  // do nothing if there is no energy info for this detector                                                                                                                                     
  if(eventEnergies.size() == 0)
      return;

  //retrive the vector of pulses for this zip
  vector<PulseData>* zipPulseList;
  map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
  if(mapItr != fMapOfZipPulses.end())
  {
    //Get the pulse list for this zip
    zipPulseList = &(mapItr->second);

    //loop over zip pulse collection
    for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
    {    
      PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
      string chanName  = aPulseData->GetChannelName();
      
      //For Q simulation
      if(chanName.find("Q")!=string::npos && fUserData.GetIntParameter("DO_CHARGESIM")==1) 
      {
        //Setup simulation class
        SimulateFromRandoms tempSimulateFromRandoms;

        //Get the trace (should be a random)
        vector<double> aPulse = aPulseData->GetBaselineSubNormPulse();  

        // retrieve templates
        vector<double> aQTemplate = fFilterData.GetTemplateTime(detNum,chanName);
        vector<double> aQTemplateX = fFilterData.GetTemplateTime(detNum,chanName+"X");
        tempSimulateFromRandoms.LoadQTemplates(aQTemplate, aQTemplateX);

        // get the calibration for this channel
        int detType = aPulseData->GetDetectorType();
        double pulseCalib = fUserData.GetQCalibration(detNum, detType, chanName);
        
        // inject pulse into charge template
        tempSimulateFromRandoms.SimQMonoenergetic(aPulse, eventEnergies, pulseCalib, chanName);

        //Overwrite the baseline subtracted, normalized pulse 
        //with the modified trace.  All fitting routines run 
        //after this point will run on the modified trace.
        aPulseData->fBSNPulseVector = aPulse;

        //store instance of this class so RQs can be read out a little later
        aPulseData->StorePulseAnalysis(tempSimulateFromRandoms);
      }
    }
  }

  return;
}



///////////////////////////////////////////////////////////////////////////


//Modify this as needed to correctly call your class
void PulseEvtBuilder::DoOptimalFilterPhononNS(int detNum, const string& sensorType)
{
     
    //retrive the vector of pulses for this zip
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);

       //loop over zip pulse collection
       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {	  
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 string chanName   = aPulseData->GetChannelName();    
	 int    detCode    = aPulseData->GetDetectorCode();
	  
          //check whether processing is desired for this sensor type
	  if(aPulseData->GetChannelType() != sensorType) { continue; }
	  
          //analysis flag for sum of phonon pulses
          if(aPulseData->GetChannelName() != "PT" || !fUserData.DoAlgorithm(detNum,"PT","OptimalFilterPhononNS")) { continue; }

	  OptimalFilterPhononNS tempOptimalFilterPhononNS;
          tempOptimalFilterPhononNS.SetVerbosity(0);  //for diagnostics
          tempOptimalFilterPhononNS.SetVerbosityRange(0,5);  //for diagnostics

	  // ------- getting templates for OF ---------

	  vector<TComplex> templateFFT = fFilterData.GetTemplateFFT(detNum,chanName);

          // ------- getting normalization for OF ---------

	  SprseMatrix COVpd;
          COVpd = fFilterData.GetCOV_PD(detNum,chanName);
	  SprseMatrix COVbase;
          COVbase = fFilterData.GetCOV_BASE_HIST(detNum,chanName);
          //cout << "COVpd is: " << COVpd.GetNrow() << "x" << COVpd.GetNcol() << endl;
          //cout << "COVbase is: " << COVbase.GetNrow() << "x" << COVbase.GetNcol() << endl;
          //cout << "COVpd is: " << COVpd.size1() << "x" << COVpd.size2() << endl;
          //cout << "COVbase is: " << COVbase.size1() << "x" << COVbase.size2() << endl;
	  double normFFT = fFilterData.GetNormFFT(detNum,chanName);
	  double sigToNoiseSq = fFilterData.GetSigToNoiseSq(detNum,chanName);

          // ------- calculate optimal filter window --------
         

          double sampleRate = -999999.; 
          double preTrigger = -999999.; 
          double traceLength = -999999.;

          if(chanName == "PT" || chanName == "PS1"  || chanName == "PS2") {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
              traceLength =  fDetectorConfigManager.GetTraceLength(detNum,sensorType);
          } else {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detCode);
              traceLength  =  fDetectorConfigManager.GetTraceLength(detCode);
          }

          int postTriggerBins = (int)traceLength - (int)floor(sampleRate * preTrigger);
  
          int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MIN")));
          int windowMax  = (int) floor(sampleRate * (preTrigger + fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MAX")));

	  int win1 = windowMax - (int) floor(sampleRate * preTrigger);
	  int win2 = windowMin + postTriggerBins + 1;
	 



          // ------- set OptimalFilterPhonon  parameters ---------


          tempOptimalFilterPhononNS.LoadTemplates(templateFFT); 
          tempOptimalFilterPhononNS.LoadNormalizations(normFFT, sigToNoiseSq, COVpd, COVbase); 
	
	 
	  //set timing parameters
	  tempOptimalFilterPhononNS.SetSampleTime(1.0/sampleRate);
	  tempOptimalFilterPhononNS.SetWindows(win1,win2);


          //get some parameters like chiwindow and threshold and israndom
	  double ptThresh =  fUserData.GetDoubleParameter(detNum,"PT_THRESHOLD");
	  int chihalfwin =  fUserData.GetIntParameter(detNum,"P_NSPEAK_CHIWINDOW_HALF");
	  //chiwindow not in input file yet
	  //int chiWin =  fUserData.GetIntParameter(detNum,"PT_CHIWINNS");
	  bool  isRandom = (fRawReader.GetEventCategory() == 0x1 ? true : false);
	  
	  //set the chi window and thresholds (currently need to be loaded all BEFORE loading OF values)
	  tempOptimalFilterPhononNS.SetChiWidth(chihalfwin); //the search window about the OFDelay
	  tempOptimalFilterPhononNS.LoadThresholds(0.0,0); //zero threshold in class, enforce threshold from here 
	  tempOptimalFilterPhononNS.SetIsRandom(isRandom); 
          //don't print randoms if verbose mode on
	  tempOptimalFilterPhononNS.SetPrintRandom(false); 

	  //get the standard phonon OF values
	  double PTOFAmps = aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFamps");
	  double PTOFAmps0 = aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFamps0");
	  double PTOFDelay = aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFdelay");
	  double PTOFChisq = aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFchisq");

	  //set the OFAmps values
	  tempOptimalFilterPhononNS.LoadOFParams(PTOFAmps,PTOFDelay);


          // -------  do Optimal Filter ------------	 

	  //Get Pulse and normalize if ISR file is being read;
	  vector<double> aBSNPulse;	 
	  aBSNPulse = aPulseData->GetBaselineSubNormPulse();  //when ISR is not read, norm = 1    

	
          if(PTOFAmps*1e8>ptThresh || isRandom){
            tempOptimalFilterPhononNS.DoCalc(aBSNPulse);
          }
	  
	  //store instance of this class so RQs can be read out a little later
	  aPulseData->StorePulseAnalysis(tempOptimalFilterPhononNS);

          if(tempOptimalFilterPhononNS.GetVerbosity()>0){
            double finalAmp = aPulseData->GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFamps");
            double amp0 = aPulseData->GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFamps0");
            double chisq = aPulseData->GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFchisq");
            double delay = aPulseData->GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFdelay");
            double finalAmpbig = aPulseData->GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFbigamps");
            double chisqbig = aPulseData->GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFbigchisq");
            double delaybig = aPulseData->GetPulseAnalysis("OptimalFilterPhononNS").GetRQVal("NFbigdelay");
            cout << "-------------------------------------READBACK-----------------------------" << endl;
            cout << "Results NSOF: " << "fAmp = " << finalAmp << " fAmp0 = " << amp0 << " fChisq = " << chisq << " fDelay = " << delay << endl;
            cout << "Results NSOF big: " << "fAmpBig = " << finalAmpbig << " fAmp0 = " << amp0 << " fChisqBig = " << chisqbig << " fDelayBig = " << delaybig << endl;
            cout << "Results OF: " << "fAmp = " << PTOFAmps << " fAmp0 = " << PTOFAmps0 << " fChisq = " << PTOFChisq << " fDelay = " << PTOFDelay << endl;
            cout << "--------------------------------------------------------------------------" << endl;
          }
       }

   } //end if detNum found in map

    return; 
}


void PulseEvtBuilder::DoOptimalFilterCharge2X2(int detNum)
{

    ////////////////////////////////////////////////
    // Calculate OptimalFilterCharge 2X2 
    //
    //
    // IMPORTANT NOTE: 
    //
    //   If broken channels (Q*status RQ > 0)
    //    ==>  single pulse fitting all channels same
    //         side and  Z time constraint  disabled
    //
    ////////////////////////////////////////////////





    // ====================================
    // Loop pulse list and fill pulse maps
    // ====================================


    // NOTE: no particular pulse order is assumed...

  
    // Define pulse maps (channel name -> pulse vector)
    map<string, vector<double> > aPulseMap2x2; // for 2X2 fitting
    map<string, vector<double> > aPulseMap1x1; // for single pulse fitting 
  
    // get vector of pulseData for this zip if available
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr == fMapOfZipPulses.end()) return; // No pulses for this detector


    // get the pulse list for this zip
    zipPulseList = &(mapItr->second);
        
    // get broken charge side list
    vector<string> brokenSides;    
    if (fUserData.DoRead("DET_STATUS_FILE"))
           brokenSides = fUserData.GetBrokenChargeSideList(detNum);  
   

    // save detType info for later use
    int detType = -999999;

    // initialize PTOFdelay for later use
    double PTdelay = -999999;

    // loop  pulses
    for(uint ipulse = 0; ipulse < zipPulseList->size(); ipulse++)
      {	  
	PulseData* aPulseData = &((*zipPulseList)[ipulse]);
	detType = aPulseData->GetDetectorType();
	
	if(detType != BatRootTypes::kZIPDetType &&
	   detType != BatRootTypes::kMZIPDetType &&
	   detType != BatRootTypes::kiZIPSoudan &&
           detType != BatRootTypes::kCDMSliteSoudanI)
	  {
	    cout <<"\nERROR! PulseEvtBuilder::DoOptimalFilterCharge2X2  OptimalFilterCharge2X2 calculation "
                 <<" not implemented for this detector type."
		 <<" Deactivate DO_CHARGE_ALGORITHM: OptimalFilterCharge2X2 in the processing settings until properly implemented."
		 << endl;
	    exit(1);
	  }

        //let's get PTOFdelay
        if (fUserData.GetIntParameter(detNum, "Q_PTDELAY_CONSTRAINT")) {
            if(aPulseData->GetChannelName() == "PT" && fUserData.DoAlgorithm(detNum,"PT","OptimalFilterPhonon"))
                PTdelay = aPulseData->GetPulseAnalysis("OptimalFilterPhonon").GetRQVal("OFdelay");
        }

	//we want only charge pulses
	if(aPulseData->IsPhononPulse()) continue;
	  
	// get pulse name
	string chanName = aPulseData->GetChannelName(); 

	// we only want QI/QO channels (could be generalized in future)
        if (chanName.find("QI")==string::npos && 
	    chanName.find("QO")==string::npos) continue;
        
	// If a single  pulse is saturated, NO OptimalFilter
        if (aPulseData->GetPulseAnalysis("BasicPulseCalc").GetRQVal("sat") != 0) { 
           aPulseMap2x2.clear();
           return;
        }

   
	// Fill maps: 1x1 for broken charge side, 2x2 for other side
	string side = "S";
	if (chanName.find("S1") !=string::npos) side = "S1";
	if (chanName.find("S2") !=string::npos) side = "S2";


        if (find(brokenSides.begin(),brokenSides.end(), side) != brokenSides.end())
           aPulseMap1x1[chanName] = aPulseData->GetBaselineSubNormPulse();
        else
           aPulseMap2x2[chanName] = aPulseData->GetBaselineSubNormPulse();
      

      } 

    // No Z time constraint if one side broken
    bool disableZtimeConstraint = false;
    if (!aPulseMap1x1.empty()) disableZtimeConstraint = true;

   
  
    // ==============================================
    // OK, we have maps of non saturated QI/QO pulses
    // let's do the optimal filter
    // ==============================================


    // ---- Optimal filter Window(s) ----
       
    double sampleRate = fDetectorConfigManager.GetSampleRate(detNum, "charge");
    double preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum, "charge");
    int traceLength = (int) fDetectorConfigManager.GetTraceLength(detNum, "charge");	 
    int postTriggerBins = (int)traceLength - (int) floor(sampleRate * preTrigger);
    
   
    double shiftPT = 0;
    if (PTdelay>-999999) 
       shiftPT = sampleRate * PTdelay;


    // X window 
    int qxwinMin = (int) floor(shiftPT - sampleRate*fUserData.GetDoubleParameter(detNum,"Q_PEAK_WINDOW_MIN"));
    int qxwinMax = (int) floor(shiftPT + sampleRate*fUserData.GetDoubleParameter(detNum,"Q_PEAK_WINDOW_MAX"));
   
    // check if negative shift 
    if (qxwinMax<0)
        qxwinMax = qxwinMax + traceLength + 1;

    if (qxwinMin<0)
        qxwinMin = qxwinMin + traceLength + 1;


    // Z window (only for iZIPs)
    int qzwinMin = 0;
    int qzwinMax = 0;
    if (detType == BatRootTypes::kiZIPSoudan && fUserData.GetIntParameter(detNum, "Q_Z_DELAY_CONSTRAINT")) {
       qzwinMin  = (int) floor(sampleRate * fUserData.GetDoubleParameter(detNum,"Q_Z_WINDOW_MIN"));
       qzwinMax  = (int) ceil(sampleRate * fUserData.GetDoubleParameter(detNum,"Q_Z_WINDOW_MAX"));      
     }

    


    // ------------------------
    // CASE Optimal Filter 2x2
    // ------------------------


    if (aPulseMap2x2.size()>0) {
      

       // Initalize OptimalFilterCharge2x2 
       OptimalFilterNxN myOptimalFilterCharge2x2;
         
      // ---- Load OF parameters ----
  

      // Windows
      myOptimalFilterCharge2x2.SetSampleTime(1.0/sampleRate);  	    
      myOptimalFilterCharge2x2.SetXwindows(qxwinMin, qxwinMax,traceLength); 
      myOptimalFilterCharge2x2.SetZwindows(qzwinMin, qzwinMax); 
   
      // Do the delay interpolation?
      myOptimalFilterCharge2x2.SetDelayInterpolateFlag(fUserData.GetIntParameter(detNum, "Q_DELAY_INTERPOLATE"));


      // Do Z time constraint?
      if (disableZtimeConstraint) 
         myOptimalFilterCharge2x2.SetZdelayConstraintFlag(0);
      else
         myOptimalFilterCharge2x2.SetZdelayConstraintFlag(fUserData.GetIntParameter(detNum, "Q_Z_DELAY_CONSTRAINT"));



      // Noise/template informations
      map<string, bool> isQinverseLoaded;
      isQinverseLoaded["S"] = false;
      isQinverseLoaded["S1"] = false;
      isQinverseLoaded["S2"] = false;

      map<string, vector<double> >::iterator pulseIter;
      for (pulseIter = aPulseMap2x2.begin(); pulseIter!=aPulseMap2x2.end();++pulseIter) 
       {
	 // channel name + xtalk name
	 string chanName = pulseIter->first;
	 string chanNameX = chanName + "X"; 

	
	 // Get information from filter file
	
	 // templates/OF
	 vector<TComplex> templateConjNoiseFFT = fFilterData.GetTemplateConjNoiseFFT(detNum,chanName);
	 vector<TComplex> templateConjNoiseFFTX = fFilterData.GetTemplateConjNoiseFFT(detNum,chanNameX); 
	 vector<TComplex> templateFFT = fFilterData.GetTemplateFFT(detNum,chanName);
	 vector<TComplex> templateFFTX = fFilterData.GetTemplateFFT(detNum,chanNameX); 

	 // normalizations
	 vector<double> noiseFFTsq = fFilterData.GetNoiseFFTsq(detNum,chanName);
	 double templateMax = fFilterData.GetTemplateMax(detNum,chanName);
	
	 // load into OptimalFilterCharge2x2
	 myOptimalFilterCharge2x2.LoadTemplates(templateFFT, templateConjNoiseFFT,chanName); 
         myOptimalFilterCharge2x2.LoadTemplates(templateFFTX, templateConjNoiseFFTX,chanNameX); 
         myOptimalFilterCharge2x2.LoadNormalizations(noiseFFTsq, templateMax, chanName); 
	
   
	 // get Qinverse (one per side)
	 string side = "S";
	 if (chanName.find("S1") !=string::npos) side = "S1";
	 if (chanName.find("S2") !=string::npos) side = "S2";
	
	 if (!isQinverseLoaded[side]) {
	   myOptimalFilterCharge2x2.LoadWinverse(fFilterData.GetQXtalkInverseMatrix(detNum,side),side);
	   isQinverseLoaded[side]= true;
	 }
      }

     
     // ---- Do Optimal Filter ----
      
     myOptimalFilterCharge2x2.DoCalc(aPulseMap2x2); 
    
     // store RQ in pulse data
     for(uint ipulse = 0; ipulse < zipPulseList->size(); ipulse++)
      {	
	PulseData* aPulseData = &((*zipPulseList)[ipulse]);
	string chanName = aPulseData->GetChannelName();
	
	// check if it is in the pulse map
	if (aPulseMap2x2.find(chanName) == aPulseMap2x2.end()) continue;

	// store OF
	myOptimalFilterCharge2x2.StoreAs(chanName);
	aPulseData->StorePulseAnalysis(myOptimalFilterCharge2x2);
      }    
  
   } // done 2x2 fitting!



   // ------------------------
   // CASE Single Pulse Fitting
   // ------------------------

   if (aPulseMap1x1.size()>0) {

      // loop pulses and fit
      map<string, vector<double> >::iterator pulseIter;
      for (pulseIter = aPulseMap1x1.begin(); pulseIter!=aPulseMap1x1.end();++pulseIter) 
        {

	 // channel name 
	 string chanName = pulseIter->first;
    
         // pulse
         vector<double> pulse = pulseIter->second;
  
         // Get information from filter file
	 vector<TComplex> templateConjNoiseFFT = fFilterData.GetTemplateConjNoiseFFT(detNum,chanName);
	 vector<TComplex> templateFFT = fFilterData.GetTemplateFFT(detNum,chanName);
	 vector<double> noiseFFTsq = fFilterData.GetNoiseFFTsq(detNum,chanName);
	 double templateMax = fFilterData.GetTemplateMax(detNum,chanName);
         double sigToNoiseSq = fFilterData.GetSigToNoiseSq(detNum,chanName);
   
         // Initalize OptimalFilterCharge1x1 
         OptimalFilterNxN myOptimalFilterCharge1x1("SingleChargePulse");
         
    
	 // load into  myOptimalFilterCharge1x1
	 myOptimalFilterCharge1x1.LoadTemplates(templateFFT, templateConjNoiseFFT,chanName); 
         myOptimalFilterCharge1x1.LoadNormalizations(sigToNoiseSq, noiseFFTsq, templateMax, chanName); 
     

         // Windows
         myOptimalFilterCharge1x1.SetSampleTime(1.0/sampleRate);  	    
         myOptimalFilterCharge1x1.SetXwindows(qxwinMin, qxwinMax,traceLength); 
   

         // Do the delay interpolation?
         myOptimalFilterCharge1x1.SetDelayInterpolateFlag(fUserData.GetIntParameter(detNum, "Q_DELAY_INTERPOLATE"));


         // do Optimal Filter
         myOptimalFilterCharge1x1.DoCalc(pulse, chanName); 
	 
         // store RQ in pulse data
         for(uint ipulse = 0; ipulse < zipPulseList->size(); ipulse++)
            {	
	      PulseData* aPulseData = &((*zipPulseList)[ipulse]);
	      string chanNamePulseData = aPulseData->GetChannelName();
	
	       // check if current channel
	       if (chanNamePulseData.compare(chanName) !=0) continue;

	       // store OF
	       myOptimalFilterCharge1x1.StoreAs(chanName);
	       aPulseData->StorePulseAnalysis(myOptimalFilterCharge1x1);
            }	 
	}
    } // done single pulse fitting

      

    return; 
}





//Modify this as needed to correctly call your class
void PulseEvtBuilder::DoPSDIntegralPhonon(int detNum, const string& sensorType)
{

    //retrive the vector of pulses for this zip
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);

       //loop over zip pulse collection
       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {	  
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	  
          //check whether processing is desired for this sensor type
	  if(aPulseData->GetChannelType() != sensorType) { continue; }
	  

          // analysis flag for sum of phonon pulses 
          // (FIXME: PT only algorithm)
          if(!(aPulseData->GetChannelName() == "PT" && fUserData.DoAlgorithm(detNum,"PT","PSDIntegralPhonon"))) { continue; }
       

          //Get the pulse
          //other options are: GetRawPulse, GetBaselineSubtractedNormPulse (norm = 1 when isr is not read)
	  vector<double> aPulse = aPulseData->GetBaselineSubPulse();  
	  
	  // Get phonon signal sampling rate
          string chanName = aPulseData->GetChannelName();
          double sampleRate = -999999;

          if(chanName == "PT" || chanName == "PS1"  || chanName == "PS2") 
            sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
          else 
            sampleRate =  fDetectorConfigManager.GetSampleRate(aPulseData->GetDetectorCode());
         

	  PSDIntegralPhonon tempPSDIntegralPhonon;

	  tempPSDIntegralPhonon.DoCalc(aPulse, sampleRate); 
	  
	  //store instance of this class so RQs can be read out a little later
	  aPulseData->StorePulseAnalysis(tempPSDIntegralPhonon);
       }

   } //end if detNum found in map

    return; 
}


void PulseEvtBuilder::DoWedgeFitPhonon(int detNum, const string& sensorType)
{

    // === retrieve pulse list for this zip ==  
    vector<PulseData>* zipPulseList;
    map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
    if(mapItr != fMapOfZipPulses.end())
    {
       //Get the pulse list for this zip
       zipPulseList = &(mapItr->second);

       // === loop over zip pulse collection ===
       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {	  
	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	  
          // ======= check whether processing is desired for this pulse type == 
	  if(aPulseData->GetChannelType() != sensorType) { continue; }
	  if(aPulseData->GetChannelName() == "PT" && !fUserData.DoAlgorithm(detNum,"PT","WedgeFitPhonon")) { continue; }
          if(aPulseData->GetChannelName() == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","WedgeFitPhonon")) { continue; }
          if(aPulseData->GetChannelName() == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","WedgeFitPhonon")) { continue; }

          // ======= don't fit if pulse is noise =======
          if((int)aPulseData->GetPulseAnalysis("NoiseSelector").GetRQVal("isnoise")) { continue;}
       
          // ======= Get the baseline subtracted pulse vector ======= 
	  vector<double> aPulse = aPulseData->GetBaselineSubPulse();  
	 
          // ======= initialize WedgeFitPhonon class =======
	  WedgeFitPhonon tempWedgeFitPhonon;

                   
          // ======= get  parameters =======
          
	  // get constants from user settings file

          int binSStart = fUserData.GetIntParameter(detNum, "P_WF_BIN_START_SUB");	
	  int binAEnd = fUserData.GetIntParameter(detNum, "P_WF_BIN_END_ADD");
	  int binSt0 = fUserData.GetIntParameter(detNum, "P_WF_BIN_T0PAR_SUB");

	  double par1Start = fUserData.GetDoubleParameter(detNum, "P_WF_PAR1_START");
	

          // RMS
          double pulseRMS = aPulseData->GetPulseAnalysis("BasicPulseCalc").GetRQVal("std");
         
          // timing informations from VarFreqRTFTWalkPhonon
          int time10 = (int) (aPulseData->GetPulseAnalysis("VarFreqRTFTWalkPhonon").GetRQVal("VWKr10")*1.25e6);
          int time20 = (int) (aPulseData->GetPulseAnalysis("VarFreqRTFTWalkPhonon").GetRQVal("VWKr20")*1.25e6);
	  int time60 = (int) (aPulseData->GetPulseAnalysis("VarFreqRTFTWalkPhonon").GetRQVal("VWKr60")*1.25e6);

          // max pulse from VarFreqRTFTWalkPhonon
          double maxADC = aPulseData->GetPulseAnalysis("VarFreqRTFTWalkPhonon").GetRQVal("max")*aPulseData->GetPulseAnalysis("BasicPulseCalc").GetRQVal("norm");
  
          // filter parameters
          double butterCutoff = aPulseData->GetPulseAnalysis("VarFreqRTFTWalkPhonon").GetRQVal("VWKCutoff");
          int butterOrder = fUserData.GetIntParameter(detNum,"P_RTFT_BUTTERWORTH_ORDER");
          
          string chanName = aPulseData->GetChannelName();
          double sampleRate = -999999;
          if(chanName == "PT" || chanName == "PS1"  || chanName == "PS2") 
            sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
          else 
            sampleRate =  fDetectorConfigManager.GetSampleRate(aPulseData->GetDetectorCode());
         
      
 	 
          // ======= set in WedgeFitPhonon and calculate fit parameters ========
          tempWedgeFitPhonon.SetFitParameters(aPulse,sampleRate,butterOrder,butterCutoff,pulseRMS,maxADC,time10,time20,time60,binSStart,binAEnd,binSt0,par1Start);

         
          // ======= now fit =======
	  tempWedgeFitPhonon.DoCalc(aPulse); 
	  
	  //======= store instance of this class so RQs can be read out a little later =======
	  aPulseData->StorePulseAnalysis(tempWedgeFitPhonon);
       }

   } //end if detNum found in map

    return; 
}



//Modify this as needed to correctly call your class
void PulseEvtBuilder::DoVetoAnalysis()
{
    //some quantities need history information, so check if trigger processsing activated
    if(!fUserData.DoTriggerProcessing())
    {
       cerr <<"PulseEvtBuilder::ERROR!  Veto Analysis requires history trigger information!"  
	    <<"\nActivate DO_PROCESSING TRIGGER in your processing settings file before proceeding."
	    << endl;
       exit(1);
    }

    //loop over entire pulse collection
    for(uint pulseItr = 0; pulseItr < fVectorOfVetoPulses.size(); pulseItr++)
    {
       PulseData*     aPulseData = &fVectorOfVetoPulses[pulseItr]; 
    
       VetoAnalysis     tempVetoAnalysis;

       //optional function for setting parameters         
       tempVetoAnalysis.InitializeParameters();
       //setting functions
       
       tempVetoAnalysis.SetVetoTriggerBin(fUserData.GetIntParameter("VT_PRETRIGGER"));
       tempVetoAnalysis.SetVetoSampleTime(fUserData.GetDoubleParameter("VT_SAMPLE_TIME"));
       tempVetoAnalysis.SetSlopeThresh(fUserData.GetDoubleParameter("VT_SLOPE_THRESH"));
       tempVetoAnalysis.SetVetoBinToVolts(fUserData.GetDoubleParameter("VT_BINTO_VOLTS"));
       tempVetoAnalysis.SetPreTime(fHistoryData.GetRQVal("VTTime20"));
       tempVetoAnalysis.SetBaselineRange(fUserData.GetIntParameter("VT_BASELINEMIN"), fUserData.GetIntParameter("VT_BASELINEMAX"));
       tempVetoAnalysis.SetTriggerOffset(fUserData.GetDoubleParameter("VT_TRIGGEROFFSET"));

//       cout <<"Doing veto analysis for panel = " << aPulseData->GetDetectorNum() << endl;

       //only option for veto pulses is RawPulse (other pulses in PulseData are not filled for veto)
       tempVetoAnalysis.DoCalc(aPulseData->GetRawPulse());

       //store instance of this class so RQs can be read out a little later
       aPulseData->StorePulseAnalysis(tempVetoAnalysis);
     }

    return; 
}



void PulseEvtBuilder::DoInflectionTime(int detNum, const string& sensorType)
{ 
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {

      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);

      // ==== loop ZIP collection ======

      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData*     aPulseData = &((*zipPulseList)[pulseItr]);

         if(aPulseData->GetChannelType() != sensorType) { continue; }

         // check if analysis flag for sum of phonon pulses
         if(aPulseData->GetChannelName() == "PT" && !fUserData.DoAlgorithm(detNum,"PT","InflectionTime")) {continue; }
         if(aPulseData->GetChannelName() == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","InflectionTime")) {continue; }
         if(aPulseData->GetChannelName() == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","InflectionTime")) {continue; }

	 int minWindow = fUserData.GetIntParameter(detNum,"P_INFLECTION_WINDOW_MIN");
	 int maxWindow = fUserData.GetIntParameter(detNum,"P_INFLECTION_WINDOW_MAX");

	 InflectionTime tempInflectionTime;
	 tempInflectionTime.SetInflectionWindow(minWindow, maxWindow);
       
	 tempInflectionTime.DoCalc(aPulseData->GetBaselineSubPulse()); 
	 
	 //store instance of this class so RQs can be read out a little later
	 aPulseData->StorePulseAnalysis(tempInflectionTime);
      }

   } //end if detNum found in map

   return; 
}

void PulseEvtBuilder::DoPipeFitPhonon(int detNum, const string& sensorType)
{
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);

      //access this here instead of inside loop over zip
      // int eventWindow = fUserData.GetIntParameter("EVENTWINDOW");
      // FIXME: not used below?

      // ==== loop ZIP collection ======

      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]); 
	 int            detChan = aPulseData->GetDetectorChannel();

         if(aPulseData->GetChannelType() != sensorType) { continue; }
        
	 // check if analysis flag for sum of phonon pulses
         if(aPulseData->GetChannelName() == "PT" && !fUserData.DoAlgorithm(detNum,"PT","PipeFitPhonon")) {continue; }
         if(aPulseData->GetChannelName() == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","PipeFitPhonon")) {continue; }
         if(aPulseData->GetChannelName() == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","PipeFitPhonon")) {continue; }

         // ------- set  PipeFitPhonon parameters --------

	 PipeFitPhonon tempPipeFitPhonon;
	 
	 //don't do the fit if it looks like noise
	 if(!(int)aPulseData->GetPulseAnalysis("NoiseSelector").GetRQVal("isnoise")) 
	 {
	    //initialize 
	    tempPipeFitPhonon.SetStartWindowMin(fUserData.GetIntParameter(detNum,"P_PF_START_WINDOW_MIN"));
	    tempPipeFitPhonon.SetStartWindowMax(fUserData.GetIntParameter(detNum,"P_PF_START_WINDOW_MAX"));
	    
	    tempPipeFitPhonon.SetStartRMSMultiplier(fUserData.GetDoubleParameter(detNum,"P_PF_START_RMSMULTIPLIER"));
	    tempPipeFitPhonon.SetStartWalkMultiplier(fUserData.GetDoubleParameter(detNum,"P_PF_START_WALKMULTIPLIER"));
	    tempPipeFitPhonon.SetStartThreshCheck(fUserData.GetDoubleParameter(detNum,"P_PF_START_THRESHCHECK"));
	    tempPipeFitPhonon.SetMaxThreshCheck(fUserData.GetDoubleParameter(detNum,"P_PF_MAX_THRESHCHECK"));
	    tempPipeFitPhonon.SetLargeRMSMult(fUserData.GetDoubleParameter(detNum,"P_PF_START_LARGERMSMULTIPLIER"));
	    tempPipeFitPhonon.SetSmallRMSTest(fUserData.GetDoubleParameter(detNum,"P_PF_TESTFOR_SMALL_RMS"));
	    tempPipeFitPhonon.SetStartSmallDefault(fUserData.GetIntParameter(detNum,"P_PF_STARTTIME_DEFAULT_SMALL"));
	    
	    tempPipeFitPhonon.SetMaxADCBinStartDiff(fUserData.GetDoubleParameter(detNum,"P_PF_MAXADCPOINT_START_DIFF"));
	    tempPipeFitPhonon.SetMaxADCBinStartMult(fUserData.GetDoubleParameter(detNum,"P_PF_MAXADCPOINT_START_MULT"));
	    tempPipeFitPhonon.SetMaxADCBinStartAdd(fUserData.GetDoubleParameter(detNum,"P_PF_MAXADCPOINT_START_ADD"));
	    tempPipeFitPhonon.SetMaxADCBinAdd(fUserData.GetDoubleParameter(detNum,"P_PF_MAXADCPOINT_ADD"));
	    
	    tempPipeFitPhonon.SetMidpointDefault(fUserData.GetDoubleParameter(detNum,"P_PF_MIDPOINT_DEFAULT"));
	    tempPipeFitPhonon.SetFallFuncEnd(fUserData.GetDoubleParameter(detNum,"P_PF_FALLFUNC_END"));
	    
	    tempPipeFitPhonon.SetRiseFuncA0Default(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_A0DEFAULT"));
	    tempPipeFitPhonon.SetRiseFuncT0Default(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_T0DEFAULT"));
	    tempPipeFitPhonon.SetRiseFuncTauDefault(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_TAUDEFAULT"));
	    tempPipeFitPhonon.SetRiseFuncKappaDefault(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_KAPPADEFAULT"));
	    tempPipeFitPhonon.SetRiseFuncA1Default(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_A1DEFAULT"));
	    tempPipeFitPhonon.SetRiseFuncPulseHeightMult(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_PULSEHEIGHTMULT"));
	    tempPipeFitPhonon.SetRiseFuncStartBinDiff(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_STARTBINDIFF"));
	    tempPipeFitPhonon.SetRiseFuncTauMult(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_TAUMULT"));
	    tempPipeFitPhonon.SetRiseFuncKappaMult(fUserData.GetDoubleParameter(detNum,"P_PF_RISEFUNC_KAPPAMULT"));
	    
	    tempPipeFitPhonon.SetFallFuncAfAdd(fUserData.GetDoubleParameter(detNum,"P_PF_FALLFUNC_AF_ADD"));
	    tempPipeFitPhonon.SetFallFuncTf1Start(fUserData.GetDoubleParameter(detNum,"P_PF_FALLFUNC_TF1_START"));
	    tempPipeFitPhonon.SetFallFuncTf2Start(fUserData.GetDoubleParameter(detNum,"P_PF_FALLFUNC_TF2_START"));
	    tempPipeFitPhonon.SetFallFuncTfrStart(fUserData.GetDoubleParameter(detNum,"P_PF_FALLFUNC_TFR_START"));
	    tempPipeFitPhonon.SetFallFuncStepSizeAdd1(fUserData.GetDoubleParameter(detNum,"P_PF_FALLFUNC_STEPSIZE_ADD1"));
	    tempPipeFitPhonon.SetFallFuncStepSizeAdd2(fUserData.GetDoubleParameter(detNum,"P_PF_FALLFUNC_STEPSIZE_ADD2"));
	    
	    tempPipeFitPhonon.SetMaxTraceStartSat(fUserData.GetDoubleParameter(detNum,"P_PF_MAXTRACESTART_SATURATED"));
	    tempPipeFitPhonon.SetMaxTraceDiffSat(fUserData.GetDoubleParameter(detNum,"P_PF_MAXTRACE_PULSE_DIFF_SAT"));
	    tempPipeFitPhonon.SetPulseheightMaxSat(fUserData.GetDoubleParameter(detNum,"P_PF_PULSEHEIGHTMAX_SATURATED"));
	    tempPipeFitPhonon.SetNumberSatBins(fUserData.GetDoubleParameter(detNum,"P_PF_NUMBER_SATURATION_BINS"));
	    
	    double pulseRMS = aPulseData->GetPulseAnalysis("BasicPulseCalc").GetRQVal("std");
	    tempPipeFitPhonon.InitializeParameters(aPulseData->GetBaselineSubPulse(), fUserData.GetVectDoubleParameter(detNum,"PIPEFITPHONON_THRESH_DET"), detChan, pulseRMS);
	    
	    // --------- Do fit -------------
	    tempPipeFitPhonon.DoCalc();
	    
	    // --------- store results --------
	    aPulseData->StorePulseAnalysis(tempPipeFitPhonon);
	
       } // end if not noise
     
     }  // ======= end loop ZIP pulses  =======

   } //end if detNum found in map

   return; 
}


void PulseEvtBuilder::DoOptimalFilterPhonon(int detNum, const string& sensorType)
{
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);


      // ===== loop ZIP pulse collection =======
      
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]); 
	 string chanName   = aPulseData->GetChannelName();    
	 int    detCode    = aPulseData->GetDetectorCode();
	 
	 if(aPulseData->GetChannelType() != sensorType) { continue; } 
	 
         // check if analysis flag for sum of phonon pulses
         if(chanName == "PT" && !fUserData.DoAlgorithm(detNum,"PT","OptimalFilterPhonon")) {continue; }
         if(chanName == "PS1" && !fUserData.DoAlgorithm(detNum,"PSIDES","OptimalFilterPhonon")) {continue; }
         if(chanName == "PS2" && !fUserData.DoAlgorithm(detNum,"PSIDES","OptimalFilterPhonon")) {continue; }

	 // ------- getting templates for OF ---------

	 vector<TComplex> templateConjNoiseFFT = fFilterData.GetTemplateConjNoiseFFT(detNum,chanName);
	 vector<TComplex> templateFFT = fFilterData.GetTemplateFFT(detNum,chanName);

	
         
         // ------- getting normalization for OF ---------

	 vector<double> noiseFFTsq  = fFilterData.GetNoiseFFTsq(detNum,chanName);
	 double normFFT = fFilterData.GetNormFFT(detNum,chanName);
	 double sigToNoiseSq = fFilterData.GetSigToNoiseSq(detNum,chanName);


         // ------- calculate optimal filter window --------
         

          double sampleRate = -999999.; 
          double preTrigger = -999999.; 
          double traceLength = -999999.;

          if(chanName == "PT" || chanName == "PS1"  || chanName == "PS2") {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
              traceLength =  fDetectorConfigManager.GetTraceLength(detNum,sensorType);
          } else {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detCode);
              traceLength  =  fDetectorConfigManager.GetTraceLength(detCode);
          }

         int postTriggerBins = (int)traceLength - (int)floor(sampleRate * preTrigger);
  
         int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MIN")));
         int windowMax  = (int) floor(sampleRate * (preTrigger + fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MAX")));

	 int win1 = windowMax - (int) floor(sampleRate * preTrigger);
	 int win2 = windowMin + postTriggerBins + 1;
	    


         // cutoff frequency
         double cutoffFreq = fUserData.GetDoubleParameter(detNum,"P_CHISQ_CUTOFF");

         // ------- set OptimalFilterPhonon  parameters ---------

         OptimalFilterPhonon tempOptimalFilterPhonon;
	 
	 tempOptimalFilterPhonon.LoadTemplates(templateFFT, templateConjNoiseFFT); 
	 tempOptimalFilterPhonon.LoadNormalizations(normFFT, sigToNoiseSq, noiseFFTsq); 
	 tempOptimalFilterPhonon.LoadCutoffFreq(cutoffFreq);
 
	 //set timing parameters
	 tempOptimalFilterPhonon.SetSampleTime(1.0/sampleRate);
	 tempOptimalFilterPhonon.SetWindows(win1,win2);

         // -------  do Optimal Filter ------------	 

	 //Get Pulse and normalize if ISR file is being read;
	 vector<double> aBSNPulse;	 
	 aBSNPulse = aPulseData->GetBaselineSubNormPulse();  //when ISR is not read, norm = 1    

	 tempOptimalFilterPhonon.DoCalc(aBSNPulse);
	 

         // -------  store  information -----------
      
	 //store instance of this class so RQs can be read out a little later
	 aPulseData->StorePulseAnalysis(tempOptimalFilterPhonon);
      
     }  // ======= end loop ZIP pulses  =======

   } //end if detNum found in map

   return; 
}




void PulseEvtBuilder::DoOptimalFilterPhononGlitch1(int detNum, const string& sensorType)
{
   string glitchChanName = "PTglitch1";

   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);


      // ===== loop ZIP pulse collection =======
      
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]); 
	 string chanName   = aPulseData->GetChannelName();    
         
	 
	 if(aPulseData->GetChannelType() != sensorType) { continue; } 

       
         //analysis for sum of phonon pulses only
         if(aPulseData->GetChannelName() != "PT" || !fUserData.DoAlgorithm(detNum,"PT","OptimalFilterPhononGlitch1")) { continue; }

        
	 // ------- getting templates for OF ---------
      
	 vector<TComplex> templateConjNoiseFFT = fFilterData.GetTemplateConjNoiseFFT(detNum,glitchChanName);
	 vector<TComplex> templateFFT = fFilterData.GetTemplateFFT(detNum,glitchChanName);

	
         
         // ------- getting normalization for OF ---------

	 vector<double> noiseFFTsq  = fFilterData.GetNoiseFFTsq(detNum,glitchChanName);
	 double normFFT = fFilterData.GetNormFFT(detNum,glitchChanName);
	 double sigToNoiseSq = fFilterData.GetSigToNoiseSq(detNum,glitchChanName);


         // ------- calculate optimal filter window --------
         // use window regular OF 

         double sampleRate = fDetectorConfigManager.GetSampleRate(detNum,sensorType);
         double preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
         double traceLength = fDetectorConfigManager.GetTraceLength(detNum,sensorType);
     
         int postTriggerBins = (int)traceLength - (int)floor(sampleRate * preTrigger);
  
         int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MIN")));
         int windowMax  = (int) floor(sampleRate * (preTrigger + fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MAX")));

	 int win1 = windowMax - (int) floor(sampleRate * preTrigger);
	 int win2 = windowMin + postTriggerBins + 1;
	    

         // ------- set OptimalFilterPhonon  parameters ---------
         OptimalFilterPhonon tempOptimalFilterPhonon("OptimalFilterPhononGlitch1");

         tempOptimalFilterPhonon.LoadTemplates(templateFFT, templateConjNoiseFFT); 
	 tempOptimalFilterPhonon.LoadNormalizations(normFFT, sigToNoiseSq, noiseFFTsq); 
	 
	 //set timing parameters
	 tempOptimalFilterPhonon.SetSampleTime(1.0/sampleRate);
	 tempOptimalFilterPhonon.SetWindows(win1,win2);

         // -------  do Optimal Filter ------------	 

	 //Get Pulse and normalize if ISR file is being read;
	 vector<double> aBSNPulse;	 
	 aBSNPulse = aPulseData->GetBaselineSubNormPulse();  //when ISR is not read, norm = 1    

	 tempOptimalFilterPhonon.DoCalc(aBSNPulse);
	 

         // -------  store  information -----------
      
	 //store instance of this class so RQs can be read out a little later
	 aPulseData->StorePulseAnalysis(tempOptimalFilterPhonon);
      
     }  // ======= end loop ZIP pulses  =======

   } //end if detNum found in map

   return; 
}




void PulseEvtBuilder::DoOptimalFilterPhononLFnoise1(int detNum, const string& sensorType)
{
   string lfnoiseChanName = "PTlfnoise1";

   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);


      // ===== loop ZIP pulse collection =======
      
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]); 
	 string chanName   = aPulseData->GetChannelName();    
         
	 
	 if(aPulseData->GetChannelType() != sensorType) { continue; } 

       
         //analysis for sum of phonon pulses only
         if(aPulseData->GetChannelName() != "PT" || !fUserData.DoAlgorithm(detNum,"PT","OptimalFilterPhononLFnoise1")) { continue; }

        
	 // ------- getting templates for OF ---------
      
	 vector<TComplex> templateConjNoiseFFT = fFilterData.GetTemplateConjNoiseFFT(detNum,lfnoiseChanName);
	 vector<TComplex> templateFFT = fFilterData.GetTemplateFFT(detNum,lfnoiseChanName);

	
         
         // ------- getting normalization for OF ---------

	 vector<double> noiseFFTsq  = fFilterData.GetNoiseFFTsq(detNum,lfnoiseChanName);
	 double normFFT = fFilterData.GetNormFFT(detNum,lfnoiseChanName);
	 double sigToNoiseSq = fFilterData.GetSigToNoiseSq(detNum,lfnoiseChanName);


         // ------- calculate optimal filter window --------
         // use window regular OF 

         double sampleRate = fDetectorConfigManager.GetSampleRate(detNum,sensorType);
         double preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
         double traceLength = fDetectorConfigManager.GetTraceLength(detNum,sensorType);
     
         int postTriggerBins = (int)traceLength - (int)floor(sampleRate * preTrigger);
  
         int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MIN")));
         int windowMax  = (int) floor(sampleRate * (preTrigger + fUserData.GetDoubleParameter(detNum,"P_PEAK_WINDOW_MAX")));

	 int win1 = windowMax - (int) floor(sampleRate * preTrigger);
	 int win2 = windowMin + postTriggerBins + 1;
	    

         // ------- set OptimalFilterPhonon  parameters ---------
         OptimalFilterPhonon tempOptimalFilterPhonon("OptimalFilterPhononLFnoise1");

         tempOptimalFilterPhonon.LoadTemplates(templateFFT, templateConjNoiseFFT); 
	 tempOptimalFilterPhonon.LoadNormalizations(normFFT, sigToNoiseSq, noiseFFTsq); 
	 
	 //set timing parameters
	 tempOptimalFilterPhonon.SetSampleTime(1.0/sampleRate);
	 tempOptimalFilterPhonon.SetWindows(win1,win2);

         // -------  do Optimal Filter ------------	 

	 //Get Pulse and normalize if ISR file is being read;
	 vector<double> aBSNPulse;	 
	 aBSNPulse = aPulseData->GetBaselineSubNormPulse();  //when ISR is not read, norm = 1    

	 tempOptimalFilterPhonon.DoCalc(aBSNPulse);
	 

         // -------  store  information -----------
      
	 //store instance of this class so RQs can be read out a little later
	 aPulseData->StorePulseAnalysis(tempOptimalFilterPhonon);
      
     }  // ======= end loop ZIP pulses  =======

   } //end if detNum found in map

   return; 
}









void PulseEvtBuilder::DoOptimalFilterCharge(int detNum)
{

   //pulses in the pulse collection are expected to be ordered by the raw data reader 
   //according to zip and channel number.

   //retrieve the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);


      // ===== loop ZIP pulse collection =======

      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]); 
	 string chanName = aPulseData->GetChannelName();
	 string sensorType = "charge"; 
	 int    detCode    = aPulseData->GetDetectorCode();

	 //we want only charge pulses
	 if(aPulseData->IsPhononPulse()) { continue; } 
	 
	 // ------- calculate optimal filter window --------

	 OptimalFilterCharge tempOptimalFilterCharge;

         
         double sampleRate = -999999.; 
         double preTrigger = -999999.; 
         double traceLength = -999999.;

         if(chanName == "QT") {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detNum,sensorType);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detNum,sensorType);
              traceLength =  fDetectorConfigManager.GetTraceLength(detNum,sensorType);
          } else {
              sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
              preTrigger =  fDetectorConfigManager.GetTriggerTime(detCode);
              traceLength  =  fDetectorConfigManager.GetTraceLength(detCode);
          }
	 
   
	 int postTriggerBins = (int)traceLength - (int) floor(sampleRate * preTrigger);

         int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"Q_PEAK_WINDOW_MIN")));
         int windowMax  = (int) floor(sampleRate * (preTrigger + fUserData.GetDoubleParameter(detNum,"Q_PEAK_WINDOW_MAX")));

	 int win1 = windowMax - (int) floor(sampleRate * preTrigger);
	 int win2 = windowMin + postTriggerBins + 1;
	 
	 // ------ set  OptimalFilterCharge parameters ---------
              	    
	 //load templates into OF
	 vector<TComplex> templateConjNoiseFFT
	    = fFilterData.GetTemplateConjNoiseFFT(detNum,chanName);
	 
	 vector<TComplex> templateFFT
	    = fFilterData.GetTemplateFFT(detNum,chanName);
	 
	 tempOptimalFilterCharge.LoadTemplates(templateFFT, templateConjNoiseFFT); 

	    
	 //load normalizations into OF
	 double normFFT = fFilterData.GetNormFFT(detNum,chanName);
	 
	 double sigToNoiseSq = fFilterData.GetSigToNoiseSq(detNum,chanName);
	 vector<double> noiseFFTsq 
	    = fFilterData.GetNoiseFFTsq(detNum,chanName);
	 double templateMax = fFilterData.GetTemplateMax(detNum,chanName);
	 
	 tempOptimalFilterCharge.LoadNormalizations(normFFT, sigToNoiseSq, noiseFFTsq, templateMax); 
	       
	    	    
	 //set timing parameters
	 tempOptimalFilterCharge.SetSampleTime(1.0/sampleRate);  	    
	 tempOptimalFilterCharge.SetWindows(win1, win2); 
	 
	 //Get Pulses and normalize, store bias' if ISR file is being read;
	 double biasQI = -999999.;
	 	 
	 if(fReadIsr)
	 {
	    biasQI = fIsrData.GetBias(detNum,chanName, (double) fAdminData.GetEventTime());
	    tempOptimalFilterCharge.SetBiasVoltages(biasQI); //this is only done if isr is read!
	 }
	    
	 // --------- Do Optimal Filter ---------------
	 
	 tempOptimalFilterCharge.DoCalc(aPulseData->GetBaselineSubNormPulse()); //when ISR is not read, norm = 1     
	 
	 
	 // ---------- store results -------------------
         
	 //storing copies of this class so RQs can be read out a little later	    
	 aPulseData->StorePulseAnalysis(tempOptimalFilterCharge);
	 
	 
      }  // ======= end loop ZIP pulses  =======
   
   } //end if detNum found in map

   return; 
}


void PulseEvtBuilder::DoOptimalFilterChargeX(int detNum, const string& side)
{ 
   
   // channels names
   string chanNameQI = "QI";
   string chanNameQO = "QO";
   string chanNameQIX = "QIX";
   string chanNameQOX = "QOX";

   // index QI theshold vector
   int indexQI = 0;
   if (side == "S2") indexQI=2;
 
   
   // check argument

   if (side == "S1" || side == "S2") {

     chanNameQI = "QI"+side;
     chanNameQO = "QO"+side;
     chanNameQIX = "QI"+side +"X";
     chanNameQOX = "QO"+side +"X";


   } else if (!side.empty()) {
      cout <<"\nERROR! PulseEvtBuilder::DoOptimalFilterChargeX:  Argument side must be 'S1', 'S2', or empty string"
		 << endl;
      exit(1);
   }
  

   //pulses in the pulse collection are expected to be ordered by the raw data reader 
   //according to zip and channel number.

   //retrieve the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);

      PulseData* aPulseDataQI = NULL;
      PulseData* aPulseDataQO = NULL;


      // ===== loop ZIP pulse collection =======

      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aTempPulseData = &((*zipPulseList)[pulseItr]); 
	 
	 if(aTempPulseData->GetDetectorType() != BatRootTypes::kZIPDetType &&
	    aTempPulseData->GetDetectorType() != BatRootTypes::kMZIPDetType &&
	    aTempPulseData->GetDetectorType() != BatRootTypes::kiZIPSoudan  &&
            aTempPulseData->GetDetectorType() != BatRootTypes::kCDMSliteSoudanI && 
            aTempPulseData->GetDetectorType() != BatRootTypes::kCDMSliteSoudanII)
	 {
	    cout <<"\nERROR! PulseEvtBuilder::DoOptimalFilterChargeX  OptimalFilterChargeX calculation not implemented for this detector type."
		 <<"  Deactivate DO_CHARGE_ALGORITHM: OptimalFilterChargeX in the processing settings until properly implemented."
		 << endl;
	    exit(1);
	 }

	 //we want only charge pulses
	 if(aTempPulseData->IsPhononPulse()) continue; 


         //we want only charge pulse of the same side
         string channelName = aTempPulseData->GetChannelName(); 
         if ((side == "S1" || side == "S2") &&  channelName.find(side)==string::npos) continue;

         // ------ find QI/QO -----------
      	 if(channelName == chanNameQI)
	    aPulseDataQI = aTempPulseData;
	 
	 if(channelName == chanNameQO)
	    aPulseDataQO = aTempPulseData;
	 
	 int detNumQI = (aPulseDataQI == NULL ? -999999 : aPulseDataQI->GetDetectorNum());
	 int detNumQO = (aPulseDataQO == NULL ? -999999 : aPulseDataQO->GetDetectorNum());
	 
       
	 //found a pair! now we can do the optimal filter if no saturated bins were found in the pulses
	 if(detNumQI == detNumQO && detNumQI != -999999 && 
	    (aPulseDataQI->GetPulseAnalysis("BasicPulseCalc").GetRQVal("sat") == 0 ) && 
	    (aPulseDataQO->GetPulseAnalysis("BasicPulseCalc").GetRQVal("sat") == 0 ) )
	 {

	    string sensorType = "charge"; 
	    int    detCode    = aPulseDataQI->GetDetectorCode(); //FIXME - temp until right functions are added to DetConfigManager

            // ------- calculate optimal filter window --------

	    OptimalFilterChargeX tempOptimalFilterChargeX;
            
            double sampleRate = fDetectorConfigManager.GetSampleRate(detCode);
            double preTrigger =  fDetectorConfigManager.GetTriggerTime(detCode);
            double traceLength = fDetectorConfigManager.GetTraceLength(detCode);
	 
	    int postTriggerBins = (int)traceLength - (int) floor(sampleRate * preTrigger);

	    int windowMin  = (int) floor(sampleRate * (preTrigger - fUserData.GetDoubleParameter(detNum,"Q_PEAK_WINDOW_MIN")));
	    int windowMax  = (int) floor(sampleRate * (preTrigger + fUserData.GetDoubleParameter(detNum,"Q_PEAK_WINDOW_MAX")));
 
            int qxwin1 = windowMax - (int) floor(sampleRate * preTrigger);
	    int qxwin2 = windowMin + postTriggerBins + 1;
	      

            // ------ set  OptimalFilterChargeX parameters ---------
              	    
	    //load templates into OF
            string channels[4] = {chanNameQI,chanNameQO,chanNameQIX,chanNameQOX};
            string channelsType[4] = {"QI","QO","QIX","QOX"};

	    for(int chanItr = 0; chanItr < 4; chanItr++)
	    {

	       vector<TComplex> templateConjNoiseFFT
		  = fFilterData.GetTemplateConjNoiseFFT(detNum,channels[chanItr]);
	       
	       vector<TComplex> templateFFT
		  = fFilterData.GetTemplateFFT(detNum,channels[chanItr]);

	       tempOptimalFilterChargeX.LoadTemplates(templateFFT, templateConjNoiseFFT, channelsType[chanItr]); 
	       
	       //load normalizations into OF
	       if(channels[chanItr] == chanNameQI || channels[chanItr] == chanNameQO)
	       {
		  double normFFT = fFilterData.GetNormFFT(detNum,channels[chanItr]);

		  double sigToNoiseSq = fFilterData.GetSigToNoiseSq(detNum,channels[chanItr]);
		  vector<double> noiseFFTsq 
		     = fFilterData.GetNoiseFFTsq(detNum,channels[chanItr]);
		  double templateMax = fFilterData.GetTemplateMax(detNum,channels[chanItr]);
		  
		  tempOptimalFilterChargeX.LoadNormalizations(normFFT, sigToNoiseSq, noiseFFTsq, templateMax, channelsType[chanItr]); 
	       }
	       
	    }
	    
	    //load QInverse
	    tempOptimalFilterChargeX.LoadQInverse(fFilterData.GetQXtalkInverseMatrix(detNum,side));
	    
	    //set timing parameters
	    tempOptimalFilterChargeX.SetSampleTime(1.0/sampleRate);  	    
	    tempOptimalFilterChargeX.SetQxWindows(qxwin1, qxwin2); 
	    
	    //load the chisq thresholds
	    vector<double> minQIQO = fUserData.GetVectDoubleParameter(detNum, "Q_CHISQ_THRESH");
	    double minQI = minQIQO[indexQI];
	    double minQO = minQIQO[indexQI+1];
	    tempOptimalFilterChargeX.LoadChisqThresholds(minQI, minQO);

	    //is it a random trigger?
	    bool isRandom = (fRawReader.GetEventCategory() == 0x1 ? true : false);
	    tempOptimalFilterChargeX.IsRandom(isRandom);

	    //do the delay interpolation?
	    tempOptimalFilterChargeX.SetDelayInterpolateFlag(fUserData.GetIntParameter(detNum, "Q_DELAY_INTERPOLATE"));

	    //Get Pulses and normalize, store bias' if ISR file is being read;
	    vector<double> aBSNPulseQI;
	    vector<double> aBSNPulseQO;
	    double biasQI = -999999.;
	    double biasQO = -999999.;
	    
	    aBSNPulseQI = aPulseDataQI->GetBaselineSubNormPulse();	//when ISR is not read, norm = 1     
	    aBSNPulseQO = aPulseDataQO->GetBaselineSubNormPulse();	//when ISR is not read, norm = 1     
	    
	    if(fReadIsr)
	    {
	       biasQI = fIsrData.GetBias(detNum,chanNameQI, (double) fAdminData.GetEventTime());
	       biasQO = fIsrData.GetBias(detNum,chanNameQO, (double) fAdminData.GetEventTime());
	       
	       tempOptimalFilterChargeX.SetBiasVoltages(biasQI, biasQO); //this is only done if isr is read!
	    }
	    
	    // --------- Do Optimal Filter ---------------

	    tempOptimalFilterChargeX.DoCalc(aBSNPulseQI, aBSNPulseQO);
	    

            // ---------- store results -------------------
           
	    //storing copies of this class so RQs can be read out a little later
	    //Note this is a little different than the other analysis classes because there is one fit for two pulses
	    tempOptimalFilterChargeX.StoreAs("QI");
	    aPulseDataQI->StorePulseAnalysis(tempOptimalFilterChargeX);
	    
	    tempOptimalFilterChargeX.StoreAs("QO");
	    aPulseDataQO->StorePulseAnalysis(tempOptimalFilterChargeX);
	    
	 } //endif we found a pair
	 
      }  // ======= end loop ZIP pulses  =======
      
   } //end if detNum found in map

   return; 
}



void PulseEvtBuilder::DoF5ChargeX(int detNum, const string& side)
{
   //The F5Charge class is structurally modeled after the OptimalFilterChargeX class

   // channels names
   string chanNameQI = "QI";
   string chanNameQO = "QO";
   string chanNameQIX = "QIX";
   string chanNameQOX = "QOX";

   // index QI theshold vector
   int indexQI = 0;
   if (side == "S2") indexQI=2;
 
   
   // check argument
   if (side == "S1" || side == "S2") {

     chanNameQI = "QI"+side;
     chanNameQO = "QO"+side;
     chanNameQIX = "QI"+side +"X";
     chanNameQOX = "QO"+side +"X";

   } else if (!side.empty()) {
      cout <<"\nERROR! PulseEvtBuilder::DoF5ChargeX:  Argument side must be 'S1', 'S2', or empty string"
		 << endl;
      exit(1);
   }
  

   
   //pulses in the pulse collection are expected to be ordered by the raw data reader 
   //according to zip and channel number.
   
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      //Get the pulse list for this zip
      zipPulseList = &(mapItr->second);

      PulseData* aPulseDataQI = NULL;
      PulseData* aPulseDataQO = NULL;

      // ========= loop over ZIP pulse collection =========

      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aTempPulseData = &((*zipPulseList)[pulseItr]); 

         if(aTempPulseData->GetDetectorType() != BatRootTypes::kZIPDetType &&
	    aTempPulseData->GetDetectorType() != BatRootTypes::kMZIPDetType &&
	    aTempPulseData->GetDetectorType() != BatRootTypes::kiZIPSoudan &&
            aTempPulseData->GetDetectorType() != BatRootTypes::kCDMSliteSoudanI && 
            aTempPulseData->GetDetectorType() != BatRootTypes::kCDMSliteSoudanII )
	 {
	   
	    cout <<"\nERROR! PulseEvtBuilder::DoF5ChargeX  F5ChargeX calculation not implemented for this detector type."
		 <<"  Deactivate DO_CHARGE_ALGORITHM: F5ChargeX in the processing settings until properly implemented."
		 << endl;
	    exit(1);
	 }
	 
	 //looking for only charge pulses
	 if(aTempPulseData->IsPhononPulse()) { continue; }
	 

         //we want only charge pulse of the same side
         string channelName = aTempPulseData->GetChannelName(); 
         if ((side == "S1" || side == "S2") &&  channelName.find(side)==string::npos) continue;


         // ------ find QI/QO -----------

         if(channelName == chanNameQI)
	    aPulseDataQI = aTempPulseData;
	 
	 if(channelName == chanNameQO)
	    aPulseDataQO = aTempPulseData;
	 
	 int detNumQI = (aPulseDataQI == NULL ? -999999 : aPulseDataQI->GetDetectorNum());
	 int detNumQO = (aPulseDataQO == NULL ? -999999 : aPulseDataQO->GetDetectorNum());
	 
	 
	 //found a pair! now we can do the F5
	 if(detNumQI == detNumQO && detNumQI != -999999)
	 {
	   
	   string sensorType = "charge"; 
	   int    detCode    = aPulseDataQI->GetDetectorCode(); //FIXME - temp until right functions are added to DetConfigManager
	    
           // ------- set F5ChargeX parameters ---------

           F5ChargeX tempF5ChargeX;
	 
	   //load templates (and template maxima) into OF
	   vector<double> templateQI = fFilterData.GetTemplateTime(detNum,chanNameQI);
	   vector<double> templateQIX = fFilterData.GetTemplateTime(detNum,chanNameQIX);
	   vector<double> templateQO = fFilterData.GetTemplateTime(detNum,chanNameQO);
	   vector<double> templateQOX = fFilterData.GetTemplateTime(detNum,chanNameQOX);
	    
	   //these values are named QIEnergy and QOEnergy in the darkpipe version of F5.   
	   double templateMaxQI = fFilterData.GetTemplateMax(detNum,chanNameQI);
	   double templateMaxQO = fFilterData.GetTemplateMax(detNum,chanNameQO);

	   tempF5ChargeX.LoadTemplates(templateQI, templateQIX, templateQO, templateQOX,
					templateMaxQI, templateMaxQO); 
	   
	   //set timing parameters and set fit window
           double sampleRate =  fDetectorConfigManager.GetSampleRate(detCode);
                          
           double gainQI = fDetectorConfigManager.GetDriverGain(aPulseDataQI->GetDetectorCode(), fAdminData.GetEventTime()); 
           double gainQO = fDetectorConfigManager.GetDriverGain(aPulseDataQO->GetDetectorCode(), fAdminData.GetEventTime());          
	   double digitizerbins = fUserData.GetDoubleParameter(detNum,"Q_DigitizerBinsPerVolt");	
	   double Qgain1 =   fUserData.GetDoubleParameter(detNum,"Q_Gain1");


	   tempF5ChargeX.SetSampleRate(sampleRate); 
	  

	   tempF5ChargeX.SetGoodQStart(fUserData.GetIntParameter(detNum,"Q_START_GOOD_CHARGE"));
	   tempF5ChargeX.SetGoodQEnd(fUserData.GetIntParameter(detNum,"Q_END_GOOD_CHARGE"));
	   tempF5ChargeX.SetNoiseQI(fUserData.GetDoubleParameter(detNum,"Q_F5_NOISE"));
	   tempF5ChargeX.SetNoiseQO(fUserData.GetDoubleParameter(detNum,"Q_F5_NOISE"));
	   tempF5ChargeX.SetTemplateStart(fUserData.GetIntParameter(detNum,"Q_TEMPLATE_START"));
           tempF5ChargeX.SetSaturationValue(fUserData.GetIntParameter(detNum,"Q_SATURATION")); 

           tempF5ChargeX.SetQIStd(aPulseDataQI->GetPulseAnalysis("BasicPulseCalc").GetRQVal("std"));
	   tempF5ChargeX.SetQOStd(aPulseDataQO->GetPulseAnalysis("BasicPulseCalc").GetRQVal("std"));
	   tempF5ChargeX.SetQIBaseline(aPulseDataQI->GetPulseAnalysis("BasicPulseCalc").GetRQVal("bs"));
	   tempF5ChargeX.SetQOBaseline(aPulseDataQO->GetPulseAnalysis("BasicPulseCalc").GetRQVal("bs"));


	    //normalize the pulses if ISR is being read
	   vector<double> aBSNPulseQI;
	   vector<double> aBSNPulseQO;
	   vector<double> aRawPulseQI;
	   vector<double> aRawPulseQO;
	   
        
           aBSNPulseQI = aPulseDataQI->GetBaselineSubNormPulse();	//when ISR is not read, norm = 1     
	   aBSNPulseQO = aPulseDataQO->GetBaselineSubNormPulse();	//when ISR is not read, norm = 1     
	    


	   //Get raw pulse for saturation dependent values
	   aRawPulseQI = aPulseDataQI->GetRawPulse();
	   aRawPulseQO = aPulseDataQO->GetRawPulse();

           //normalized pulses not BS subtracted
	   tempF5ChargeX.SetGainQI(gainQI*Qgain1*digitizerbins);
	   tempF5ChargeX.SetGainQO(gainQO*Qgain1*digitizerbins); 
           
	   //in case pulse is not saturating get OF delay
	   if((aPulseDataQI->GetPulseAnalysis("BasicPulseCalc").GetRQVal("sat") == 0 ) && 
	     (aPulseDataQO->GetPulseAnalysis("BasicPulseCalc").GetRQVal("sat") == 0 ) )
	     {
               // check analysis
               string analysisName;
               if (aPulseDataQI->HasPulseAnalysis("OptimalFilterChargeX"))
                    analysisName = "OptimalFilterChargeX";
               if (aPulseDataQI->HasPulseAnalysis("OptimalFilterCharge2X2"))
                    analysisName = "OptimalFilterCharge2X2";
               
               if (!analysisName.empty()) {
	         TCDMSAnalysis tempOptimalFilter = aPulseDataQI->GetPulseAnalysis(analysisName);
	         tempF5ChargeX.SetOFDelay(tempOptimalFilter.GetRQVal("OFdelay")); 
               }
	     }
	   tempF5ChargeX.DoCalc(aRawPulseQI, aRawPulseQO);
	 
           	    
           // ----------- store results -------------

	   //For debugging only!  storing the fake QI and QO pulses in the testVector slot of PulseData so that it can be accessed later
	   //aPulseDataQI->fTestPulseVector = tempF5ChargeX.fFakePulseQI;
	   //aPulseDataQO->fTestPulseVector = tempF5ChargeX.fFakePulseQO;
	    
	   //storing copies of this class so RQs can be read out a little later
	   //Note this is a little different than the other analysis classes because there is one fit for two pulses
	   tempF5ChargeX.StoreAs("QI");
	   aPulseDataQI->StorePulseAnalysis(tempF5ChargeX);
	   
	   tempF5ChargeX.StoreAs("QO");
	   aPulseDataQO->StorePulseAnalysis(tempF5ChargeX);
	   
	 } //endif we found a pair
	 
      } //  ====== end loop over ZIP pulse ========

      
   } //end if detNum found in map
   
   return; 
}


// GPIB timing analysis

void PulseEvtBuilder::DoGpibTimingCalc()
{
  // GPIB (Flash Times)
  fGpibData.DoCalc((double) fAdminData.GetEventTime());
  return;
}


void PulseEvtBuilder::DoIsrTimingCalc()
{
 
  // ISR (LastISRTime)
  fIsrData.DoCalcLastIsrTime((double) fAdminData.GetEventTime());

  return;
}


//  DMM  analysis
void PulseEvtBuilder::DoDmmCalc(int detNum)
{
  fDmmData.DoCalc(detNum, (double) fAdminData.GetEventTime());
  return;
}


//  ISR  analysis
//  This is deprecated as of 2011 (starting w/ Soudan R132) - FIXME - remove altogether?
void PulseEvtBuilder::DoIsrCalc(int detNum)
{
  fIsrData.DoCalcBias(detNum, (double) fAdminData.GetEventTime());
  return;
}

// Database analysis
void PulseEvtBuilder::DoDatabaseEventCalc()
{
  fDatabaseManager.StoreEventRQs(fAdminData.GetEventTime());
}

/* Commented this out because of some compilation issues. [AJA]
void PulseEvtBuilder::DoDatabasePulseCalc(int detNum)
{
  time_t evTime = fAdminData.GetEventTime();
  
  map< int, vector<PulseData> >::iterator mapItr = fMapOfZipPulses.find(detNum);
  if( mapItr != fMapOfZipPulses.end() ){
    vector<PulseData>& pulses = mapItr->second;
    for(vector<PulseData>::iterator pulse = pulses.begin(); pulse!=pulses.end();
	++pulse){
      uint32_t detCode = pulse->GetDetectorCode();


      // If needed, modify detector code (case raw data has been modified)
      if (fUserData.DoModifyRawData()) 
        {
          int detNum =  ChannelMapHelper::GetDetNumFromCode(detCode);

          map<int, string> modificationMap = fUserData.GetRawDataModificationMap();          
          if (modificationMap.find(detNum)!=modificationMap.end())
            { 
              string modificationType = modificationMap.find(detNum)->second;
              
              if (modificationType.compare("CDMSliteSoudanI") == 0) {

                 uint32_t  detCodeBaseOld = (uint32_t) ChannelMapHelper::CalcDetCodeBase(11,detNum);
                 uint32_t  detCodeBaseNew = (uint32_t) ChannelMapHelper::CalcDetCodeBase(21,detNum);
       
                 if (detCode==(detCodeBaseNew + 2)) detCode=detCodeBaseOld + 8;
                 if (detCode==(detCodeBaseNew + 3)) detCode=detCodeBaseOld + 3;
                 if (detCode==(detCodeBaseNew + 4)) detCode=detCodeBaseOld + 10;	
                 if (detCode==(detCodeBaseNew + 5)) detCode=detCodeBaseOld + 5;
              }
            }
          } 
 
      // store database RQs
      std::string sensorType = pulse->GetChannelType();
      fDatabaseManager.StoreDetectorRQs(evTime, detCode, sensorType);
      //construct a temporaty dummy TCDMSAnalysis to hold the RQ list
      GenericRQStorage rqstore("DatabaseInfo",
			       fDatabaseManager.GetDetectorRQList());
      pulse->StorePulseAnalysis(rqstore);
    }
  }
  }*/


// ================== Read-only access to Pulse Collections  ========================

//This function gives read-only access to the pulse values!
void PulseEvtBuilder::FillVetoPulseCollection(vector<PulseData>& pulseCollection) const
{
   pulseCollection = fVectorOfVetoPulses;
   return;
}

//This function gives read-only access to the pulse values!
void PulseEvtBuilder::FillSingleZipPulseCollection(vector<PulseData>& pulseCollection,
						const int& detNum) const
{

   vector<PulseData> tempCollection;

   //retrive the vector of pulses for this zip
   map< int, vector<PulseData> >::const_iterator mapItr = fMapOfZipPulses.find(detNum);
   if(mapItr != fMapOfZipPulses.end())
   {
      tempCollection = mapItr->second;
   }  
 
   pulseCollection = tempCollection;

   return;
}

// ====================== Utility Functions  =========================

//defunct, not being used right now
bool PulseEvtBuilder::IsChosenType(const PulseData& aPulseData, const string& whichPulses) const
{
   bool isChosen = false;

   //Next, checking channel type 
   if(whichPulses != "chooseall" && whichPulses != "chooseveto"  &&
      whichPulses != "choosezip" && whichPulses != "choosephonon"  &&
      whichPulses != "choosecharge" && whichPulses != "none")
   { 
     cout <<"PulseEvtBuilder::WARNING!  Unknown pulse collection choice: " << whichPulses
	  <<" no pulse selected! " << endl;
     return isChosen;
   }

   if(whichPulses == "chooseall")
   {
      isChosen = true;
      return isChosen;
   }

   if(whichPulses == "chooseveto")
   {
      if(aPulseData.IsVetoPulse()) isChosen = true;
      return isChosen;
   }

   if(whichPulses == "choosezip")
   {
      if(aPulseData.IsZipPulse()) isChosen = true;
      return isChosen;
   }

   if(whichPulses == "choosephonon")
   {
      if(aPulseData.IsPhononPulse()) isChosen = true;
      return isChosen;
   }

   if(whichPulses == "choosecharge")
   {
      if(aPulseData.IsChargePulse()) isChosen = true;
      return isChosen;
   }

      return false;
}

