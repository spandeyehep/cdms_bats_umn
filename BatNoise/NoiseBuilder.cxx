///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: NoiseBuilder
//Authors: L. Hsu, M. Kos, B. Serfass
//Description:  This class performs the analysis of traces for noise generation.  It controls the output generation of the noise files.
//It also interfaces the user commands and raw data reading.
//
//File Import By: L. Hsu
//Creation Date: Nov. 17, 2008
//
//Modifications:
//   20111118  M. Kelsey / B. Serfass -- Use CDMSBATSDIR/BatNoise instead of BATROOTDIR/noise
//   20141210  B. Serfass -- Add 2D OF template calculationb
//   20171229  N. Mast  --  Add baseline slope noise selection
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "zlib.h"
#include "time.h"
#include <iomanip> //for debugging
#include <regex.h>

#include "TComplex.h"
#include "TMatrixD.h"
#include "TTree.h"
#include "TString.h"
#include "TGraphErrors.h"


#include "ChannelMapHelper.h"
#include "PulseTools.h"
#include "NoiseBuilder.h"
#include "OptimalFilterPhonon.h"

using namespace std;

////////////////////////////////////////////////////////

//default constructor
NoiseBuilder::NoiseBuilder(UserDataManager& myUserData, DetectorConfigManager& myDetConfigManager,
			   TemplateDataManager& myTemplateData) :
   fUserData(myUserData),
   fTemplateData(myTemplateData),
   fDetConfigManager(myDetConfigManager)
{
  //cout <<"Constructing NoiseBuilder" << endl;

   //Configure the NoiseBuilder
   ConfigureNoiseBuilder();

   //Configure Correlation Data
   ConfigureCorrelationData();

   //Register the data classes so that they will be read
   fRawReader.RegisterAdminData(&fAdminData);

   //Pulses handled a little differently
   fRawReader.RegisterZipPulseMap(&fMapOfZipPulses); 
   
}

NoiseBuilder::~NoiseBuilder()
{
  //cout <<"Hello from NoiseBuilder destructor!" << endl; 
}

//This function builds a map of NoiseData objects and
//loads the appropriate templates for each NoiseData object
void NoiseBuilder::ConfigureNoiseBuilder()
{
  //NoiseData are stored in ascending order according to det code.  There is one NoiseData object per detector channel
  //There are also NoiseData objects for each of the "special" pulses: PT, QT, QIX, QOX, and for iZIP  PS1,PS2
  //The NoiseData objects are arranged in a map analagous to the way the PulseData objects are stored
  //The order is: physical channels (QI, QO, PA, ...), then summed channels (QT, PT, PS1, PS2), then cross talk channels (optional)
 
  // Retrieve the detector map and configure NoiseBuilder from this list
  map<int, int> detMap = fDetConfigManager.GetDetectorMap();  
  map<int, int>::iterator detMapItr = detMap.begin();

  // === Loop over detectors and make a vector of NoiseData objects for each ===

  for( ; detMapItr != detMap.end(); detMapItr++)
  {
     int detNum = detMapItr->first;
     int detType = detMapItr->second;
     int detCode = ChannelMapHelper::CalcDetCodeBase(detType, detNum);

     //template flag
     bool calcPhononTemplate = fUserData.CalcPhononTemplate(detNum);
     bool calcChargeTemplate = fUserData.CalcChargeTemplate(detNum);
     
     //make vector of NoiseData objects for this detector
     vector<NoiseData> tempVectorOfNoiseData;
     int noiseDataCtr = 0; //keep track of index number associated w/ detector code

     //construct the sum of phonon template and sum of charge templates with proper normalization
     vector<double> phononSumTemplate;
     double phononSumNorm = 0.0;
     double phononSampleRate = 0.0; 

     //1.  Loop and make a NoiseData object for every channel... 
     
     //fill the channel name list based on the detector type
     vector<string> channelNameList; 
     ChannelMapHelper::FillAllChannelList(detType, channelNameList);

     cout <<"Filled the channel list! The number of channels is " << channelNameList.size() << endl; 

     for(uint chanItr=0; chanItr < channelNameList.size(); chanItr++)
     {
	string channelName = channelNameList[chanItr];
	string channelType = ChannelMapHelper::GetChannelType(channelName);
	string parNameBase = ChannelMapHelper::GetChannelNameBase(channelName);

//	cout<<"Curr chan: "<<channelName<<endl;

	//Getting trigger windows and sampleRate
	double sampleRate = fDetConfigManager.GetSampleRate(detCode+chanItr);
	int nbBins = (int)fDetConfigManager.GetTraceLength(detCode+chanItr); //should be an int
	int preTriggerBins = (int)(sampleRate*fDetConfigManager.GetTriggerTime(detCode+chanItr)); 

	//Load templates and construct PT and QT templates with proper normalization
	double templateNorm = fUserData.GetDoubleParameter(detNum,parNameBase + "_PULSENORM");
        vector<double> templateVect;

	//if phonon
	if(channelType == "phonon") 
	{ 
	   phononSampleRate = sampleRate; //store this for PT template
	   
           // phonon template: make a double exponential shape template 
          
           if (calcPhononTemplate) {

             double risetime = fUserData.GetRiseTime(detNum,detType,channelName);
             double falltime = fUserData.GetFallTime(detNum,detType,channelName);
             templateVect = fTemplateData.GetDoubleExpForm(risetime,falltime,nbBins,preTriggerBins,sampleRate);

	     //double normalization = PulseTools::Area(templateVect)/(templateNorm*sampleRate);
             double normalization = PulseTools::MaxADC(templateVect)/templateNorm;
	     templateVect = PulseTools::Normalize(templateVect, normalization);

           } 
	   else 
	   {
	     templateVect = fTemplateData.GetTemplate(detNum, channelName);
	   }
        
	   //for PT template
	   phononSumNorm +=  1.0;

	   //add the template to the sum of phonon templates
	   //no relative normalization applied because agreement with template shape is generally poor to begin with
	   if(phononSumTemplate.size() == 0)
	   {
	      phononSumTemplate = templateVect;
              if (!calcPhononTemplate)
	      {
                phononSumTemplate = fTemplateData.GetTemplate(detNum, "PT");
	      }

	   } else {  
               if (calcPhononTemplate)
	         phononSumTemplate = PulseTools::SumPulses(phononSumTemplate, templateVect); 				    
	   }

	} 
        else //if charge
	{

           // charge template
           if (calcChargeTemplate) {

	       double risetime = fUserData.GetDoubleParameter(detNum, "Q_RISE_TIME");
	       double falltime = fUserData.GetDoubleParameter(detNum, "Q_FALL_TIME");
	       templateVect = fTemplateData.GetDoubleExpForm(risetime,falltime,nbBins,preTriggerBins,sampleRate);
	       double normalization = PulseTools::MaxADC(templateVect)/templateNorm;
               templateVect = PulseTools::Normalize(templateVect, normalization);
            
           } 
	   else 
	   {
	     templateVect = fTemplateData.GetTemplate(detNum, channelName);
	     templateVect = PulseTools::Normalize(templateVect, templateNorm);
	   }
        

	   //no QT calculation right now

	} //end else charge

	//Store templates for individual channels QI, QO, PA, PB, PC, PD
	NoiseData tempNoiseData(detCode+chanItr, channelName);	
	tempNoiseData.LoadSampleRate(sampleRate); //do this before loading the template
	tempNoiseData.LoadTemplate(templateVect); //store pulse template
	tempVectorOfNoiseData.push_back(tempNoiseData);
	noiseDataCtr++;

     } //end loop over channels
     

     //2. Last, make a NoiseData object for PT and QT  (and PS1, PS2 for iZIP)
     //  (to store selection histograms and compute noise quantities for sum of pulses)
     
     //constructing QT NoiseData object (no template or sample rate stored), QT presently used for pulse selection only
     NoiseData tempQTNoiseData(detCode + ChannelMapHelper::GetQTIndex(detType), "QT");
     tempVectorOfNoiseData.push_back(tempQTNoiseData);
     noiseDataCtr++;

     //constructing PT NoiseData object and template
     if(ChannelMapHelper::GetNPhononChannels(detType) > 0){  //**********************ATTEMPTED FIX FOR Q only DETECTOR********************** 
       NoiseData tempPTNoiseData(detCode + ChannelMapHelper::GetPTIndex(detType), "PT");
     
       if(calcPhononTemplate)
         phononSumTemplate= PulseTools::Normalize(phononSumTemplate, phononSumNorm);  //norm by num phonon channels, but only for funct form

       tempPTNoiseData.LoadSampleRate(phononSampleRate);  //do this before loading the template
       tempPTNoiseData.LoadTemplate(phononSumTemplate);
       tempVectorOfNoiseData.push_back(tempPTNoiseData);
       noiseDataCtr++;
     }//*************END Q only fix


     // constructing PS1/PS2 NoiseData object and template
     if (detType==BatRootTypes::kiZIPSoudan) { 
            
        NoiseData tempPS1NoiseData(detCode + ChannelMapHelper::GetPS1Index(detType), "PS1");
        tempPS1NoiseData.LoadSampleRate(phononSampleRate);  //do this before loading the template
        tempPS1NoiseData.LoadTemplate(phononSumTemplate);
        tempVectorOfNoiseData.push_back(tempPS1NoiseData);
        noiseDataCtr++;

        NoiseData tempPS2NoiseData(detCode + ChannelMapHelper::GetPS2Index(detType), "PS2");
        tempPS2NoiseData.LoadSampleRate(phononSampleRate);  //do this before loading the template
        tempPS2NoiseData.LoadTemplate(phononSumTemplate);
        tempVectorOfNoiseData.push_back(tempPS2NoiseData);
        noiseDataCtr++;
 
     }


     //3. Make a NoiseData object for the cross talk components - may be zero depending on detector type
     vector<string> xTalkNameList; 
     ChannelMapHelper::FillCrossTalkNameList(detType, xTalkNameList);

     for(uint crossItr=0; crossItr < xTalkNameList.size(); crossItr++)
     {
	string channelName = xTalkNameList[crossItr];
	double sampleRate;
        double preTrigger;
        double postTrigger;
        	
	sampleRate = fDetConfigManager.GetSampleRate(detNum, "charge");
	preTrigger = fDetConfigManager.GetTriggerTime(detNum, "charge")*sampleRate;
	postTrigger = (int)fDetConfigManager.GetTraceLength(detNum, "charge") - preTrigger;

        int nbBins = (int)(preTrigger + postTrigger);

	NoiseData tempNoiseData(detCode+noiseDataCtr, channelName);
	tempNoiseData.LoadSampleRate(sampleRate);  //do this before loading the template
	
        vector<double> templateVect;
        if (calcChargeTemplate)
	{
           templateVect = fTemplateData.GetZerosForm(nbBins);	
	}
        else
	{
	  templateVect = fTemplateData.GetTemplate(detNum, channelName);
	}

	tempNoiseData.LoadTemplate(templateVect); //store pulse template
	tempVectorOfNoiseData.push_back(tempNoiseData);
	noiseDataCtr++;

     }


     // 4. Additional noise data: 
     //         - Phonon/charge Glitches
     //         - Low frequency noise
     //         - 2D slow/fast templates
     
     // phonon Glitch1
     if (fUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhononGlitch1")) {
      
         NoiseData tempPTglitch1NoiseData(detCode+noiseDataCtr, "PTglitch1");
         tempPTglitch1NoiseData.LoadSampleRate(phononSampleRate); 
  
         // TEMP FIXME: if calcPhononTemplate, use regular phonon template, will
         // need to calculate a better template in the future
       
         vector<double> templateVect = phononSumTemplate;
         if (!calcPhononTemplate)
               templateVect =  fTemplateData.GetTemplate(detNum, "PTglitch1"); 

         tempPTglitch1NoiseData.LoadTemplate(templateVect);
         tempVectorOfNoiseData.push_back(tempPTglitch1NoiseData);
         noiseDataCtr++;
     }



      // phonon LFnoise1
     if (fUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhononLFnoise1")) {
      
         NoiseData tempPTlfnoise1NoiseData(detCode+noiseDataCtr, "PTlfnoise1");
         tempPTlfnoise1NoiseData.LoadSampleRate(phononSampleRate); 
  
         // TEMP FIXME: if calcPhononTemplate, use regular phonon template, will
         // need to calculate a better template in the future
       
         vector<double> templateVect = phononSumTemplate;
         if (!calcPhononTemplate)
               templateVect =  fTemplateData.GetTemplate(detNum, "PTlfnoise1"); 

         tempPTlfnoise1NoiseData.LoadTemplate(templateVect);
         tempVectorOfNoiseData.push_back(tempPTlfnoise1NoiseData);
         noiseDataCtr++;
     }


     // Add 2D OF slow and fast template
     
     // individual channels
     if(fUserData.DoAlgorithm(detNum, "phonon", "OptimalFilterPhonon1X2")) {
       
       for(uint chanItr=0; chanItr < channelNameList.size(); chanItr++)
	 {
	   string channelName = channelNameList[chanItr];
	   string channelType = ChannelMapHelper::GetChannelType(channelName);

	   if (channelType != "phonon")
	     continue;

	   // get sample rate
	   double sampleRate = fDetConfigManager.GetSampleRate(detCode+chanItr);
	   
	   // get templates
	   string channelNameSlow =  channelName + "slow";
	   vector<double>  templateVectSlow = fTemplateData.GetTemplate(detNum, channelNameSlow);

	   string channelNameFast =  channelName + "fast";
	   vector<double>  templateVectFast= fTemplateData.GetTemplate(detNum, channelNameFast);
	   

	   //Create Noise data and Store templates

	   // slow template
	   NoiseData tempNoiseDataSlow(detCode+noiseDataCtr, channelNameSlow);	
	   tempNoiseDataSlow.LoadSampleRate(sampleRate); //do this before loading the template
	   tempNoiseDataSlow.LoadTemplate(templateVectSlow); //store pulse template

	   tempVectorOfNoiseData.push_back(tempNoiseDataSlow);
	   noiseDataCtr++;

	   // fast  template
	   NoiseData tempNoiseDataFast(detCode+noiseDataCtr, channelNameFast);	
	   tempNoiseDataFast.LoadSampleRate(sampleRate); //do this before loading the template
	   tempNoiseDataFast.LoadTemplate(templateVectFast); //store pulse template

	   tempVectorOfNoiseData.push_back(tempNoiseDataFast);
	   noiseDataCtr++;

	 }
     }

     // PT/PS1/PS2
     if(fUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhonon1X2"))
       {
     	 // Get slow/fast  templates
	 vector<double>  templateVectSlow = fTemplateData.GetTemplate(detNum, "PTslow");
	 vector<double>  templateVectFast = fTemplateData.GetTemplate(detNum, "PTfast");
	 
	 //Create Noise data and Store templates
	 
	 // Slow template
	 NoiseData tempNoiseDataSlow(detCode+noiseDataCtr, "PTslow");	
	 tempNoiseDataSlow.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseDataSlow.LoadTemplate(templateVectSlow); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseDataSlow);
	 noiseDataCtr++;
	 
	 // Fast template
	 NoiseData tempNoiseDataFast(detCode+noiseDataCtr, "PTfast");	
	 tempNoiseDataFast.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseDataFast.LoadTemplate(templateVectFast); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseDataFast);
	 noiseDataCtr++;
       }
       
     if(fUserData.DoAlgorithm(detNum, "PSIDES", "OptimalFilterPhonon1X2") &&
	detType==BatRootTypes::kiZIPSoudan)
       {
     
	 // Get slow/fast  templates
	 vector<double>  templateVectSlow = fTemplateData.GetTemplate(detNum, "PTslow");
	 vector<double>  templateVectFast = fTemplateData.GetTemplate(detNum, "PTfast");
	 
	 //Create Noise data and Store templates
	 
	 // S! template
	 NoiseData tempNoiseDataSlowS1(detCode+noiseDataCtr, "PS1slow");	
	 tempNoiseDataSlowS1.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseDataSlowS1.LoadTemplate(templateVectSlow); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseDataSlowS1);
	 noiseDataCtr++;
	 
	 
	 NoiseData tempNoiseDataFastS1(detCode+noiseDataCtr, "PS1fast");	
	 tempNoiseDataFastS1.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseDataFastS1.LoadTemplate(templateVectFast); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseDataFastS1);
	 noiseDataCtr++;
	 
	 // S2 template
	 NoiseData tempNoiseDataSlowS2(detCode+noiseDataCtr, "PS2slow");	
	 tempNoiseDataSlowS2.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseDataSlowS2.LoadTemplate(templateVectSlow); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseDataSlowS2);
	 noiseDataCtr++;
	 
	 
	 NoiseData tempNoiseDataFastS2(detCode+noiseDataCtr, "PS2fast");	
	 tempNoiseDataFastS2.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseDataFastS2.LoadTemplate(templateVectFast); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseDataFastS2);
	 noiseDataCtr++;

       }
       


     // Add DMC template
     
     // individual channels
     if(fUserData.DoAlgorithm(detNum, "phonon", "OptimalFilterPhononDMC")) {
       
       for(uint chanItr=0; chanItr < channelNameList.size(); chanItr++)
	 {
	   string channelName = channelNameList[chanItr];
	   string channelType = ChannelMapHelper::GetChannelType(channelName);
	   
	   if (channelType != "phonon")
	     continue;
	   
	   // get sample rate
	   double sampleRate = fDetConfigManager.GetSampleRate(detCode+chanItr);
	   
	   // get templates
	   string channelNameDMC =  channelName + "dmc";
	   vector<double>  templateVect= fTemplateData.GetTemplate(detNum, channelNameDMC);
	   

	   //Create Noise data and Store template
	   NoiseData tempNoiseData(detCode+noiseDataCtr, channelNameDMC);	
	   tempNoiseData.LoadSampleRate(sampleRate); //do this before loading the template
	   tempNoiseData.LoadTemplate(templateVect); //store pulse template

	   tempVectorOfNoiseData.push_back(tempNoiseData);
	   noiseDataCtr++;

	 }
     }

     // PT/PS1/PS2
     if(fUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhononDMC"))
       {
     	 // Get template
	 vector<double>  templateVect = fTemplateData.GetTemplate(detNum, "PTdmc");
	 
	 //Create Noise data and Store templates
	 NoiseData tempNoiseData(detCode+noiseDataCtr, "PTdmc");	
	 tempNoiseData.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseData.LoadTemplate(templateVect); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseData);
	 noiseDataCtr++;
       }
       
     if(fUserData.DoAlgorithm(detNum, "PSIDES", "OptimalFilterPhononDMC") &&
	detType==BatRootTypes::kiZIPSoudan)
       {
     
	 // Get template (using PT)
	 vector<double>  templateVect = fTemplateData.GetTemplate(detNum, "PTdmc");
	 
	 //Create Noise data and Store templates
	 
	 // S1 template
	 NoiseData tempNoiseDataS1(detCode+noiseDataCtr, "PS1dmc");	
	 tempNoiseDataS1.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseDataS1.LoadTemplate(templateVect); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseDataS1);
	 noiseDataCtr++;
	 
	 // S2 template
	 NoiseData tempNoiseDataS2(detCode+noiseDataCtr, "PS2dmc");	
	 tempNoiseDataS2.LoadSampleRate(phononSampleRate); //do this before loading the template
	 tempNoiseDataS2.LoadTemplate(templateVect); //store pulse template
	 tempVectorOfNoiseData.push_back(tempNoiseDataS2);
	 noiseDataCtr++;

       }
       
  
     
     //store vector in map according to detector number
     fMapOfNoiseData.insert(pair<int, vector<NoiseData> >(detNum, tempVectorOfNoiseData));

  } //done looping over detectors


  return;
}


//This function builds a map of CorrelationData objects to be used while looping over events.
//But only if the ChargeNoiseCorrelation flag is set to true in the processing config file
void NoiseBuilder::ConfigureCorrelationData()
{
  //CorrelationData are defined for pairs of detectors.   
  //There is no inherent order to this list.
 
  // Retrieve the detector map and configure NoiseBuilder from this list
  map<int, int> detMap = fDetConfigManager.GetDetectorMap();  
  map<int, int>::iterator detMapItr = detMap.begin();

  // === Loop and make a vector of CorrelationData objects for every detector ===

  for( ; detMapItr != detMap.end(); detMapItr++)
  {
     int detNum = detMapItr->first;
     int detType = detMapItr->second;
     int detCode = ChannelMapHelper::CalcDetCodeBase(detType, detNum);

     //check if this is requested by the processing config file
     if( !fUserData.DoAlgorithm(detNum, "charge", "ChargeNoiseCovariance") )   { continue; }

     //at the moment, this code doesnt' work for izips
     if( detType != BatRootTypes::kZIPDetType && detType != BatRootTypes::kMZIPDetType )
     {
       cout <<"ERROR! NoiseBuilder::ConfigureCorrelationData - Requesting correlation data for unsupported detector type!"
	    <<"You must implement this before proceeding."
	    << endl;
       exit(1);
     }

     //make vector of CorrelationData objects for this detector
     vector<CorrelationData> tempVectorOfCorrelationData;

     //Getting trigger windows and charge sample rate - retrieves for channel 0, which should be charge
     double qSampleRate = fDetConfigManager.GetSampleRate(detCode);
     
     //Construct a CorrelationData object for the QIQO pair
     CorrelationData tempCorrelationData(detNum, "QI", "QO");	
     tempCorrelationData.LoadSampleRate(qSampleRate); 
     tempVectorOfCorrelationData.push_back(tempCorrelationData);


     //store vector in map according to detector number
     fMapOfCorrelationData.insert(pair<int, vector<CorrelationData> >(detNum, tempVectorOfCorrelationData));


  } //done looping over zips

  return;

}


// ======================== for reading the raw data file ====================================

void NoiseBuilder::OpenRawFile(const string& inputRawDataFile)
{
  //open the file
  fRawReader.OpenRawDataFile(fUserData.GetPath("RAW_DATA"), inputRawDataFile);
   
  //read the header
  fRawReader.ReadFileHeader(true);

  // Allow modification of configuration if needed   
  if (fUserData.DoModifyRawData()) {
 
      map<int, string> modificationMap = fUserData.GetRawDataModificationMap();
   
      if (!modificationMap.empty())
         fRawReader.ModifyRawData(modificationMap);
      else {
        cerr <<"NoiseBuilder::ERROR!  Cannot modify raw data without modification type!"  
	    <<"\nNeed to add a MODIFICATION_TYPE parameter in processing settings."
	    << endl;
        exit(1);
      }
   }

  
  return;
}

void NoiseBuilder::ResetRawFile()
{
  
  fRawReader.ResetRawDataFile();
   
  //read the header
  fRawReader.ReadFileHeader(true);
  
  // Allow modification of configuration if needed   
  if (fUserData.DoModifyRawData()) {
    
      map<int, string> modificationMap = fUserData.GetRawDataModificationMap();
   
      if (!modificationMap.empty())
         fRawReader.ModifyRawData(modificationMap);
      else {
        cerr <<"NoiseBuilder::ERROR!  Cannot modify raw data without modification type!"  
	    <<"\nNeed to add a MODIFICATION_TYPE parameter in processing settings."
	    << endl;
        exit(1);
      }
  }

  return;
}

int NoiseBuilder::ReadNextEvent()
{

   //First clear data containers of previous event's data
   fRawReader.Clear();

   //Read the event!
   int checkStatus = fRawReader.ReadRawDataRecord();

   // Allow modification of pulse data if needed
   if (checkStatus && fUserData.DoModifyRawData()) {

      map<int, string> modificationMap = fUserData.GetRawDataModificationMap();
   
      if (!modificationMap.empty())
         fRawReader.ModifyRawData(modificationMap);
      else {
        cerr <<"NoiseBuilder::ERROR!  Cannot modify raw data without modification type!"  
	    <<"\nNeed to add a MODIFICATION_TYPE parameter in processing settings."
	    << endl;
        exit(1);
      }
   }


   return checkStatus;
}

// ======================== for the output file ====================================

void NoiseBuilder::ConfigureOutputFile(const string& outputFileName)
{
   bool debugOn = false;

    //Open output file
   string noisePath = fUserData.GetPath("NOISE_FILES");
   string noisePrefix = fUserData.GetPrefix("NOISE_PREFIX");
   fOutputFile = TFile::Open(Form("%s%s%s", noisePath.c_str(), noisePrefix.c_str(), outputFileName.c_str()), "recreate",Form("%s%s", noisePrefix.c_str(),outputFileName.c_str()));

 
   // === setup a directory for all zips regardless of whether we will have templates for them 

   // Retrieve the detector map and configure NoiseBuilder from this list
   map<int, int> detMap = fDetConfigManager.GetDetectorMap();  
   map<int, int>::iterator detMapItr = detMap.begin();


   // === loop over selected detectors ===

   for( ; detMapItr != detMap.end(); detMapItr++)
   {
      int detNum = detMapItr->first;

      if(debugOn) cout <<"making a directory!" << detNum << endl;
      fOutputFile->mkdir(Form("zip%d", detNum), Form("zip%d", detNum)); 
   }

   
   // === setup an info and a detectorConfig directory ===

   fOutputFile->mkdir("infoDir", "version and cut info");  
   fOutputFile->mkdir("detectorConfigDir","Detector configuration directory");   
 

   return;
}

void NoiseBuilder::WriteOutputFile()
{

  cout <<"\nWriting filter file: " << fOutputFile->GetName() << endl;

  // === Get date ===

  char date[20];
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(date,80,"%d-%b-%Y",timeinfo); 
   
  // === Get git tag===
  char cbversion[1024];
  char bcversion[1024];
  sprintf(cbversion,"%s",__CB_GIT_VERSION);
  sprintf(bcversion,"%s",__BC_GIT_VERSION);

  //decompose the git tag
  string gitTagStr_cdmsbats;
  int gitTagStr_cdmsbats_past=0;
  string gitTagStr_cdmsbats_hash;
  string gitTagStr_cdmsbats_prefix;
  string gitTagStr_batcommon;
  int gitTagStr_batcommon_past=0;
  string gitTagStr_batcommon_hash;
  string gitTagStr_batcommon_prefix;

  //set up the regex match
  regex_t version_regex;
  //string trailing="-([1-9][0-9]*)-g([a-f0-9]{40})";
  string matchversion="((.+)_)?v([0-9]+[.-][0-9]+([.-][0-9]+)?)(-([1-9][0-9]*)-g([a-f0-9]{40}))?";
  regmatch_t matchptr[8];
  char submatch[1024];
  int reti = regcomp(&version_regex,matchversion.c_str(),REG_EXTENDED);

  //match the cdmsbats version
  reti = regexec(&version_regex,cbversion,8,matchptr,0);
  if(reti==0){
   ostringstream gitTagStream;
   int size;

   //the version
   size = matchptr[3].rm_eo - matchptr[3].rm_so;
   strncpy(submatch,(cbversion+matchptr[3].rm_so),size);
   submatch[size]='\0';
   gitTagStream << "v" << submatch;
   gitTagStr_cdmsbats = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();

   //the commit
   size = matchptr[6].rm_eo - matchptr[6].rm_so;
   strncpy(submatch,(cbversion+matchptr[6].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   if(gitTagStream.str()=="")
     gitTagStr_cdmsbats_past=0;
   else{
     istringstream gitTagStreamIn(gitTagStream.str());
     gitTagStreamIn >> gitTagStr_cdmsbats_past;
   }
   gitTagStream.str("");
   gitTagStream.clear();
   
   //the hash
   size = matchptr[7].rm_eo - matchptr[7].rm_so;
   strncpy(submatch,(cbversion+matchptr[7].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   gitTagStr_cdmsbats_hash = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();

   //the prefix 
   size = matchptr[2].rm_eo - matchptr[2].rm_so;
   strncpy(submatch,(cbversion+matchptr[2].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   gitTagStr_cdmsbats_prefix = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();
  }
  else{
    gitTagStr_cdmsbats="";
    gitTagStr_cdmsbats_past=0;
    gitTagStr_cdmsbats_hash="";
    gitTagStr_cdmsbats_prefix="";
  }

  //match the BatCommon version
  reti = regexec(&version_regex,bcversion,8,matchptr,0);
  if(reti==0){
   ostringstream gitTagStream;
   int size;

   //the version
   size = matchptr[3].rm_eo - matchptr[3].rm_so;
   strncpy(submatch,(bcversion+matchptr[3].rm_so),size);
   submatch[size]='\0';
   gitTagStream << "v" << submatch;
   gitTagStr_batcommon = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();

   //the commit
   size = matchptr[6].rm_eo - matchptr[6].rm_so;
   strncpy(submatch,(bcversion+matchptr[6].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   if(gitTagStream.str()=="")
     gitTagStr_batcommon_past=0;
   else{
     istringstream gitTagStreamIn(gitTagStream.str());
     gitTagStreamIn >> gitTagStr_batcommon_past;
   }
   gitTagStream.str("");
   gitTagStream.clear();
   
   //the hash
   size = matchptr[7].rm_eo - matchptr[7].rm_so;
   strncpy(submatch,(bcversion+matchptr[7].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   gitTagStr_batcommon_hash = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();

   //the prefix 
   size = matchptr[2].rm_eo - matchptr[2].rm_so;
   strncpy(submatch,(bcversion+matchptr[2].rm_so),size);
   submatch[size]='\0';
   gitTagStream << submatch;
   gitTagStr_batcommon_prefix = gitTagStream.str();
   gitTagStream.str("");
   gitTagStream.clear();
  }
  else{
    gitTagStr_batcommon="";
    gitTagStr_batcommon_past=0;
    gitTagStr_batcommon_hash="";
    gitTagStr_batcommon_prefix="";
  }


  if(gitTagStr_cdmsbats_prefix!="")
    cout << "NoiseBuilder::WriteOutputFile WARNING! proceeding with non-standard cdmsbats tag prefix, " << gitTagStr_cdmsbats_prefix << endl;

  if(gitTagStr_cdmsbats_past>0)
    cout << "NoiseBuilder::WriteOutputFile WARNING! NOT at a cdmsbats release, we are " << gitTagStr_cdmsbats_past << " commits past a release, on commit " << gitTagStr_cdmsbats_hash << endl;

  if(gitTagStr_cdmsbats==""){
    cerr << "NoiseBuilder::WriteOutputFile ERROR! attempting to proceed with cdmsbats code of UNKNOWN origin, clone the code again: "
	 << "\n\tgit clone username@nero.stanford.edu:/data/git/Reconstruction/cdmsbats " 
	 << "\n\tcd cdmsbats/BatCommon" 
	 << "\n\tgit pull " << endl;
    exit(1);
   }


  if(gitTagStr_batcommon_prefix!="")
    cout << "NoiseBuilder::WriteOutputFile WARNING! proceeding with non-standard batcommon tag prefix, " << gitTagStr_batcommon_prefix << endl;

  if(gitTagStr_batcommon_past>0)
    cout << "NoiseBuilder::WriteOutputFile WARNING! NOT at a batcommon release, we are " << gitTagStr_batcommon_past << " commits past a release, on commit " << gitTagStr_batcommon_hash << endl;

  if(gitTagStr_batcommon==""){
    cerr << "NoiseBuilder::WriteOutputFile ERROR! proceeding with batcommon code of UNKNOWN origin, clone the code again: "
	 << "\n\tgit clone username@nero.stanford.edu:/data/git/Reconstruction/cdmsbats " 
	 << "\n\tcd cdmsbats/BatCommon" 
	 << "\n\tgit pull " << endl;
    exit(1);
   }

  // ===== Writing Output =====

  map<int, int> detMap = fDetConfigManager.GetDetectorMap();  
  map<int, int>::iterator detMapItr = detMap.begin();

  //For each detector, store all NoiseData histograms in folder with detector name
  for( ; detMapItr != detMap.end(); detMapItr++)
    {
      int detNum = detMapItr->first;

      //for later storage in the filterInfo tree
      float  puMax = -999999.;
      float  noisePMax = -999999.;
      float  noiseQMax = -999999.;
      float  noisePOFchisqCut= -999999.;

      fOutputFile->cd(Form("zip%d",detNum));


      // ========== Store the noise and filter data ===============

      //retrive the vector of NoiseData for this zip
      vector<NoiseData>* noiseDataList;
      map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
      
      if( noiseMapItr != fMapOfNoiseData.end() )
      {
	//Get the noise list for this zip
	noiseDataList = &(noiseMapItr->second);

	//counter for duplicate items that only need to be stored once
	int storeOnceCtr = 1;

        // counter for storing Qinverse just once per detector (and both sides for iZIP)
        int storeOnceQinverseCtr = 1;
        int storeOnceQS1inverseCtr = 1;
        int storeOnceQS2inverseCtr = 1;
	 
	//loop, make use of ordering of NoiseData objects (should correpond to PulseData list)
	for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
	{
	  NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);

          string channelName = aNoiseData->GetChannelName();
	    
	  //writing noise selection histograms
	  for(uint histItr=0; histItr < aNoiseData->fNoiseSelectionHistograms.size(); histItr++)
  	  {
	    (aNoiseData->fNoiseSelectionHistograms)[histItr]->Write();
	  }
	    
	  //store noise cuts for filterInfo tree
	  if(channelName == "PT") noisePMax = aNoiseData->fMinMaxCut;
          if(channelName == "PT") noisePOFchisqCut = aNoiseData->fPOFchisqCut;
	  if(channelName == "QT") noiseQMax = aNoiseData->fMinMaxCut;
	  if(storeOnceCtr == 1)   puMax = aNoiseData->fPileupCut; //same for all channels

	  //no noise quantities are calculated for QT right now
	  if(channelName == "QT") continue;
	    
	  //write noise quantities - if they exist
	  aNoiseData->GetHistNoisePSD().Write();
	  aNoiseData->GetHistNoiseFFT().Write();
	  aNoiseData->GetHistNoiseFFTsq().Write();
	  aNoiseData->GetHistOptimalFilterRe().Write();
	  aNoiseData->GetHistOptimalFilterIm().Write();
	  aNoiseData->GetHistNormFFT().Write();
	  aNoiseData->GetHistSigToNoiseSq().Write();

	  //write qinverse (charge only)
	  if (ChannelMapHelper::GetChannelType(channelName)=="charge") {

            if (storeOnceQS1inverseCtr==1 && channelName.find("S1")!=string::npos) {
              aNoiseData->GetHistQInverse().Write();  
              storeOnceQS1inverseCtr++;  }

            if (storeOnceQS2inverseCtr==1 && channelName.find("S2")!=string::npos) {
              aNoiseData->GetHistQInverse().Write();  
              storeOnceQS2inverseCtr++;  }
        
            if (storeOnceQinverseCtr==1 && !(channelName.find("S1")!=string::npos || channelName.find("S2")!=string::npos)) {
              aNoiseData->GetHistQInverse().Write();  
              storeOnceQinverseCtr++;  }

          }
  

	  //write pulse template - if they exist
	  aNoiseData->GetHistTemplateTime().Write();
	  aNoiseData->GetHistTemplateFFTRe().Write();
	  aNoiseData->GetHistTemplateFFTIm().Write();
	  
	  //write the event list - only do it once per detector 
	  if(storeOnceCtr == 1)
	    aNoiseData->GetHistEventList().Write();
	  
	  //increment storeOnceCtr
	  storeOnceCtr++;


          // store covariance matrix (if available)
          TGraphErrors *graph = fTemplateData.GetCovarianceMatrix(detNum,channelName);  
          if (graph != NULL) graph->Write();




	} //end loop over pulses on this zip
	 

	// ========== Store the correlation data, if requested ===============           

	//First check if this is requested by the processing config file
	if( fUserData.DoAlgorithm(detNum, "charge", "ChargeNoiseCovariance") )   
	{
	  //retrive the vector of CorrelationData for this zip
	  vector<CorrelationData>* correlationDataList;
	  map< int, vector<CorrelationData> >::iterator correlationMapItr = fMapOfCorrelationData.find(detNum);
	  
	  if( correlationMapItr != fMapOfCorrelationData.end() )
 	  {
	    //Get the correlation list for this zip
	    correlationDataList = &(correlationMapItr->second);
	       
	    //loop over pulses, make use of ordering of CorrelationData objects (should correpond to PulseData list)
	    for(uint correlationItr = 0; correlationItr < correlationDataList->size(); correlationItr++)
	    {
	      CorrelationData* aCorrelationData = &((*correlationDataList)[correlationItr]);
	      (aCorrelationData->GetHistNoiseCov()).Write();  
	    } //end loop over pulses
	  }
	    
	} //end if correlation data calc requested

	// ========= Store the info data ===============
	 
	fOutputFile->cd("infoDir");

	// create a Tree for each zip to store info
	TTree *infoTree = new TTree(Form("infoZip%d",detNum),Form("Z%d filter information tree",detNum));
	
	// filter tag 
	string fileName = fOutputFile->GetTitle();
	size_t  pos = fileName.find(".root");
	string filterTagStr = fileName.substr(0,pos)+ "_" + date;
	char filterTag[filterTagStr.size()+1];
	strcpy(filterTag,filterTagStr.c_str()); 
	
	// template tag
	string templateTagStr = fTemplateData.GetTemplateTag(detNum);
	char templateTag[templateTagStr.size()+1];
	strcpy(templateTag,templateTagStr.c_str());

	// git tag
	ostringstream gitTagStream_cdmsbats;
	ostringstream gitTagStream_batcommon;
	gitTagStream_cdmsbats << cbversion;
	gitTagStream_batcommon << bcversion;
	char gitTag_cdmsbats[gitTagStream_cdmsbats.str().size()+1];
	strcpy(gitTag_cdmsbats,gitTagStream_cdmsbats.str().c_str());
	char gitTag_batcommon[gitTagStream_batcommon.str().size()+1];
	strcpy(gitTag_batcommon,gitTagStream_batcommon.str().c_str());
	
	infoTree->Branch("date", &date, "date/C");
	infoTree->Branch("templateTag",&templateTag, "templateTag/C");
	infoTree->Branch("filterTag",&filterTag, "filterTag/C");
	infoTree->Branch("gitTag_cdmsbats",&gitTag_cdmsbats, "gitTag_cdmsbats/C");
	infoTree->Branch("gitTag_batcommon",&gitTag_batcommon, "gitTag_batcommon/C");
	infoTree->Branch("puMax", &puMax, "puMax/F");
	infoTree->Branch("noisePMax", &noisePMax, "noisePMax/F");
	infoTree->Branch("noiseQMax", &noiseQMax, "noiseQMax/F");
        infoTree->Branch("noisePOFchisqCut", &noisePOFchisqCut, "noisePOFchisqCut/F");	
	
        infoTree->Fill();
	infoTree->Write();


	// ========= Store the detector config data ===============
	
	fOutputFile->cd("detectorConfigDir");
	TTree* configTree = new TTree(Form("detectorConfigZip%d", detNum), Form("detectorConfigZip%d", detNum));
	
	// store the detector tower index, which actually comes from the user settings file
	double detTowerIndex = (double)fUserData.GetIntParameter(detNum, "INDEX_IN_TOWER");
	configTree->Branch("IndexWithinTower", &detTowerIndex, "IndexWithinTower/D");
	
	// now loop over the detector config map and store all quantities
	map<string, double> detConfigTest = fDetConfigManager.GetDetectorConfiguration(detNum);    
	map< string, double >::iterator detConfigItr = detConfigTest.begin();
      
	for( ; detConfigItr != detConfigTest.end(); detConfigItr++)
	{
	
	  string name = detConfigItr->first;
	  configTree->Branch(name.c_str(), &(detConfigItr->second), (name+"/D").c_str());
	  
	} // end loop over config parameters

	configTree->Fill();
	configTree->Write();

      } //end if detNum found in map	 
      
    } //end loop over zips

  fOutputFile->Close();

  return;

}


void NoiseBuilder::StorePulses(int detNum)
{

   string eventNumber = Form("%d",fAdminData.GetEvent());
   
   //create this directory if it doesn't exist
   if( fOutputFile->FindObjectAny(Form("pulsedumps_zip%d", detNum)) == NULL)
   {
      fOutputFile->mkdir(Form("pulsedumps_zip%d", detNum), Form("pulsedumps_zip%d", detNum)); 
   }
   
   //change to the directory
   fOutputFile->cd(Form("pulsedumps_zip%d",detNum));


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
	 vector<double> aPulse = aPulseData->GetRawPulse();
	 string channelName = aPulseData->GetChannelName();
	 
	 (PulseTools::Vector2TH1D(aPulse, channelName+"RawPulse"+eventNumber)).Write();
      }

   } //end if detNum found in map

   return;

}


// ================== Pulse Calculations  ========================

//normalized to take out driver gains, this is used by the minmax cut
//Note:  this code expects to be run on zips (i.e. 2 charge and 4 phonon channels)
void NoiseBuilder::CalcSumOfPulses(int detNum)
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
       
       vector<double> phononSumPulse;
       double phononSumNorm = 0.;

       vector<double> chargeSumPulse;
       double chargeSumNorm = 0.;

       // get status RQ (if available)
       // Do not use channel if broken
       
       vector<string>  brokenChannels;
       if (fUserData.DoRead("DET_STATUS_FILE"))
            brokenChannels = fUserData.GetBrokenChannelList(detNum);  


       // ===== loop ZIP pulse collection =======

	  cout << "Zip pulse list size: (MF was here)" << zipPulseList->size() <<endl;
       for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
       {

	  PulseData* aPulseData = &((*zipPulseList)[pulseItr]); 
	  int    detCode = aPulseData->GetDetectorCode();
	  //string sensorType = aPulseData->GetChannelType(); 
	  string channelName = aPulseData->GetChannelName();
	  detType = aPulseData->GetDetectorType();

	  cout << "Curr chan: (MF was here)" << channelName <<endl;

          // skip channel if not use for noise selection  (set in config file)  
          if (!fUserData.UseChannelNoiseSelection(detNum, detType,channelName)) continue;
        
                   
          // skip channel if broken
           if (find(brokenChannels.begin(), brokenChannels.end(), channelName) != brokenChannels.end()) continue;



          // ---------- construct sum of pulses ---------
	  
          //Now construct a pulse that is the sum of all (normalized) charge pulses
	  if(aPulseData->IsChargePulse()) 
	  { 
	     double pulseNorm = fDetConfigManager.GetDriverGain(detCode, fAdminData.GetEventTime());
	     chargeSumNorm += 1.0/(ChannelMapHelper::GetNChargeChannels(detType)*pulseNorm);

	     if(chargeSumPulse.size() == 0)
	     {
		chargeSumPulse = PulseTools::Normalize(aPulseData->fPulseVector, pulseNorm); 		
	     } 
	     else 
	     {	
		vector<double> tempPulse = PulseTools::Normalize(aPulseData->fPulseVector, pulseNorm); 
		chargeSumPulse = PulseTools::SumPulses(chargeSumPulse, tempPulse);     			    
	     }
	  }
	  
          //Now construct a pulse that is the sum of all (normalized) phonon pulses
	  if(aPulseData->IsPhononPulse())
	  {

             double pulseCalib = fUserData.GetRelativeCalibration(detNum, detType, channelName);
	     double pulseNorm = fDetConfigManager.GetDriverGain(detCode, fAdminData.GetEventTime());
	     phononSumNorm +=  1.0/(ChannelMapHelper::GetNPhononChannels(detType)*pulseNorm);

	     if(phononSumPulse.size() == 0)
	     {
		phononSumPulse = PulseTools::Normalize(aPulseData->fPulseVector, pulseNorm);
                phononSumPulse = PulseTools::Scale(phononSumPulse, pulseCalib); 
	     } 
	     else 
	     {
		vector<double> tempPulse = PulseTools::Normalize(aPulseData->fPulseVector, pulseNorm); 
	        tempPulse = PulseTools::Scale(tempPulse, pulseCalib); 
        	phononSumPulse = PulseTools::SumPulses(phononSumPulse, tempPulse); 				    
	     }
	  }
	  
       } // ======= end loop ZIP pulses  =======

       //Add special sum of charge pulses as an additional PulseData object in the PulseData list
       PulseData chargeSumPulseData;
       chargeSumPulseData.fDetType = detType;
       chargeSumPulseData.fDetNum = detNum;
       chargeSumPulseData.fDetChannel = ChannelMapHelper::GetQTIndex(detType); //depends on number of physical channels in the det
       chargeSumPulseData.fDetCode = ChannelMapHelper::CalcDetCodeBase(detType, detNum) + chargeSumPulseData.fDetChannel; 
       chargeSumPulseData.fChannelName = ChannelMapHelper::GetChannelName(detType, chargeSumPulseData.fDetChannel);
       chargeSumPulseData.fNADCBins = chargeSumPulse.size();

       chargeSumPulseData.fIsCharge = true;
       chargeSumPulseData.fIsZip = true;
       chargeSumPulseData.fIsPhonon = false;
       chargeSumPulseData.fIsVeto = false;

       if(chargeSumNorm == 0.0) chargeSumNorm = 1.0;  //no normalization if isr not read

       if(ChannelMapHelper::GetNChargeChannels(detType) > 0)  //**********************ATTEMPTED FIX FOR HV DETECTOR**********************
	  chargeSumPulseData.fPulseVector = PulseTools::Normalize(chargeSumPulse, chargeSumNorm); //this is normalized!

       zipPulseList->push_back(chargeSumPulseData);

       //Add special sum of phonon pulses as an additional PulseData object in the PulseData list
       
       if(ChannelMapHelper::GetNPhononChannels(detType) > 0){  //For Q only detector
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

	       if(phononSumNorm == 0.0) phononSumNorm = 1.0;  //no normalization if isr not read

	       // this is the weighted sum...the sign of the gain cancels! 
	       phononSumPulseData.fPulseVector = PulseTools::Normalize(phononSumPulse, phononSumNorm);  

	       zipPulseList->push_back(phononSumPulseData);

       }
    }  //end if detNum found in map

   return;
}

// ================== Noise Selection  ========================

//this histograms the minmax value for only the sum of phonon and the sum of charge pulses
void NoiseBuilder::ConstructMinMaxDistribution(int detNum)
{
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {
      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
   
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);
    
      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);

//	 cout << "NoiseBuilder::ConstructMinMaxDistrubution, currChan: " << aPulseData->GetChannelName() <<endl;

// 	 cout <<"NoiseBuilder::ConstructMinMaxDistrubution Checking ordering of MapOfZipPulses or MapofNoisePulses!"
// 	      <<"noise data is " << aNoiseData->GetChannelName() <<", and pulse data is " << aPulseData->GetChannelName() 
// 	      << endl;

	 //we only store this for PT and QT
	 if(aPulseData->GetChannelName() != "PT" && aPulseData->GetChannelName() != "QT") { continue; }

	 if(aPulseData->GetChannelName() == "QT" && ChannelMapHelper::GetNChargeChannels(aPulseData->GetDetectorType()) == 0) { continue; } // ******* MAT WUZ HERE ********

	 //make sure these objects correspond to the same detector and channel
	 if(aNoiseData->GetDetectorCode() != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::ConstructMinMaxDistrubution ERROR in ordering of MapOfZipPulses or MapofNoisePulses!"
                 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode() 
		 << endl;
	    exit(1);
	 }

	 //Initialize the histogram
	 int minmax_max = fUserData.GetIntParameter(detNum, "MINMAX_MAX");
	 aNoiseData->InitializeNoiseSelectionHistogram("minmax", minmax_max, 0.0, (double)minmax_max); //should be positive by construction

	 //Compute MinMax
	 //use float to circumvent rounding issues for case of true integer minmax values
	 float minmax = PulseTools::MinMax(aPulseData->GetRawPulse()); //"RawPulse" = weighted sum for case of PT and QT 
	 
	 cout <<"Calculating minmax for channel " <<aPulseData->GetChannelName() <<" value = " << minmax << endl;

	 //Store the value in the histogram 
	 (aNoiseData->fMinMaxValues).push_back(minmax);
	 aNoiseData->HistogramValue("minmax", minmax); 

      } //end loop over pulses on this zip

   } //end if detNum found in map	 

   return;
}

//apply minmax cut only on sum of phonon and sum of pulses
//the minmax distribution must be positive
void NoiseBuilder::CalcMinMaxCut(int detNum)
{
   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(noiseMapItr != fMapOfNoiseData.end() )
   {
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);

      //loop over NoiseData objects
      for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
      {
	 NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);

	 //we only cut on this value for PT and QT
	 if(aNoiseData->GetChannelName() != "PT" && aNoiseData->GetChannelName() != "QT") { continue; }

	 if(aNoiseData->GetChannelName() == "QT" && ChannelMapHelper::GetNChargeChannels(aNoiseData->GetDetectorType()) == 0) { continue; } // ******* MAT WUZ HERE ********

	 //Construct the minmax histogram
	 int nb = fUserData.GetIntParameter(detNum, "MINMAX_MAX");
//MF
cout<<endl<<"start number of bins is MINMAX_MAX: "<<nb<<endl;
//
	 TH1D minmaxHisto("minmax","minmax", nb, 0.0, (double)nb); 
	 vector<double> minmaxValues = aNoiseData->fMinMaxValues;

	 //fill the minmax histogram
	 for(uint binItr=0; binItr < minmaxValues.size(); binItr++)
	 {
	    minmaxHisto.Fill(minmaxValues[binItr]);
	 }

	 //Compute Cut value
         int i=0;
	 double ythresh = fUserData.GetDoubleParameter(detNum, "MINMAX_YTHRESH"); 
//MF
cout<<"ythresh: "<<ythresh<<endl;
//
	 double ymax = 0.0; // storage for maximum bin height
	 double maxbin = 0; //bin at which maximum occurs

	 //find the minimum value that is greater than 0
	 double minval = 999999.0;
	 for(uint checkItr=0; checkItr<minmaxValues.size(); checkItr++)
	 {
	    if(minmaxValues[checkItr] < minval) minval = minmaxValues[checkItr];
	 }
//MF
cout<<"minval, the minimum minmax in the randoms: "<<minval<<endl;
//

	 //if fewer than ythresh events in most populated bin, then rebin 
         while (ymax < ythresh && 2*minmaxHisto.GetBinWidth(i) < 128)
	 {
           nb = minmaxHisto.GetNbinsX();
	   
	   for(i=0;i<nb;i++){
		      
	      if (minmaxHisto.GetBinContent(i+1) > ymax) 
	      {
		 ymax = minmaxHisto.GetBinContent(i+1);
		 maxbin = (double) (i)*minmaxHisto.GetBinWidth(1); //note, no extra +1 bc value i falls in bin i+1
	      }
	   }

	   minmaxHisto.Rebin(2);
	 }
  
	 aNoiseData->fMinMaxCut = minval + fUserData.GetDoubleParameter(detNum, "MINMAX_NSIGMA")*(maxbin-minval) + 1; 
//MF
cout<<"minval: "<<minval<<"; maxbin: "<<maxbin<<"; cut threshold: "<<aNoiseData->fMinMaxCut<<endl;
//

      } //end loop over NoiseData for this zip

   } //end if detNum found in map	 

   return;
}

//apply minmax cut only on sum of phonon and sum of pulses
int NoiseBuilder::PassMinMaxCut(int detNum)
{
   int pass = 1;  //start out with failed case

   //MCF TEST
   cout<<"Check 1\n";

   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {

      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
   
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);

      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);

	 //we only cut on this value for PT and QT
	 if(aPulseData->GetChannelName() != "PT" && aPulseData->GetChannelName() != "QT") { continue; }

	 if(aPulseData->GetChannelName() == "QT" && ChannelMapHelper::GetNChargeChannels(aPulseData->GetDetectorType()) == 0) { continue; } // ******* MAT WUZ HERE ********

	 //make sure these objects correspond to the same detector and channel
	 if(aNoiseData->GetDetectorCode() != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::PassMinMaxCut ERROR in ordering of MapOfZipPulses or MapofNoisePulses!  Check code"
		 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode() 
		 << endl;

	    exit(1);
	 }
      
	 //Check whether this particular pulse passes or not
	 float minmax = PulseTools::MinMax(aPulseData->GetRawPulse()); 
	 
	 if(minmax >= aNoiseData->fMinMaxCut) { pass = 0; }

  	 cout << setprecision(15) <<"minmax = " << minmax
  	      <<"\ncutval = " << aNoiseData->fMinMaxCut
  	      << endl;

      } //end loop over pulses on this zip

   } //end if detNum found in map	 


   return pass;
}

//this is generated for each channel
void NoiseBuilder::ConstructPileUpDistribution(int detNum)
{
   
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {
      double maxPileup = -999999.; //only store the maxPileup out of all channels 
      
      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
   
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);
    
      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);
	 string     chanName = aPulseData->GetChannelName();

	 //do not construct pileup from non-physical channels (i.e. summed pulses or cross-talk)
	 if( !ChannelMapHelper::IsPhysicalChannel(chanName) )
	 { continue; }


	 //make sure these objects correspond to the same detector and channel
	 if(aNoiseData->GetDetectorCode() != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::ConstructPileUpDistrubution ERROR in ordering of MapOfZipPulses or MapofNoisePulses!  Check code"
                 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode()  
		 << endl;
	    exit(1);
	 }

	 //Compute PileUp and check against maxPileppppup
	 int nChunk = fUserData.GetIntParameter(detNum, "PILEUP_NCHUNK");
	 double pileup = PulseTools::PileUp(aPulseData->GetRawPulse(), nChunk); 

	 if(pileup > maxPileup) maxPileup = pileup;
        

      } //end loop over pulses on this zip


      //Loop again to histogram maxPileup value for each detector (same histogram is stored for all)
      for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
      {
	 NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);

	 //we do not store this for non physical
	 string chanName = aNoiseData->GetChannelName();
	 if( !ChannelMapHelper::IsPhysicalChannel(chanName))
	 { continue; }


	 //Initialize the histogram
	 //Note - these limits are arbitrary, this could be improved!! 
	 aNoiseData->InitializeNoiseSelectionHistogram("pileup", 10000, 0, (double)100);

	 //Store the value in the histogram
	 (aNoiseData->fPileupValues).push_back(maxPileup);
	 aNoiseData->HistogramValue("pileup", maxPileup); 
	 
      }

   } //end if detNum found in map	 

   return;
}

//apply this cut for each channel
void NoiseBuilder::CalcPileupCut(int detNum, bool isTFData)
{ 
   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(noiseMapItr != fMapOfNoiseData.end() )
   {
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);

      //loop over NoiseData
      for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
      {
	 NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
	 string     chanName = aNoiseData->GetChannelName();

	 //do not construct pileup from non-physical channels (i.e. summed pulses or cross-talk)
	 if( !ChannelMapHelper::IsPhysicalChannel(chanName) )
	 { continue; }

	 double pileupCut;

//	 isTFData = 1; //for debugging

	 //compute cut based on histogram of values if its TF data, otherwise use constant value from config file
	 if(isTFData)
	 {
	    //Compute Cut value
	    vector<double> pileupVect = aNoiseData->fPileupValues;
	    int nVal = pileupVect.size();
	    double maxPU = -999999.;

	    for(int vectItr=0; vectItr < nVal; vectItr++)
	    {
	       if(pileupVect[vectItr] > maxPU) maxPU = pileupVect[vectItr];
	    }

	    double nOverflow = 0;
	    pileupCut = maxPU;
	    
            //keep changing the upper limit on the histogram until the condition is satisfied
	    double pileupFrac = fUserData.GetDoubleParameter(detNum, "PILEUP_FRAC");
	    int    nbins = fUserData.GetIntParameter(detNum, "PILEUP_BINS");

	    while( (nOverflow/(double)nVal) < pileupFrac)
	    {
	       TH1D tempHist("pileup","pileup", nbins, 0, pileupCut); 

	       //fill the histogram w/ pileup values
	       for(int vectItr=0; vectItr < nVal; vectItr++)
	       {
		  tempHist.Fill(pileupVect[vectItr]);
	       }

	       nOverflow = tempHist.GetBinContent(nbins+1); 
	       
	       if((double)nOverflow/(double)nVal > pileupFrac) { break; } //so that we break out of the loop at the proper time
		  
	       pileupCut *= 0.5;  

	    }
	    
	 }
	 else
	 {
	    //get pileup from config
	    pileupCut = fUserData.GetDoubleParameter(detNum,"PILEUP_CUT"); 
	    
	 }

	 aNoiseData->fPileupCut = pileupCut;

//	 cout <<"pileup cut = " << pileupCut <<", on det = " << detNum << endl;

      } //end loop over pulses on this zip

   } //end if detNum found in map	 

   return;
}

//apply this cut for each channel
int NoiseBuilder::PassPileupCut(int detNum)
{
   int pass = 1;
 
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {

      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
   
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);

      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);
	 string     chanName = aPulseData->GetChannelName();

	 //do not construct pileup from non-physical channels (i.e. summed pulses or cross-talk)
	 if( !ChannelMapHelper::IsPhysicalChannel(chanName) )
	 { continue; }


	 //make sure these objects correspond to the same detector and channel
	 if(aNoiseData->GetDetectorCode() != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::PassMinMaxCut ERROR in ordering of MapOfZipPulses or MapofNoisePulses!  Check code"
		 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode() 
		 << endl;

	    exit(1);
	 }
      
	 int nChunk = fUserData.GetIntParameter(detNum, "PILEUP_NCHUNK");
	 double pileup = PulseTools::PileUp(aPulseData->GetRawPulse(), nChunk);  

	 if(pileup >= aNoiseData->fPileupCut) { pass = 0; }
	 
      } //end loop over pulses on this zip

   } //end if detNum found in map	 

//   cout <<"In PassPileupCut, pass val = " << pass << endl;

   return pass;
}

//this histograms the prepulse bs value for only the sum of phonon and the sum of charge pulses
void NoiseBuilder::ConstructPTBSslopeDistribution(int detNum)
{
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {
      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);

      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);
    
      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);

	 //we only store this for PT and QT
	 if(aPulseData->GetChannelName() != "PT" && aPulseData->GetChannelName() != "QT") { continue; }
	 if(aPulseData->GetChannelName() == "QT" && ChannelMapHelper::GetNChargeChannels(aPulseData->GetDetectorType()) == 0) { continue; }

	 //make sure these objects correspond to the same detector and channel
	 if(aNoiseData->GetDetectorCode() != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::ConstructPTBSslopeDistrubution ERROR in ordering of MapOfZipPulses or MapofNoisePulses!"
                 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode() 
		 << endl;
	    exit(1);
	 }

	 //Initialize the histogram
	 if (!fUserData.HasDoubleParameter(detNum, "PTBSslope_DIST_MAX")){
	   cout <<"NoiseBuilder::ConstructPTBSslopeDistrubution ERROR loading user parameter: " << "PTBSslope_DIST_MAX" << endl;
	   exit(1);
	 }

	 int PTBSslope_max = fUserData.GetDoubleParameter(detNum, "PTBSslope_DIST_MAX");
	 aNoiseData->InitializeNoiseSelectionHistogram("PTBSslope", 1000, -(double)PTBSslope_max, (double)PTBSslope_max); //should be positive by construction

	 //Compute BSslope
	 //use float to circumvent rounding issues for case of true integer minmax values
	 if (!fUserData.HasIntParameter(detNum, "P_BASELINE_MIN")){
	   cout <<"NoiseBuilder::ConstructPTBSslopeDistrubution ERROR loading user parameter: " << "P_BASELINE_MIN" << endl;
	   exit(1);
	 }
	 
	 if (!fUserData.HasIntParameter(detNum, "P_BASELINE_MAX")){
	   cout <<"NoiseBuilder::ConstructPTBSslopeDistrubution ERROR loading user parameter: " << "P_BASELINE_MAX" << endl;
	   exit(1);
	 }
	
	 if (!fUserData.HasIntParameter(detNum, "P_POSTBASELINE")){
	   cout <<"NoiseBuilder::ConstructPTBSslopeDistrubution ERROR loading user parameter: " << "P_POSTBASELINE" << endl;
	   exit(1);
	 }

	 int bin_min = fUserData.GetIntParameter(detNum, "P_BASELINE_MIN");
	 int bin_max = fUserData.GetIntParameter(detNum, "P_BASELINE_MAX");
	 int bin_post = fUserData.GetIntParameter(detNum, "P_POSTBASELINE");
	 
	 float BSslope = PulseTools::SlopedBaseline(aPulseData->GetRawPulse(),bin_min,bin_max,bin_post); //"RawPulse" = weighted sum for case of PT and QT 
	 
	 //cout <<"Calculating bs slope for channel " <<aPulseData->GetChannelName() <<" value = " << BSslope << " [ADC/bin]" <<  endl;

	 //Store the value in the histogram 
	 (aNoiseData->fPTBSslopeValues).push_back(BSslope);
	 aNoiseData->HistogramValue("PTBSslope", BSslope); 

      } //end loop over pulses on this zip

   } //end if detNum found in map	 

   return;
}

//apply PTBSslope cut only on sum of phonon and sum of pulses
//No fancy messing around with distributions, just have the user set hard cut values for now
int NoiseBuilder::PassPTBSslopeCut(int detNum)
{
   int pass = 1;  //start out with failed case

   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {

      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
   
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);

      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);

	 //we only cut on this value for PT and QT
	 if(aPulseData->GetChannelName() != "PT" && aPulseData->GetChannelName() != "QT") { continue; }
	 if(aPulseData->GetChannelName() == "QT" && ChannelMapHelper::GetNChargeChannels(aPulseData->GetDetectorType()) == 0) { continue; }

	 //make sure these objects correspond to the same detector and channel
	 if(aNoiseData->GetDetectorCode() != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::PassPTBSslopeCut ERROR in ordering of MapOfZipPulses or MapofNoisePulses!  Check code"
		 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode() 
		 << endl;

	    exit(1);
	 }
	 //Check whether this particular pulse passes or not
	 if (!fUserData.HasIntParameter(detNum, "P_BASELINE_MIN")){
	   cout <<"NoiseBuilder::PassPTBSslopeCut ERROR loading user parameter: " << "P_BASELINE_MIN" << endl;
	   exit(1);
	 }
	 
	 if (!fUserData.HasIntParameter(detNum, "P_BASELINE_MAX")){
	   cout <<"NoiseBuilder::PassPTBSslopeCut ERROR loading user parameter: " << "P_BASELINE_MAX" << endl;
	   exit(1);
	 }
	
	 if (!fUserData.HasIntParameter(detNum, "P_POSTBASELINE")){
	   cout <<"NoiseBuilder::PassPTBSslopeCut ERROR loading user parameter: " << "P_POSTBASELINE" << endl;
	   exit(1);
	 }
	 if (!fUserData.HasDoubleParameter(detNum, "PTBSslope_CUT_MIN")){
	   cout <<"NoiseBuilder::PassPTBSslopeCut ERROR loading user parameter: " << "PTBSslope_CUT_MIN" << endl;
	   exit(1);
	 }
	 if (!fUserData.HasDoubleParameter(detNum, "PTBSslope_CUT_MAX")){
	   cout <<"NoiseBuilder::PassPTBSslopeCut ERROR loading user parameter: " << "PTBSslope_CUT_MAX" << endl;
	   exit(1);
	 }



	 int bin_min = fUserData.GetIntParameter(detNum, "P_BASELINE_MIN");
         int bin_max = fUserData.GetIntParameter(detNum, "P_BASELINE_MAX");
         int bin_post = fUserData.GetIntParameter(detNum, "P_POSTBASELINE");
         float BSslope_min = fUserData.GetDoubleParameter(detNum, "PTBSslope_CUT_MIN");
         float BSslope_max = fUserData.GetDoubleParameter(detNum, "PTBSslope_CUT_MAX");

	 float BSslope = PulseTools::SlopedBaseline(aPulseData->GetRawPulse(),bin_min,bin_max,bin_post);
	 
	 
	 if( BSslope<=BSslope_min ||  BSslope>=BSslope_max ) { pass = 0; }
	
	 cout << "NoiseBuilder::PassPTBSslopeCut cutRange: (" << BSslope_min << ", " << BSslope_max << "), BSslope: " << BSslope << ", pass: " << pass << endl;
	 
      } //end loop over pulses on this zip

   } //end if detNum found in map	 


   return pass;
}


//this doesn't need to be checked on a detector-by-detector basis
//we are only doing this for legibility in BatNoise
int NoiseBuilder::PassRandomTriggerCut()
{
   int pass = 0;

   if(fRawReader.GetEventCategory() == 0x1)
   {
      pass = 1;
   }

   return pass;
}

//This used to be a check for digitizer clips in DarkPipe, but it was replaced in cdmsbats with
//this slightly more stringent requirement on the maximum size of the pulse
//if any pulse is saturated, discard the whole event for this detector
int NoiseBuilder::PassSaturationCut(int detNum)
{
   int pass = 1; //we start with pass

   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() )
   {
      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
   
      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 string sensorType = aPulseData->GetChannelType(); 
	 string channelName = aPulseData->GetChannelName();

	 double satVal = fUserData.GetIntParameter(detNum, ChannelMapHelper::GetChannelNameBase(channelName) + "_SATURATION");

	 //do not check this for PT and QT
	 if( !ChannelMapHelper::IsPhysicalChannel(channelName) ) { continue; }

	 if(PulseTools::IsSaturated(aPulseData->GetRawPulse(), satVal)) pass = 0;

      } //end loop over pulses on this zip

   } //end if detNum found in map
   
//   cout <<"In PassSaturationCut, pass value = " << pass << endl;

   return pass;
}


//this histograms the minmax value for only the sum of phonon and the sum of charge pulses
void NoiseBuilder::ConstructPOFchisqDistribution(int detNum)
{
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {
      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
   
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);
    
      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);


	 //we only store this for PT
	 if(aPulseData->GetChannelName() != "PT") { continue; }

	 //make sure these objects correspond to the same detector and channel
	 if(aNoiseData->GetDetectorCode() != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::ConstructPOFchiSqDistribution ERROR in ordering of MapOfZipPulses or MapofNoisePulses!"
                 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode() 
		 << endl;
	    exit(1);
	 }

	 //Initialize the histogram
	 aNoiseData->InitializeNoiseSelectionHistogram("POFchisq", 1000, 0.0, (double)100); //should be positive by construction

	 // =========  Perform Optimal Filter using white noise  =========
         
         double sampleRate = aNoiseData->fSampleRate;
         vector<double> pulse = aPulseData->GetRawPulse();
   
 	 // get Chi2
         double POFchisq = DoOptimalFilter(pulse,sampleRate);
    

	 //Store the value in the histogram 
	  (aNoiseData->fPOFchisqValues).push_back(POFchisq);
	  aNoiseData->HistogramValue("POFchisq", POFchisq); 

      } //end loop over pulses on this zip

   } //end if detNum found in map	 
   
   return;  
}




//apply minmax cut only on sum of phonon and sum of pulses
//the minmax distribution must be positive
void NoiseBuilder::CalcPOFchisqCut(int detNum)
{
   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(noiseMapItr != fMapOfNoiseData.end() )
   {
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);

      //loop over NoiseData objects
      for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
      {
	 NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);

	 //we only cut on this value for PT
	 if(aNoiseData->GetChannelName() != "PT") { continue; }

	 // Get vector of chi2
	 vector<double> POFchisqValues = aNoiseData->fPOFchisqValues;
   
         // sort vector
         sort(POFchisqValues.begin(),POFchisqValues.end());
              

         // find cut
         int percent = fUserData.GetIntParameter(detNum, "P_CHISQ_CUT_PERCENT"); 
         int binCut =(int) floor(POFchisqValues.size()*percent/100);
    
	 aNoiseData->fPOFchisqCut = POFchisqValues[binCut];


      } //end loop over NoiseData for this zip

   } //end if detNum found in map	 

   return;
}

//apply minmax cut only on sum of phonon and sum of pulses
int NoiseBuilder::PassPOFchisqCut(int detNum)
{

   int pass = 1;  //start out with failed case

   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {

      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
   
      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);

      //loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list)
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);

	 //we only cut on this value for PT
	 if(aPulseData->GetChannelName() != "PT") { continue; }

	 //make sure these objects correspond to the same detector and channel
	 if(aNoiseData->GetDetectorCode() != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::PassPOFchisqCut ERROR in ordering of MapOfZipPulses or MapofNoisePulses!  Check code"
		 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode() 
		 << endl;

	    exit(1);
	 }
      
         double sampleRate = aNoiseData->fSampleRate;
         vector<double> pulse = aPulseData->GetRawPulse();
   
 	 // get Chi2
         double POFchisq = DoOptimalFilter(pulse,sampleRate);
      
	 if(POFchisq >= aNoiseData->fPOFchisqCut) { pass = 0; }


      } //end loop over pulses on this zip

   } //end if detNum found in map	 


   return pass;
}




double NoiseBuilder::DoOptimalFilter(vector<double> pulse, double sampleRate)
{
  	 
    // =========  Perform Optimal Filter using white noise  =========
       
    double traceLength =  pulse.size();
     
  
    // Define OF window (-> use entire trace)
    int win1 = (int) traceLength;
    int win2 = (int) traceLength+1;
	    

    // Define Template 

    // For now, just a straight line
    vector<double> templateTime;
    for (int ibin=0; ibin<traceLength; ibin++) 
               templateTime.push_back(1.0); 


    // PSD (-> white noise for now)
    vector<double> noiseFFTsq = templateTime;


    // ---- Compute optimalFilter ----
          
    // With a straight line template, we would just need to 
    // compute sum(abs(pulseFFT)^2), however let's keep
    // the full code in case use a difference template, 
       

    vector<TComplex> templateFFT;
    PulseTools::RealToComplexFFT(templateTime,templateFFT);
   
    //normalize by sqrt(sampleRate)
    for(uint binCtr=0; binCtr < templateFFT.size(); binCtr++)
       {
         TComplex scalefactor(1.0/sqrt(sampleRate),0.0);
         templateFFT[binCtr] *= scalefactor;
       }


    vector<TComplex> optimalFilter;	 
    for(uint binCtr=0; binCtr<noiseFFTsq.size(); binCtr++)
	    optimalFilter.push_back(TComplex::Conjugate(templateFFT[binCtr])/noiseFFTsq[binCtr]);
	    
       	 
    // ---- compute normalization for amplitude estimator --------
	 
   double normFFT = 0.0;
   for(uint binCtr=0; binCtr < noiseFFTsq.size(); binCtr++)
	      normFFT += pow(TComplex::Abs(templateFFT[binCtr]),2)/noiseFFTsq[binCtr];
	
   double sigToNoiseSq = normFFT; 
	 normFFT *= 1.0/sqrt(noiseFFTsq.size()); // scale by 1/sqrt(N) for normFFT
	
   
         
   // ------- set OptimalFilterPhonon  parameters ---------

   OptimalFilterPhonon myOptimalFilter;

       
   myOptimalFilter.LoadTemplates(templateFFT, optimalFilter); 
   myOptimalFilter.LoadNormalizations(normFFT, sigToNoiseSq, noiseFFTsq); 
   myOptimalFilter.SetSampleTime(1.0/sampleRate);
   myOptimalFilter.SetWindows(win1,win2);

         
   // -------  do Optimal Filter ------------	 

   //Get Pulse and normalize if ISR file is being read;
   myOptimalFilter.DoCalc(pulse);
	    
   return myOptimalFilter.GetChisq();

 }








// ================== Noise Calclations  ========================

// all cuts are applied on an event and detector basis before calling this function
// all pulses for this detector will be added to the average PSD's
void NoiseBuilder::BuildAveragePSD(int detNum)
{
   //retrive the vector of pulses for this zip
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   if(pulseMapItr != fMapOfZipPulses.end() && noiseMapItr != fMapOfNoiseData.end() )
   {

      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);

      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);

      //store the detector type
      int detType = 0;  //will get stored multiple times, but better be the same for all pulses on this det

      //Construct sum of phonon pulses (again) with proper normalizations now.  
      //FIXME - someday we should only do this once, so we need to 
      //fix CalcSumOfPulses and carefully recheck the minmax routine - LLH 2/09
      vector<double> phononSumPulse;
      vector<double> phononSumS1Pulse;
      vector<double> phononSumS2Pulse;
      double phononSampleRate = 0.0;

      // Get list broken channels
      vector<string>  brokenPhononChannels;
      if (fUserData.DoRead("DET_STATUS_FILE"))
            brokenPhononChannels = fUserData.GetBrokenPhononChannelList(detNum);  


      // === loop over pulses, make use of ordering of NoiseData objects (should correpond to PulseData list) ===

      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);
	 NoiseData* aNoiseData = &((*noiseDataList)[pulseItr]);

	 //we compute an average psd for the channels (QI, QO, PA, PB, PC, PD, and PT) 
	 //no need to compute QT and we need to recalculate PT with correct normalization (different from minmax)
         //add PS1 and PS2 for iZIP only 
	 //FIXME - make the minmax normalization agree with this one someday when we have time to debug it carefully
	 //for QIX and QOX we store copies of the QI and QO average psd (this happens later)

	 string channelName = aNoiseData->GetChannelName();
	 detType            = aNoiseData->GetDetectorType();
	 int detCode        = aNoiseData->GetDetectorCode();

	 if( !ChannelMapHelper::IsPhysicalChannel(channelName) )
	 { continue; }

//	 cout <<"Building Noise PSD! " << channelName << endl;

	 //make sure these objects correspond to the same detector and channel
	 if(detCode != aPulseData->GetDetectorCode())
	 {
	    cout <<"NoiseBuilder::BuildAveragePSD ERROR in ordering of MapOfZipPulses or MapofNoisePulses!  Check code"
                 <<"noise data is " << aNoiseData->GetDetectorCode() <<", and pulse data is " << aPulseData->GetDetectorCode()  
		 << endl;

	    exit(1);
	 }

	 //get normalization for the pulse
         
	 double normalization = 1;	 
         string sensorType = aPulseData->GetChannelType(); 

	 if(sensorType == "phonon")
	 {
	   normalization = fDetConfigManager.GetPNormADCToAmps(detCode, fAdminData.GetEventTime());
	 }

	 if(sensorType == "charge")
	 {
	   normalization = fDetConfigManager.GetQNormADCToVolts(detCode, fAdminData.GetEventTime());
	 }
	 
         // get sampleRate
	 double sampleRate = fDetConfigManager.GetSampleRate(detCode);
	
  	 if(aPulseData->IsPhononPulse()) phononSampleRate = sampleRate; //storing this for PT

	 //construct normalized pulse and get PSD
	 vector<double> pulse = PulseTools::Normalize(aPulseData->GetRawPulse(), normalization);
	 vector<double> pulsePSD;
	 PulseTools::Time2PSD(pulse, sampleRate, pulsePSD);

	 //add this PSD to the average PSD
	 aNoiseData->AddToAveragePSD(pulsePSD);

	 //store the event number
	 (aNoiseData->fEventList).push_back(fAdminData.GetEvent());

	 //if its a phonon pulse, add it to PT so that we can later compute the averagePSD
	 if(aPulseData->IsPhononPulse())
	 {
            // skip broken channels
            if (find(brokenPhononChannels.begin(), brokenPhononChannels.end(), channelName) != brokenPhononChannels.end()) continue;

              
	    //include relative phonon scale factors (these are scaled relative to channel A)
            int detType = aPulseData->GetDetectorType();
	    double pulseCalib = fUserData.GetRelativeCalibration(detNum, detType, aPulseData->GetChannelName());

            // total phonon (sum of all channels)
	    if(phononSumPulse.size() == 0)
	       phononSumPulse = PulseTools::Scale(pulse, pulseCalib);   
	    else 
	       phononSumPulse = PulseTools::SumPulses(phononSumPulse, PulseTools::Scale(pulse, pulseCalib)); 	        

	
            
            // sum phonon channels on side 1 
            if (channelName.find("S1")!=string::npos) {
          
              if(phononSumS1Pulse.size() == 0)
	        phononSumS1Pulse = PulseTools::Scale(pulse, pulseCalib); //applying rough calibration factor
	      else 
	        phononSumS1Pulse = PulseTools::SumPulses(phononSumS1Pulse, PulseTools::Scale(pulse, pulseCalib)); 

	     }
	  
            // sum phonon channels on side 2
            if (channelName.find("S2")!=string::npos) {
          
             if(phononSumS2Pulse.size() == 0)
	        phononSumS2Pulse = PulseTools::Scale(pulse, pulseCalib); //applying rough calibration factor
	      else 
	       phononSumS2Pulse = PulseTools::SumPulses(phononSumS2Pulse, PulseTools::Scale(pulse, pulseCalib)); 

	     }
	    
         } // end phonon pulse

      } //end loop over pulses on this zip
      

      // === Now, loop over NoiseData list to construct the average PSD for the PT pulse (and PS1/PS2 for iZIP) ===

      for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
      {  
	 NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);

	 //constructing pulsePSD for PT
	 if(aNoiseData->GetChannelName() == "PT")
	 {
	    if(phononSampleRate == 0.0)
	    {
	       cout <<"NoiseBuilder::BuildAveragePSD ERROR! phononSampleRate not set, why isn't the noise PSD being calculated for phonons?"
		    << endl;
	       exit(1);
	    }

	    //use phononSumPulse to get PSD
	    vector<double> pulsePSD;
	    PulseTools::Time2PSD(phononSumPulse, phononSampleRate, pulsePSD);
	    
	    //add this PSD to the average PSD
	    aNoiseData->AddToAveragePSD(pulsePSD);
	 }


         //constructing pulsePSD for PS1
	 if(aNoiseData->GetChannelName() == "PS1")
	 {
	    if(phononSampleRate == 0.0)
	    {
	       cout <<"NoiseBuilder::BuildAveragePSD ERROR! phononSampleRate not set, why isn't the noise PSD being calculated for phonons?"
		    << endl;
	       exit(1);
	    }

	    //use phononSumPulse to get PSD
	    vector<double> pulsePSD;
	    PulseTools::Time2PSD(phononSumS1Pulse, phononSampleRate, pulsePSD);
	    
	    //add this PSD to the average PSD
	    aNoiseData->AddToAveragePSD(pulsePSD);
	 }


         //constructing pulsePSD for PS2
	 if(aNoiseData->GetChannelName() == "PS2")
	 {
	    if(phononSampleRate == 0.0)
	    {
	       cout <<"NoiseBuilder::BuildAveragePSD ERROR! phononSampleRate not set, why isn't the noise PSD being calculated for phonons?"
		    << endl;
	       exit(1);
	    }

	    //use phononSumPulse to get PSD
	    vector<double> pulsePSD;
	    PulseTools::Time2PSD(phononSumS2Pulse, phononSampleRate, pulsePSD);
	    
	    //add this PSD to the average PSD
	    aNoiseData->AddToAveragePSD(pulsePSD);
	 }

      } //end loop for calculating PSD for PT pulse


      // === loop over NoiseData objects again to copy the QI/QO averagePSD's into QIX/QOX averagePSD's ===
      //                            only for detetors with 2-channel cross talk!

      // CDMS2 and mZIP
      if( detType == BatRootTypes::kZIPDetType || detType == BatRootTypes::kMZIPDetType || detType == BatRootTypes::kCDMSliteSoudanI)  
      {
	 
	 //store the QI and QO averagePSD's so that we can copy them to QIX and QOX
	 vector<double> aQIAveragePSD;
	 vector<double> aQOAveragePSD;
	 
	 //This logic expects the physical channels to be stored in the list before cross-talk
	 bool aQIPSDStored = false;
	 bool aQOPSDStored = false;
	 
	 for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
	 {  
	    NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
	    
	    //store these so we can copy them to the cross-talk NoiseData later
	    if(aNoiseData->GetChannelName() == "QI") 
	    {
	       aQIAveragePSD = aNoiseData->fAveragePSD;
	       aQIPSDStored = true;
	    }	 
	    
	    if(aNoiseData->GetChannelName() == "QO")
	    { 
	       aQOAveragePSD = aNoiseData->fAveragePSD;
	       aQOPSDStored = true;
	    }
	    
	    //Finding cross-talk NoiseData objects, now store the PSD's
	    if(aNoiseData->GetChannelName() == "QIX")
	    {
	       if(aQIPSDStored)
		  aNoiseData->fAveragePSD = aQIAveragePSD;
	       else
	       {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR!  QIX NoiseData object appears before QI NoiseData object. "
		       <<"NoiseData objects appear to be out of order, please check initialization! "
		       << endl;
		  exit(1);
	       }
	    }
	    
	    if(aNoiseData->GetChannelName() == "QOX")
	    {
	       if(aQOPSDStored)
	       {
		  aNoiseData->fAveragePSD = aQOAveragePSD;
	       }
	       else
	       {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR!  QOX NoiseData object appears before QO NoiseData object. "
		       <<"NoiseData objects appear to be out of order, please check initialization! "
		       << endl;
		  exit(1);
	       }
	    }

	 }  //end loop for copying charge PSD's to cross-talk NoiseData

      } //end if appropriate detector type for 2-channel cross talk

   
      // now for iZIP
      if( detType==BatRootTypes::kiZIPSoudan || detType == BatRootTypes::kiZIPSNOlab)  
      {
	 
	 //store the QI/QO S1,S2 averagePSD's so that we can copy them to QIX and QOX (S1,S2)
	 vector<double> aQIS1AveragePSD;
	 vector<double> aQOS1AveragePSD;
         vector<double> aQIS2AveragePSD;
	 vector<double> aQOS2AveragePSD;

	 
	 //This logic expects the physical channels to be stored in the list before cross-talk
	 bool aQIS1PSDStored = false;
	 bool aQOS1PSDStored = false;
         bool aQIS2PSDStored = false;
	 bool aQOS2PSDStored = false;
	 
	 for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
	 {  
	    NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
	    
	    //store these so we can copy them to the cross-talk NoiseData later
	    if(aNoiseData->GetChannelName() == "QIS1") 
	    {
	       aQIS1AveragePSD = aNoiseData->fAveragePSD;
	       aQIS1PSDStored = true;
	    }	 
	    
	    if(aNoiseData->GetChannelName() == "QOS1")
	    { 
	       aQOS1AveragePSD = aNoiseData->fAveragePSD;
	       aQOS1PSDStored = true;
	    }
	    
            if(aNoiseData->GetChannelName() == "QIS2") 
	    {
	       aQIS2AveragePSD = aNoiseData->fAveragePSD;
	       aQIS2PSDStored = true;
	    }	 
	    
	    if(aNoiseData->GetChannelName() == "QOS2")
	    { 
	       aQOS2AveragePSD = aNoiseData->fAveragePSD;
	       aQOS2PSDStored = true;
	    }
	    



	    //Finding cross-talk NoiseData objects, now store the PSD's
	    if(aNoiseData->GetChannelName() == "QIS1X")
	    {
	       if(aQIS1PSDStored)
		  aNoiseData->fAveragePSD = aQIS1AveragePSD;
	       else
	       {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR!  QIS1X NoiseData object appears before QI NoiseData object. "
		       <<"NoiseData objects appear to be out of order, please check initialization! "
		       << endl;
		  exit(1);
	       }
	    }
	    
	    if(aNoiseData->GetChannelName() == "QOS1X")
	    {
	       if(aQOS1PSDStored)
	       {
		  aNoiseData->fAveragePSD = aQOS1AveragePSD;
	       }
	       else
	       {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR!  QOS1X NoiseData object appears before QO NoiseData object. "
		       <<"NoiseData objects appear to be out of order, please check initialization! "
		       << endl;
		  exit(1);
	       }
	    }

	    if(aNoiseData->GetChannelName() == "QIS2X")
	    {
	       if(aQIS2PSDStored)
		  aNoiseData->fAveragePSD = aQIS2AveragePSD;
	       else
	       {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR!  QIS2X NoiseData object appears before QI NoiseData object. "
		       <<"NoiseData objects appear to be out of order, please check initialization! "
		       << endl;
		  exit(1);
	       }
	    }
	    
	    if(aNoiseData->GetChannelName() == "QOS2X")
	    {
	       if(aQOS2PSDStored)
	       {
		  aNoiseData->fAveragePSD = aQOS2AveragePSD;
	       }
	       else
	       {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR!  QOS2X NoiseData object appears before QO NoiseData object. "
		       <<"NoiseData objects appear to be out of order, please check initialization! "
		       << endl;
		  exit(1);
	       }
	    }


	 }  //end loop for copying charge PSD's to cross-talk NoiseData

      } //end if appropriate detector type for iZIP
   
   

      // loop again noise to add  Average PSD for glitches/LF noise templates
      // and the other extra noise data
        
      
       if (fUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhononGlitch1")) {
      
          // using PT
          vector<double> aPTAveragePSD;
 	  bool aPTPSDStored = false;
	 
       	  for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
	   {  
	       NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
	    
	       //store PT (should come before PTgltich1 in the noise data list)
	       if(aNoiseData->GetChannelName() == "PT") 
	        {
	          aPTAveragePSD = aNoiseData->fAveragePSD;
	          aPTPSDStored = true;
	        }	 
	    	    
	       // find PTglitch1
	       if(aNoiseData->GetChannelName() == "PTglitch1")
	        {
	         if(aPTPSDStored)
		   aNoiseData->fAveragePSD = aPTAveragePSD;
	         else
	           {
		     cout <<"\nNoiseBuilder::BuildAveragePSD ERROR!  PTgltich1 NoiseData object appears before PT NoiseData object. "
		          <<"NoiseData objects appear to be out of order, please check initialization! "
		          << endl;
		     exit(1);
	           }
	        }
  
	    } // loop noise

      } // PTglitch1 
	    

      if (fUserData.DoAlgorithm(detNum, "PT", "OptimalFilterPhononLFnoise1")) {
      
          // using PT
          vector<double> aPTAveragePSD;
 	  bool aPTPSDStored = false;
	 
       	  for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
	   {  
	       NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
	    
	       //store PT (should come before PTlfnoise1 in the noise data list)
	       if(aNoiseData->GetChannelName() == "PT") 
	        {
	          aPTAveragePSD = aNoiseData->fAveragePSD;
	          aPTPSDStored = true;
	        }	 
	    	    
	       // find PTlfnoise1
	       if(aNoiseData->GetChannelName() == "PTlfnoise1")
	        {
	         if(aPTPSDStored)
		   aNoiseData->fAveragePSD = aPTAveragePSD;
	         else
	           {
		     cout <<"\nNoiseBuilder::BuildAveragePSD ERROR!  PTlfnoise1 NoiseData object appears before PT NoiseData object. "
		          <<"NoiseData objects appear to be out of order, please check initialization! "
		          << endl;
		     exit(1);
	           }
	        }
  
	   } // loop noise

      } // PTlfnoise1 

   
      // 2D OF 
      if (fUserData.DoAlgorithm(detNum, "phonon", "OptimalFilterPhonon1X2")) {
	
	map<string,vector<double> > noisePSDmap;
	for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
	  {  
	    NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
	    
	    string channelName = aNoiseData->GetChannelName();
	    string channelType = ChannelMapHelper::GetChannelType(channelName);
	    
	    if(channelType != "phonon")
	      continue;
	    
	    // fill map
	    if(ChannelMapHelper::IsPhysicalChannel(channelName) ||
	       channelName == "PT" ||
	       channelName == "PS1" ||
	       channelName == "PS2") 
	      {
		vector<double> averagePSD = aNoiseData->fAveragePSD;
		noisePSDmap.insert( pair<string,vector<double> >(channelName,averagePSD));
	      }
	    
	    
	    // Relying  on the order of noiseDataList, we can now add PSD to 
	    // noise data as the map should be filled
	    
	    // Slow Template
	    size_t posSlow = channelName.find("slow");  
	    
	    if (posSlow != string::npos) 
	      {
		string channelNameMap = channelName.substr(0,posSlow);
		map<string, vector<double> >::iterator noisePSDmapItr = noisePSDmap.find(channelNameMap);
		if (noisePSDmapItr!=noisePSDmap.end()) {
		  aNoiseData->fAveragePSD = noisePSDmap[channelNameMap];
		} else {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR! " << channelName 
		       << " NoiseData object appears to be out of order. "
		       <<" Please check initialization! "
		     << endl;
		  exit(1);
		}
	      }
	    
	      
	    // Fast template
	    size_t posFast = channelName.find("fast");  
	    if (posFast != string::npos) 
	      {
		string channelNameMap = channelName.substr(0,posFast);
		map<string, vector<double> >::iterator noisePSDmapItr = noisePSDmap.find(channelNameMap);
		if (noisePSDmapItr !=noisePSDmap.end()) {
		  aNoiseData->fAveragePSD = noisePSDmap[channelNameMap];
		} else {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR! " << channelName 
		       << " NoiseData object appears to be out of order. "
		       <<" Please check initialization! "
		       << endl;
		  exit(1);
		}
	      }

	  } // loop noise list
      } // end if 2DOF
   
      


      // DMC tenplate
      if (fUserData.DoAlgorithm(detNum, "phonon", "OptimalFilterPhononDMC")) {
	
	map<string,vector<double> > noisePSDmap;
	for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
	  {  
	    NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
	    
	    string channelName = aNoiseData->GetChannelName();
	    string channelType = ChannelMapHelper::GetChannelType(channelName);
	    
	    if(channelType != "phonon")
	      continue;
	    
	    // fill map
	    if(ChannelMapHelper::IsPhysicalChannel(channelName) ||
	       channelName == "PT" ||
	       channelName == "PS1" ||
	       channelName == "PS2") 
	      {
		vector<double> averagePSD = aNoiseData->fAveragePSD;
		noisePSDmap.insert( pair<string,vector<double> >(channelName,averagePSD));
	      }
	    
	    
	    // Relying  on the order of noiseDataList, we can now add PSD to 
	    // noise data as the map should be filled
	    
	    size_t pos = channelName.find("dmc");  
	    if (pos != string::npos) 
	      {
		string channelNameMap = channelName.substr(0,pos);
		map<string, vector<double> >::iterator noisePSDmapItr = noisePSDmap.find(channelNameMap);
		if (noisePSDmapItr !=noisePSDmap.end()) {
		  aNoiseData->fAveragePSD = noisePSDmap[channelNameMap];
		} else {
		  cout <<"\nNoiseBuilder::BuildAveragePSD ERROR! " << channelName 
		       << " NoiseData object appears to be out of order. "
		       <<" Please check initialization! "
		       << endl;
		  exit(1);
		}
	      }

	  } // loop noise list
      } // end if dmc
   
      


	
   // === DONE! ===

   } //end if detNum found in map	 

   return;
}

// computes covariance matrix elements between QI and QO noise pulses
// all cuts are applied on an event and detector basis before calling this function
// all pulses for this detector will be added to the average 
void NoiseBuilder::BuildQIQOCov(int detNum)
{

   double sampleRate = 0;

   //store the QI and QO pulses to compute the covariance 
   vector<double> aQIPulse;
   vector<double> aQOPulse;


   // --- retrive the vector of pulses for this zip ---
   vector<PulseData>* zipPulseList;
   map< int, vector<PulseData> >::iterator pulseMapItr = fMapOfZipPulses.find(detNum);

   if(pulseMapItr != fMapOfZipPulses.end())
   {

      //Get the pulse list for this zip
      zipPulseList = &(pulseMapItr->second);
    
      //loop over pulses and stash the QI/QO noise traces for calculation of covariance
      for(uint pulseItr = 0; pulseItr < zipPulseList->size(); pulseItr++)
      {
	 PulseData* aPulseData = &((*zipPulseList)[pulseItr]);

	 //Currently this routine only works for 2-charge channel ZIPS.  It may be updated
	 //for the future to run on iZIPs.  For now inserting a check that its a valid ZIP
	 //type and exit if not
	 if(aPulseData->GetDetectorType() != BatRootTypes::kZIPDetType &&
	    aPulseData->GetDetectorType() != BatRootTypes::kMZIPDetType)
	 {
	    cout <<"\nERROR! NoiseBuilder::BuildQIQOCov  Covariance calculation not yet implemented for this detector type."
		 <<"  Deactivate DO_CHARGE_ALGORITHM: ChargeNoiseCovariance in the processing config until properly implemented."
		 << endl;
	    exit(1);
	 }

	 //only interested in QI and QO pulses
	 if(aPulseData->GetChannelName() != "QI" && aPulseData->GetChannelName() != "QO")
	 {  continue; }

	 //get normalization for the pulse
         
	 //	 double normalization = 1;	 

         string sensorType  = aPulseData->GetChannelType(); 
	 string channelName = aPulseData->GetChannelName();
	 int    detCode     = aPulseData->GetDetectorCode();
	 sampleRate         = fDetConfigManager.GetSampleRate(detCode); //assumes sample rate is same for all charge chan
	 	 
	 double normalization = fDetConfigManager.GetQNormADCToVolts(detCode, fAdminData.GetEventTime());
	
	 //construct normalized pulse and get PSD
	 if(aPulseData->GetChannelName() == "QI")
	 {
	    aQIPulse = PulseTools::Normalize(aPulseData->GetRawPulse(), normalization);
	 }

	 if(aPulseData->GetChannelName() == "QO")
	 {
	    aQOPulse = PulseTools::Normalize(aPulseData->GetRawPulse(), normalization);
	 }	 


      } //end loop over pulses on this zip
      
   } //end if detNum found in map	 

   
   // --- Next calculate the covariance and store it in CorrelationData
   
   // Compute the Correlation
   vector<double> qiqoCov_Re;
   vector<double> qiqoCov_Im;

   PulseTools::Time2FFTCov(aQIPulse, aQOPulse, sampleRate, qiqoCov_Re, qiqoCov_Im);

   
   //Retrive the vector of CorrelationData for this detector
   vector<CorrelationData>* correlationDataList;
   map< int, vector<CorrelationData> >::iterator correlationMapItr = fMapOfCorrelationData.find(detNum);

   //Check that a list is found for this detector
   if(correlationMapItr != fMapOfCorrelationData.end())
   {
      
      //Get the CorrelationData list for this detector
      correlationDataList = &(correlationMapItr->second);

      //Loop over CorrelationData in list and look for match to QIQO
      for(uint corrItr = 0; corrItr < correlationDataList->size(); corrItr++)
      {
	 
	 CorrelationData* aCorrelationData = &((*correlationDataList)[corrItr]);
	 
	 //make sure these objects correspond to the same detector and channels
	 if(aCorrelationData->GetDetectorNum() != detNum || !aCorrelationData->IsChannelMatch("QI", "QO"))
	 {
	    cout <<"CorrelationBuilder::BuildQIQOCov ERROR in configuration CorrelationData map !  Check code!"
		 <<"\nerror encountered on detector " << aCorrelationData->GetDetectorNum() 
		 <<"\nfor channels " << aCorrelationData->GetChannelNames() 
		 << endl;
	    
	    exit(1);
	 }
      
	 //Found a match, now add it to the average covariance
	 aCorrelationData->AddToAverageCov(qiqoCov_Re, qiqoCov_Im);
      

      } //done looping over CorrelationData list for this detector

   } //end if found correlation list correponding to this detector


   return;
}

//====================================================================================

//This function calls one of two functions: CalcNoiseWithCrossTalk or CalcNoiseNoCrossTalk
//The former only works on 2-charge channel zips at the moment.   It may be upgraded in the
//future for more than 2-charge channel zips, but at the moment we do not have need for this
//becuase iZIPs to no exhibit significant cross-talk.
void NoiseBuilder::CalcNoiseQuantities(const int detNum, const int totEvtCtr)
{

   //retrive the vector of NoiseData for this zip
   vector<NoiseData>* noiseDataList;
   map< int, vector<NoiseData> >::iterator noiseMapItr = fMapOfNoiseData.find(detNum);
   
   //check if the vector was found
   if(noiseMapItr != fMapOfNoiseData.end() )
   {

      //Get the noise list for this zip
      noiseDataList = &(noiseMapItr->second);
      
      //access first noise data object in list to get the detector type
      NoiseData* aNoiseData = &((*noiseDataList)[0]);
      int        detType = aNoiseData->GetDetectorType();
 
      //choose which algorithm to follow based on whether there's a 2-channel cross talk
      //calculation to do or not.  If not, then generated uncorrelated noise and PSD info

      if(detType == BatRootTypes::kZIPDetType || detType == BatRootTypes::kMZIPDetType ||
         detType == BatRootTypes::kCDMSliteSoudanI) 
      {
	CalcNoiseWithCrossTalk(noiseDataList, detNum, totEvtCtr);
      }	
      else if(detType==BatRootTypes::kiZIPSoudan || detType==BatRootTypes::kiZIPSNOlab) 
      {
         CalcNoiseWithCrossTalk(noiseDataList, detNum, totEvtCtr,"S1");
         CalcNoiseWithCrossTalk(noiseDataList, detNum, totEvtCtr,"S2");
      } 
      else 
      {
	 CalcNoiseNoCrossTalk(noiseDataList, detNum, totEvtCtr);
      }


   } //end if detNum found in map	 


   return;
}

//This computes cross talk quantities for zips that have two charge channels as well as noise quantities
//for phonon pulses
void NoiseBuilder::CalcNoiseWithCrossTalk(vector<NoiseData>* noiseDataList, const int detNum, const int totEvtCtr,const string& side)
{

   // channels names
   string chanNameQI = "QI";
   string chanNameQO = "QO";
   string chanNameQIX = "QIX";
   string chanNameQOX = "QOX";
   
   // check argument and add S1 or S2 channel name
   if (side == "S1" || side == "S2") {

     chanNameQI = "QI"+side;
     chanNameQO = "QO"+side;
     chanNameQIX = "QI"+side +"X";
     chanNameQOX = "QO"+side +"X";


   } else if (!side.empty()) {
      cout <<"\nERROR! NoiseBuilder::CalcNoiseWithCrossTalk:  Argument side must be 'S1', 'S2', or empty string"
		 << endl;
      exit(1);
   }


   bool isEmptyPSD = false;
   bool isFirstChannel = true;

   //Store these NoiseData objects so that we can use them later to calculate the cross-talk matrix
   NoiseData* aQINoiseData = NULL;
   NoiseData* aQONoiseData = NULL;
   NoiseData* aQIXNoiseData = NULL;
   NoiseData* aQOXNoiseData = NULL;
   
   //loop over NoiseData
   for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
   {
      NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
      
      //this should be the same for all channels that have an average PSD
      if(isFirstChannel)
      {
	 cout << aNoiseData->fPSDCount <<" events out of " 
	      << totEvtCtr <<" selected as noise for detector " << detNum 
	      << endl;

	 isFirstChannel = false;
      }
      
      //do not perform this calculation on QT becuase we don't need it
      if(aNoiseData->GetChannelName() == "QT") { continue; }
      
      vector<double> averagePSD = aNoiseData->fAveragePSD;
      
      //should we just let the code exit if the size is 0 instead of issuing a warning? 
      if(averagePSD.size() != 0)
      {
	 // === store the charge NoiseData objects for matrix calculation ===
	 
	 if(aNoiseData->GetChannelName() == chanNameQI) aQINoiseData = aNoiseData;
	 if(aNoiseData->GetChannelName() == chanNameQO) aQONoiseData = aNoiseData;
	 if(aNoiseData->GetChannelName() == chanNameQIX) aQIXNoiseData = aNoiseData;
	 if(aNoiseData->GetChannelName() == chanNameQOX) aQOXNoiseData = aNoiseData;
	 
	 // === compute and store noiseFFT ===
	 int originalTraceLengthType = aNoiseData->GetTraceLengthType();

	 vector<double> tempNoiseFFT;
	 PulseTools::PSD2FFT(averagePSD, tempNoiseFFT,originalTraceLengthType);
	 aNoiseData->fNoiseFFT = tempNoiseFFT;
	 
	 // === compute and store noiseFFTsq ===
	 
	 vector<double> tempNoiseFFTsq;
	 for(uint binCtr=0; binCtr<tempNoiseFFT.size(); binCtr++)
	 {
	    tempNoiseFFTsq.push_back(tempNoiseFFT[binCtr]*tempNoiseFFT[binCtr]);
	 }
	 aNoiseData->fNoiseFFTsq = tempNoiseFFTsq;
	 
	 // === compute and store optimalFilter ===
	 
	 vector<TComplex> tempOptimalFilter;
	 vector<TComplex> templateFFT = aNoiseData->fTemplateFFT;
	 
	 if(templateFFT.size() != tempNoiseFFTsq.size())
	 {
            //cout << "With X talk" << endl;
            //cout << "Channel " << aNoiseData->GetChannelName() << endl;
            //cout << "templateFFT.size() = " << templateFFT.size() << endl;
            //cout << "templateFFTsq.size() = " << tempNoiseFFTsq.size() << endl;
	    cout <<"NoiseBuilder::CalcNoiseQuantities ERROR! pulse template FFT and noiseFFTsq have different number of bins!"
		 << endl;
	    exit(1);
	 }
	 
	 //setting DC component to zero - be careful not to divide by this later!!
	 TComplex dc(0.0, 0.0);
	 tempOptimalFilter.push_back(dc);
	 
	 for(uint binCtr=1; binCtr<tempNoiseFFT.size(); binCtr++)
	 {
	    tempOptimalFilter.push_back(TComplex::Conjugate(templateFFT[binCtr])/tempNoiseFFTsq[binCtr]);
	    
// 	       cout <<"bin = " << binCtr 
// 		    <<"\nOFval = " << tempOptimalFilter[binCtr] 
// 		    <<", templateFFT val = " << templateFFT[binCtr]
// 		    <<", noiseFFTsq val = " << tempNoiseFFTsq[binCtr]
// 		    << endl;
	 }
	 
	 aNoiseData->fOptimalFilter = tempOptimalFilter;
	 
	 
	 // === compute and store normalization for amplitude estimator ===
	 
	 double tempNormFFT = 0.0;
	 
	 //compute sum, ignoring the DC component
	 for(uint binCtr=1; binCtr < tempNoiseFFTsq.size(); binCtr++)
	 {
	    tempNormFFT += pow(TComplex::Abs(templateFFT[binCtr]),2)/tempNoiseFFTsq[binCtr];
	 }
	 
	 // Note, normFFT and sigToNoiseSq are really the same quantity except for 1/sqrt(N) factor
	 // we're storing both for legacy reasons now (they must be matched with terms that have 
	 // the correct sqrt(N) factors in the numerator).  This can be cleaned up in the future.
	 aNoiseData->fSigToNoiseSq = tempNormFFT; 
	 
	 tempNormFFT *= 1.0/sqrt(tempNoiseFFTsq.size()); // scale by 1/sqrt(N) for normFFT
	 aNoiseData->fNormFFT = tempNormFFT; 
	 
      } //end if averagePSD.size() != 0
      else
      {
	 cout <<"\nNoiseBuilder::CalcNoiseQuantities WARNING!  AveragePSD is zero."
	      <<"\nNo noise pulses were found for detector " << detNum <<" channel " << aNoiseData->GetChannelName()
	      <<" did you set the proper cut values?"
	      << endl;
	 
	 isEmptyPSD = true;
      }
      
   } //end loop over NoiseData for this zip
   
   if(isEmptyPSD) return; //don't continue with cross talk calculation if PSD's are empty
   
   //make sure we have found all the charge channels
   if((aQINoiseData == NULL || aQONoiseData == NULL || aQIXNoiseData == NULL || aQOXNoiseData == NULL))
   {
      cout <<"NoiseBuilder::CalcNoiseQuantities ERROR!  Missing NoiseData object for charge or charge cross talk channel on det "
	   << detNum
	   <<". Please check the NoiseBuilder configuration!"
	   << endl;
      
      exit(1);
   }
   
   // === compute Qinverse ===
   
   //some checks that the noise quantities are stored in vectors of the same length between channels
   if(aQINoiseData->fTemplateFFT.size() != aQONoiseData->fTemplateFFT.size() ||
      aQINoiseData->fTemplateFFT.size() != aQIXNoiseData->fTemplateFFT.size() ||
      aQINoiseData->fTemplateFFT.size() != aQOXNoiseData->fTemplateFFT.size())
   {
      cout <<"NoiseBuilder::CalcNoiseQuantities ERROR! The lengths of the pulse templates do not match between channels!  "
	   <<"We cannot compute qInverse this way!"
	   << endl;
      
      exit(1);	 
   }

   double matrix00 = 0.0;
   double matrix11 = 0.0;
   double matrix01 = 0.0;
   
   //ignore the DC component
   for(uint binCtr=1; binCtr < aQINoiseData->fTemplateFFT.size(); binCtr++)
   {
      matrix00 += pow(TComplex::Abs(aQINoiseData->fTemplateFFT[binCtr]),2)/aQINoiseData->fNoiseFFTsq[binCtr] 
	 + pow(TComplex::Abs(aQOXNoiseData->fTemplateFFT[binCtr]),2)/aQONoiseData->fNoiseFFTsq[binCtr] ;
      
      matrix11 += pow(TComplex::Abs(aQONoiseData->fTemplateFFT[binCtr]),2)/aQONoiseData->fNoiseFFTsq[binCtr]  
	 + pow(TComplex::Abs(aQIXNoiseData->fTemplateFFT[binCtr]),2)/aQINoiseData->fNoiseFFTsq[binCtr] ;
      
      TComplex tempVal = (aQIXNoiseData->fTemplateFFT[binCtr])
	 *(TComplex::Conjugate(aQINoiseData->fTemplateFFT[binCtr]))/aQINoiseData->fNoiseFFTsq[binCtr]; 
      tempVal += (aQOXNoiseData->fTemplateFFT[binCtr])
	 *(TComplex::Conjugate(aQONoiseData->fTemplateFFT[binCtr]))/aQONoiseData->fNoiseFFTsq[binCtr];
      
      matrix01 += tempVal.Re();
   }

   //if the charge channel is obviously railed, isn't it just better to warn instead of fail? [ANV]
   if((matrix00==0 || matrix00 != matrix00) && (matrix11==0 || matrix11 != matrix11) && (matrix01==0 || matrix01 != matrix01)){
      cout <<"NoiseBuilder::CalcNoiseQuantities WARNING! one or more charge channels on detector " << aQINoiseData->GetDetectorNum() << " appears to be railed, setting crosstalk matrix to identity." << endl;
      matrix00=1.0;
      matrix11=1.0;
      matrix01=0.0;
   }
   
   TMatrixD chisqMatrix(2,2);
   chisqMatrix[0][0] = matrix00;
   chisqMatrix[0][1] = matrix01;
   chisqMatrix[1][0] = matrix01; //symmetric matrix
   chisqMatrix[1][1] = matrix11;
   
   //check whether the matrix is invertable
   if(chisqMatrix.Determinant() == 0)
   {
      cout <<"NoiseBuilder::CalcNoiseQuantities ERROR!  Problem computing qInverse, matrix is not invertable!"
	   <<"\nchisqMatrix = " << chisqMatrix[0][0] <<" " << chisqMatrix[0][1]
	   <<"\n              " << chisqMatrix[1][0] <<" " << chisqMatrix[1][1]
	   << endl;
      
      cout << "NoiseBuilder::CalcNoiseQuantities INFO: the values of some parameters  for the error above are: " << endl;
      cout << "matrix00: " << matrix00 << endl;
      cout << "matrix01: " << matrix01 << endl;
      cout << "matrix11: " << matrix11 << endl;
      exit(1);
   }
   
   TMatrixD qInverse = chisqMatrix;
   qInverse.Invert(); 
   
   //storing duplicate in all charge noise data
   aQINoiseData->fQInverse = (TMatrixD)qInverse;
   aQONoiseData->fQInverse = (TMatrixD)qInverse;
   aQIXNoiseData->fQInverse = (TMatrixD)qInverse;
   aQOXNoiseData->fQInverse = (TMatrixD)qInverse;

   return;
}


//This is a simpler calculation for the case of no cross-talk between any channels
void NoiseBuilder::CalcNoiseNoCrossTalk(vector<NoiseData>* noiseDataList, const int detNum, const int totEvtCtr)
{

   bool isFirstChannel = true;

   //loop over NoiseData collection

   for(uint noiseItr = 0; noiseItr < noiseDataList->size(); noiseItr++)
   {
      NoiseData* aNoiseData = &((*noiseDataList)[noiseItr]);
      
      //this should be the same for all channels that have an average PSD so just display first
      if(isFirstChannel) 
      {
	 cout << aNoiseData->fPSDCount <<" events out of " 
	      << totEvtCtr <<" selected as noise for detector " << detNum 
	      << endl;

	 isFirstChannel = false;
      }
      
      // channel Name
      string chanName = aNoiseData->GetChannelName();

      //do not perform this calculation on QT becuase we don't need it
      if(chanName.compare("QT")==0) { continue; }
      if(chanName.find("X")!= string::npos) { continue; }
           
      vector<double> averagePSD = aNoiseData->fAveragePSD;
      
      //should we just let the code exit if the size is 0 instead of issuing a warning? 
      if(averagePSD.size() != 0)
      {
	 
	 // === compute and store noiseFFT ===
	 int originalTraceLengthType = aNoiseData->GetTraceLengthType();
	 
	 vector<double> tempNoiseFFT;
	 PulseTools::PSD2FFT(averagePSD, tempNoiseFFT,originalTraceLengthType);
	 aNoiseData->fNoiseFFT = tempNoiseFFT;
	 
	 // === compute and store noiseFFTsq ===
	 
	 vector<double> tempNoiseFFTsq;
	 for(uint binCtr=0; binCtr<tempNoiseFFT.size(); binCtr++)
	 {
	    tempNoiseFFTsq.push_back(tempNoiseFFT[binCtr]*tempNoiseFFT[binCtr]);
	 }
	 aNoiseData->fNoiseFFTsq = tempNoiseFFTsq;
	 
	 // === compute and store optimalFilter ===
	 
	 vector<TComplex> tempOptimalFilter;
	 vector<TComplex> templateFFT = aNoiseData->fTemplateFFT;
	 
	 if(templateFFT.size() != tempNoiseFFTsq.size())
	 {
            //cout << "Without X talk" << endl;
            //cout << "Channel " << aNoiseData->GetChannelName() << endl;
            //cout << "templateFFT.size() = " << templateFFT.size() << endl;
            //cout << "templateFFTsq.size() = " << tempNoiseFFTsq.size() << endl;
	    cout <<"NoiseBuilder::CalcNoiseQuantities ERROR! pulse template FFT and noiseFFTsq have different number of bins!"
		 << endl;
	    cout <<templateFFT.size()<<" and "<<tempNoiseFFTsq.size()<<" bins respectively."<<endl;
	    exit(1);
	 }
	 
	 //setting DC component to zero - be careful not to divide by this later!!
	 TComplex dc(0.0, 0.0);
	 tempOptimalFilter.push_back(dc);
	 
	 for(uint binCtr=1; binCtr<tempNoiseFFT.size(); binCtr++)
	 {
	    tempOptimalFilter.push_back(TComplex::Conjugate(templateFFT[binCtr])/tempNoiseFFTsq[binCtr]);
	    
// 	       cout <<"bin = " << binCtr 
// 		    <<"\nOFval = " << tempOptimalFilter[binCtr] 
// 		    <<", templateFFT val = " << templateFFT[binCtr]
// 		    <<", noiseFFTsq val = " << tempNoiseFFTsq[binCtr]
// 		    << endl;
	 }
	 
	 aNoiseData->fOptimalFilter = tempOptimalFilter;
	 
	 
	 // === compute and store normalization for amplitude estimator ===
	 
	 double tempNormFFT = 0.0;
	 
	 //compute sum, ignoring the DC component
	 for(uint binCtr=1; binCtr < tempNoiseFFTsq.size(); binCtr++)
	 {
	    tempNormFFT += pow(TComplex::Abs(templateFFT[binCtr]),2)/tempNoiseFFTsq[binCtr];
	 }
	 
	 // Note, normFFT and sigToNoiseSq are really the same quantity except for 1/sqrt(N) factor
	 // we're storing both for legacy reasons now (they must be matched with terms that have 
	 // the correct sqrt(N) factors in the numerator).  This can be cleaned up in the future.
	 aNoiseData->fSigToNoiseSq = tempNormFFT; 
	 
	 tempNormFFT *= 1.0/sqrt(tempNoiseFFTsq.size()); // scale by 1/sqrt(N) for normFFT
	 aNoiseData->fNormFFT = tempNormFFT; 
	 
      } //end if averagePSD.size() != 0
      else
      {
	 cout <<"\nNoiseBuilder::CalcNoiseQuantities WARNING!  AveragePSD is zero."
	      <<"\nNo noise pulses were found for detector " << detNum <<" channel " << aNoiseData->GetChannelName()
	      <<" did you set the proper cut values?"
	      << endl;
      }
      
   } //end loop over NoiseData for this zip
   

   return;
}


// ================== Read-only access to Pulse Collections  ========================


//This function gives read-only access to the pulse values!
void NoiseBuilder::FillSingleZipPulseCollection(vector<PulseData>& pulseCollection,
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



