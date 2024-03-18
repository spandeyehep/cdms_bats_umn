///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: BatCalibIOManager
//Authors: L. Hsu, B. Serfass
//Description:  This class controls the generation of the output files and the reading of the input files.   It defines and manages 
//a series of maps which contain the input RQ's and their values.  This class borrows heavily from the output file generating class in BatRoot (BatOutputManager).
//
//File Import By: L. Hsu
//Creation Date: Dec. 23, 2009
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include "time.h"
#include <sys/stat.h>

#include "TError.h"
#include "TKey.h"
#include "TLeaf.h"

#include "BatCalibIOManager.h"
#include "BatCalibTypes.h"


using namespace std;

// ======================================================================

BatCalibIOManager::BatCalibIOManager(const UserDataManager& myUserData, const string& inputSeries, 
				     const string& dumpName)  :
    fDebugOn(false),
    fInputSeries(inputSeries),
    fFileWriter(NULL),
    fOptions(myUserData),
    fActiveReadTree(NULL),
    fOutputRRQTree(NULL)
{

   // --- construct full pathname to the RQ file ---

   string inputFilename;
   int isFound;
   struct stat buffer ;

   //check whether user wants to run on "merged" or "unmerged" files
   //for now unmerged handling is to make a single output per input file since CAP
   //can't deal with merged rrq and unmerged rq files
   if(fOptions.GetIntParameter("USE_MERGED_RQ") == 1)
   {
      inputFilename = Form("%s_%s.root", fOptions.GetPrefix("RQ_DATA_PREFIX").c_str(), fInputSeries.c_str());
   }
   else
   {
      inputFilename = Form("%s_%s_%s.root", fOptions.GetPrefix("RQ_DATA_PREFIX").c_str(), fInputSeries.c_str(), dumpName.c_str());
   }

   fInputDataPathName = fOptions.GetPath("BATCALIB_RQDATA") + inputFilename;
   isFound = stat( fInputDataPathName.c_str(), &buffer );

   //check whether the file exists 
   if ( isFound == 0 )
   {
      cout <<"**** Detected RQ file: " << fInputDataPathName << endl;
   }
   else
   {
      cout <<"ERROR! Cannot find input RQ file: " << fInputDataPathName <<"\nplease check prefixes and pathnames!"
	   << endl;
      exit(1);
   }


   //read the BatRoot analysis flags
   ReadBatRootFlags();

   // --- Open the output (RRQ) file in overwrite mode to guarantee a fresh file ---

   string outputFilename;

   //Again, a slightly different treatment for merged vs unmerged files
   if(fOptions.GetIntParameter("USE_MERGED_RQ") == 1)
   {
     outputFilename = Form("%s_%s.root", fOptions.GetPrefix("RRQ_DATA_PREFIX").c_str(), 
			   fInputSeries.c_str()); 
   }
   else
   {
     outputFilename = Form("%s_%s_%s.root", fOptions.GetPrefix("RRQ_DATA_PREFIX").c_str(), 
			   fInputSeries.c_str(), dumpName.c_str()); 
   }

   fOutputDataPathName = fOptions.GetPath("BATCALIB_RRQDATA") + outputFilename;

   //Open file in recreate mode (overwrites existing file)

   fFileWriter = TFile::Open(fOutputDataPathName.c_str(), "RECREATE");
   cout <<"Opening file " << fOutputDataPathName <<" for RRQ writing" << endl;
   
   // create directories

   fFileWriter->mkdir("calibInfoDir", "Info directory");
   fFileWriter->mkdir("rrqDir", "RRQ directory");
   
   //Now don't close the file because ROOT is evil.
   //Compression and memory management will be horrible if you declare the tree before opening the file later 
   //(I only wasted 4 hours of a Sunday afternoon to figure this out).
   //This is probably a bug that should be reported to the authors at some point. - LLH

}

//Note this is implicitly being called at the end of each iteration over zip number, when CalibrateZipData object is destroyed!  
BatCalibIOManager::~BatCalibIOManager() 
{ 
//   cout <<"Goodbye from ~BatCalibIOManager()!" << endl;
}

// ========================== Input Management Functions ==================================

// This function returns a map with key = detector number and val = detector code.
// The information is obtained either through the detector configuration trees (stored in
// rq files since early 2011, or constructed from the user settings files.
map<int, int> BatCalibIOManager::GetDetectorMap()
{

  map<int, int> detectorMap;

  // === First check to see if detector configuration has been stored in the rq file ===
  // Careful!  This routine may not work if you are trying to read in multiple files with a wildcard.
  // This is a disabled feature at the moment.
  
  TFile* fileReader = TFile::Open(fInputDataPathName.c_str(), "READ");

  if(fileReader == 0 || fileReader->IsZombie())
  {
    cout <<"ERROR BatCalibIOManager::GetDetectorMap!  \nProblem reading input RQ file: " 
	 << fInputDataPathName 
	 << endl;
    exit(1);
  }
        
  TDirectory* configDir = fileReader->GetDirectory("detectorConfigDir");
 
  if(configDir != 0)
  {
   
    TList* keyList = configDir->GetListOfKeys();
    TIter keysItr(keyList);
 
    for(int keyCtr = 0; keyCtr < keyList->GetSize(); keyCtr++)
    { 

      TKey* key = (TKey*)keysItr.Next();
      string searchPrefix("detectorConfigZip");
      string keyName(key->GetName());
      string keyPrefix = keyName.substr(0, searchPrefix.size()); 
 
      // ignore if it doesn't have the right name
      if(keyPrefix != searchPrefix)
	continue;

      // if its a detectorConfigZip tree, extract the detector number
      TTree* detConfigTree = (TTree*)key->ReadObj();
      string keySuffix = keyName.substr(searchPrefix.size(), keyName.size()-(searchPrefix.size()) );

      // retrieve the detector number and type
      int detNum = atoi(keySuffix.c_str());
      double detType;

      detConfigTree->SetBranchAddress("DetType", &detType);
      detConfigTree->GetEntry(0);

      //now store in the map, but only if the user has specified 
      if( fOptions.DoZipProcessing(detNum) )
      {
	detectorMap.insert(pair< int, int >(detNum, (int)detType)); 
      }

  
    } //end loop over keys list

  }

  // === If no detector configuration, then construct the map from the user settings ===
  else
  {

    cout <<"In BatCalibIOManager::GetDetectrMap()  filling map from user settings file!" << endl;

    for(int detNum = 1; detNum <= fOptions.GetMaxZIPs() ; detNum++)
    {
      if( fOptions.DoZipProcessing(detNum) )
      {
	int detType = fOptions.GetIntParameter(detNum, "DET_TYPE");
	detectorMap.insert(pair< int, int >(detNum, detType)); 
      }
    }
    
  } //end if detectorConfigDir not found

 

  return detectorMap;
}

//reads and stores analysis flags from batroot
void BatCalibIOManager::ReadBatRootFlags()
{
  TChain* settingsTree = new TChain("infoDir/userSettingsTree");
  
  //The next 3 lines of ugliness are all ROOT's fault 
  //There is no way to check validity of an invalid TChain without
  //triggering an error statement.   
  //To suppress the error, Rene Brun suggests to do it this way

  gErrorIgnoreLevel=3001; //suppress the ROOT error - this variable is a ROOT global 
  int readSuccess = settingsTree->Add(fInputDataPathName.c_str(), 0);
  int nEntries = settingsTree->GetEntries();
  gErrorIgnoreLevel = 2; //to reset
  
  //check whether it was stored, if not then return assume yes
  if(readSuccess == 0 || nEntries == 0)
  {
    cout <<"WARNING!  Settings tree not found, so cannot check BatRoot processing flags"
	 <<"Will assume all are on by default"
	 << endl;
      
    return;
  }
  
  // === parse contents of tree for BatRoot flags ===

  TObjArray* leafList = settingsTree->GetListOfLeaves();

  int flagval;
  for(int leafItr = 0; leafItr < leafList->GetEntries(); leafItr++)
  {
    //get name and retrieve first word separated by "_"
    string leafName((leafList->At(leafItr))->GetName());

    int splitPoint = leafName.find("_alg");
    string firstWord = leafName.substr(0,splitPoint+4); //+4 for the "_alg" tag 
    string secondWord = leafName.substr(splitPoint+5);

    if(firstWord == "PT_alg" || firstWord == "PSIDES_alg")
    {
      vector<int> emptyVect;
      string analysisName = firstWord.substr(0,splitPoint) + "_" + secondWord;
      fBatRootPhononAlgMap.insert(pair<string, vector<int> >(analysisName, emptyVect));
    }
     else if(firstWord == "phonon_alg")
    {
      vector<int> emptyVect;
      fBatRootPhononAlgMap.insert(pair<string, vector<int> >(secondWord, emptyVect));
    }
     else if(firstWord == "charge_alg")
    {
      vector<int> emptyVect;
      fBatRootChargeAlgMap.insert(pair<string, vector<int> >(secondWord, emptyVect));
    }

    //get the read or write flags and put them in a Map 
    //FIXME 
    //currently, have to explicitly remove "WRITE_RQ" from list
    //because it has more than one element (is a vector) 
    //either there needs to be a convention or we need to read the
    //length of the vector first and only select ones with a single
    //integer element [ANV]
    if((leafName.find("WRITE_RQ") == string::npos) && 
      ((leafName.find("READ_") != string::npos) || (leafName.find("WRITE_") != string::npos))){
      settingsTree->SetBranchAddress(leafName.c_str(),&flagval);
      settingsTree->GetEntry(0); 
      fBatRootUserSettingsFlagMap[leafName] = flagval;
    }

    if(leafName == "DO_SIM_FROM_PULSE" || leafName == "DO_SIM_FROM_TEMPLATE" ||
       leafName == "DO_PHONONSIM" || leafName == "DO_CHARGESIM" ||
       leafName == "DO_PTSIM" || leafName == "DO_PSIDESSIM" ||
       leafName == "DO_PCHANSIM")
    {
	settingsTree->SetBranchAddress(leafName.c_str(),&flagval);
	settingsTree->GetEntry(0);
	fBatRootUserSettingsFlagMap[leafName] = flagval;
    }

  } //end loop over leaves in the settings tree


  //Loop over entries in the tree and fill the algorithm maps.
  //Note that one "entry" corresponds to a vector of values with 
  //length = MAX_ZIPS, so one entry has MAX_ZIPS values per leaf.
  //We really only need to read the first entry.  Multiple entries
  //occur when we merge rq files

  int maxZips;
  settingsTree->SetBranchAddress("MAX_ZIPS", &maxZips);
  settingsTree->GetEntry(0); //don't loop if multiple entries, they're redundant
    
  //iterate over charge algorithms and add value to list
  map<string, vector<int> >::iterator mapItr = fBatRootChargeAlgMap.begin();
  for( ; mapItr != fBatRootChargeAlgMap.end(); mapItr++)
  {
    string algName = "charge_alg_" + mapItr->first;
    
    int valList[maxZips];
    string branchName =  algName; 
    settingsTree->SetBranchAddress(branchName.c_str(), valList);
    settingsTree->GetEntry(0);

    (mapItr->second).assign(valList, valList+maxZips);

    // reset for next reading
    settingsTree->ResetBranchAddresses();
  }

  //iterate over phonon algorithms and add value to list
  mapItr = fBatRootPhononAlgMap.begin();
  for( ; mapItr != fBatRootPhononAlgMap.end(); mapItr++)
  {
    string algName =  mapItr->first;
    int splitPoint = algName.find_first_of("_");
    
    if (algName.substr(0,splitPoint) == "PT" || algName.substr(0,splitPoint) == "PSIDES") 
      algName  = algName.substr(0,splitPoint) + "_alg" +  algName.substr(splitPoint);
    else
      algName = "phonon_alg_" + mapItr->first;
    
    
    int valList[maxZips];
    string branchName =  algName; 
    settingsTree->SetBranchAddress(branchName.c_str(), valList);
    settingsTree->GetEntry(0);

    (mapItr->second).assign(valList, valList+maxZips);

    // reset for next reading
    settingsTree->ResetBranchAddresses();
    
  }


  // === cleanup ===

  settingsTree->Delete();

  return;
}

int BatCalibIOManager::CheckBatRootChargeAlg(const string& algName, int detNum)
{
  int wasOn = 1;
  
  //check whether it exists in the map
  if(fBatRootChargeAlgMap.count(algName) > 0)
  {
    wasOn = (fBatRootChargeAlgMap[algName])[detNum-1];
 
    return wasOn;
  }
  else
  {
    cout <<"WARNING!  Looking for charge algorithm " << algName 
	 <<", but flag not stored. Will not compute the derived RRQs!" 
	 << endl;

    return 0;
  }

}

//By default assume on, even if it cant' find the algorithm
int BatCalibIOManager::CheckBatRootPhononAlg(const string& algName, int detNum)
{
  int wasOn = 1;

  //check whether it exists in the map
  if(fBatRootPhononAlgMap.count(algName) > 0)
  {
    wasOn = (fBatRootPhononAlgMap[algName])[detNum-1];
 
    return wasOn;
  }
  else
  {

    cout <<"WARNING!  Looking for phonon algorithm " << algName 
 	 <<", but flag not stored.  Will not compute the derived RRQs" 
 	 << endl;

    return 0;
  }

}

bool BatCalibIOManager::CheckBatRootUserSettingsFlags(const string& flagName)
{
  //assume false
  bool isSet = false;

  //check whether it exists in the map
  if(fBatRootUserSettingsFlagMap.find(flagName) != fBatRootUserSettingsFlagMap.end())
  {
    if(fBatRootUserSettingsFlagMap[flagName] > 0)
      isSet = true;
    else
      isSet = false;
  }
  else
  {
    cout <<"BatCalibIOManager::CheckBatRootUserSettingsFlags(): WARNING!  Did not find BatRoot Flag " 
       << flagName <<", will assume false" << endl;
  }

  return isSet;
}

int BatCalibIOManager::LoadTree(const string& dir, const string& treename)
{
   TString treePathName = dir + "/" + treename;
   fActiveReadTree = new TChain(treePathName);

   //The next 3 lines of ugliness are all ROOT's fault 
   //There is no way to check validity of an invalid TChain without
   //triggering an error statement.   
   //To supporess the error, Rene Brun suggests to do it this way

   gErrorIgnoreLevel=3001; //suppress the ROOT error - this variable is a ROOT global 
   int readSuccess = fActiveReadTree->Add(fInputDataPathName.c_str(), 0);
   int nEntries = fActiveReadTree->GetEntries();
   gErrorIgnoreLevel = 2; //to reset

   //Check whether the chain is valid
   if(readSuccess == 0 || nEntries == 0)
   {
      cout <<"WARNING! Requested tree: " << treename <<" was not found!  Skipping..." << endl;
      return 0;
   }

   cout <<"Loading Tree : " << fActiveReadTree->GetName() << endl;

   //deactivate *all* branches by default for sake of speed
   //to read specific branches, activate them with the BatCalibIOManager::Activate command
   //fActiveReadTree->
   //Is the above necessary?

   return 1;
}

void BatCalibIOManager::DeleteActiveTree()
{
   cout <<"Deleting Tree and active branches : " << fActiveReadTree->GetName() << endl;

   //freeing memory allocated for the branch map

    for(map<string, double*>::iterator mapItr = fActiveBranchMap.begin(); mapItr != fActiveBranchMap.end(); ++mapItr)
    {
       delete (*mapItr).second;
    }

    fActiveBranchMap.clear();

//   cout <<"Done cleaning out the active branch map!" << endl;


   //freeing memory for the active tree

   fActiveReadTree->Delete();

//   cout <<"Done deleting the active read tree!" << endl;

   return;
}

void BatCalibIOManager::AddFriendTree(const string& dir, const string& treename)
{
   TString treePathName = dir + "/" + treename;

   gErrorIgnoreLevel=3001; //suppress the ROOT error - this variable is a ROOT global 
   fActiveReadTree->AddFriend(treePathName);
   gErrorIgnoreLevel = 2; //to reset


   cout <<"Adding friend: " << treePathName << endl;

   return;
   
}

void BatCalibIOManager::ReadNextEntry(int eventCtr)
{
   fActiveReadTree->GetEntry(eventCtr);
   
   return;
}

int BatCalibIOManager::GetMaxEntries()
{
   return fActiveReadTree->GetEntries();
}

//The date needs to be in the formate of DD-Month-YYYY (i.e. 31-Dec-2009)
bool BatCalibIOManager::DoesRQFilePredate(const string& checkDate)
{
   bool doesRQFilePredate = 0;

   //Get the stored date in the RQ file
   char storedDate[20];

   TChain* processingTree = new TChain("infoDir/processingTree");

   //The next 3 lines of ugliness are all ROOT's fault 
   //There is no way to check validity of an invalid TChain without
   //triggering an error statement.   
   //To supporess the error, Rene Brun suggests to do it this way

   gErrorIgnoreLevel=3001; //suppress the ROOT error - this variable is a ROOT global 
   int readSuccess = processingTree->Add(fInputDataPathName.c_str(), 0);
   int nEntries = processingTree->GetEntries();
   gErrorIgnoreLevel = 2; //to reset
   
   //check whether it was stored, if not then exit and assume yes
   if(readSuccess == 0 || nEntries == 0)
   {
      cout <<"WARNING!  Processing tree not found, so cannot check date of file."
	   <<"Assuming it is from first production and continuing!"
	   << endl;
      
      return 1;
   }

   processingTree->SetBranchAddress("date", storedDate);
   processingTree->GetEntry(0);

   //put it into the time structure
   char* storedDateAndTime;
   storedDateAndTime = Form("%s-00-00-00", storedDate);

   struct tm storedDateInfo;
   if(strptime(storedDateAndTime,"%d-%b-%Y-%H-%M-%S", &storedDateInfo) == NULL)
   {
      cout <<"ERROR BatCalibIOManager::DoesRQFilePredate() problem reading stored date!" 
 	   << endl;
      exit(1);
   }
   time_t storedTime = mktime(&storedDateInfo);

//   cout  <<"Rechecking the stored date: " << asctime(&storedDateInfo) << endl;

   //put the check daate into the time structure
   char* checkDateAndTime;
   checkDateAndTime = Form("%s-00-00-00", checkDate.c_str());
   
   struct tm checkDateInfo;
   if(strptime(checkDateAndTime,"%d-%b-%Y-%H-%M-%S", &checkDateInfo) == NULL)
    {
      cout <<"ERROR BatCalibIOManager::DoesRQFilePredate() problem reading check date, check string format!" 
	   << endl;
      exit(1);
   }
   time_t checkTime = mktime(&checkDateInfo);

//   cout  <<"the check date: " << asctime(&checkDateInfo) << endl;

   doesRQFilePredate = (difftime(checkTime, storedTime) > 0 ? 1 : 0);

   // === cleanup ===

   processingTree->Delete();


   // === done ===

   return doesRQFilePredate;
}

//add this quantity to the map
int BatCalibIOManager::Activate(const string& varName)
{

   //check that the variable does not exist yet
   if( fActiveBranchMap.count(varName) > 0 )
   {
      cerr <<"BatCalibIOManager::Activate ERROR! Trying to add variable ActiveBranchMap, but it already exists! " << varName
 	   << endl;
      exit(1);
   }
       
   //add the variable to the map 
   fActiveBranchMap.insert(pair<string,double*>(varName, new double)); 
   //cout <<"Activating branch: " << varName << endl;

   //set the branch address to the map entry
   map< string, double*>::iterator mapItr = fActiveBranchMap.find(varName);
   fActiveReadTree->SetBranchAddress(varName.c_str(), mapItr->second);
   
   //Check that the RQ exists and exit if not
   if(fActiveReadTree->GetBranchStatus(varName.c_str()) != 1)
   {
      cout <<"ERROR! BatCalibIOManager::Activate()  Attempting to read " << varName <<" but it does not exist in rq file! "
	   <<"Check your options file before continuing!"
	   << endl;

      exit(1);
   }

   return fActiveReadTree->GetBranchStatus(varName.c_str());
}

//it is sometimes useful to have the series without an underscore [ANV]
string BatCalibIOManager::GetSeriesStringWithoutUnderscore()
{
  ostringstream series;
  //iterate through string
  string::iterator itr;
  for(itr = fInputSeries.begin(); itr != fInputSeries.end(); ++itr){
    if(*itr != '_')
      series << *itr;  
  }

  return series.str();
}

double BatCalibIOManager::Get(const string& varName)
{

   //check that the variable exists
   if( fActiveBranchMap.count(varName)== 0 )
   {
      cerr <<"BatCalibIOManager::Get ERROR! Trying to get variable that is not active: " << varName
 	   << endl;
      exit(1);
   }

   
   return *(fActiveBranchMap[varName]);
}

//a special function to get the optimal filter resolutions from the filter trees
// FIXME, keep this function for back compatibility
void BatCalibIOManager::FillOFResolution(int detNum, vector<double>& delaySig, vector<double>& ampSig)
{

   // === clear the vectors ===
   
   if(delaySig.size() > 0) delaySig.clear();
   if(ampSig.size() > 0)   ampSig.clear();

   // === open the file ===

   TChain* inputFilterTree = new TChain(Form("infoDir/filterTreeZip%d", detNum));

   //The next 3 lines of ugliness are all ROOT's fault 
   //There is no way to check validity of an invalid TChain without
   //triggering an error statement.   
   //To supporess the error, Rene Brun suggests to do it this way

   gErrorIgnoreLevel=3001; //suppress the ROOT error - this variable is a ROOT global 
   int readSuccess = inputFilterTree->Add(fInputDataPathName.c_str(), 0);
   int nEntries = inputFilterTree->GetEntries();
   gErrorIgnoreLevel = 2; //to reset

   std::vector<double*> delaySigPtr;
   std::vector<double*> ampSigPtr;
   
   //setup input and output for all phonon channels
   for(int chanItr=0; chanItr < BatCalibTypes::kZIPFLIPNAllChan; chanItr++)
   {

      //input
      string prefix = BatCalibTypes::kZIPFLIPChannelNames[chanItr];
      string delName = prefix+"delaysig";
      string ampName = prefix+"ampsig";

      delaySigPtr.push_back(new double);
      ampSigPtr.push_back(new double);

      //skip if trees doesn't exist
      if(readSuccess == 0 || nEntries == 0) 
      { 
	 cout <<"BatCalibIOManager::FillOFResolution WARNING! requested resolution not stored, replacing w/ 0.0"
	      << endl;

	 *(delaySigPtr[chanItr]) = 0;
	 *(ampSigPtr[chanItr]) = 0;

	 continue;
      }

      inputFilterTree->SetBranchAddress(delName.c_str(), delaySigPtr[chanItr]);
      inputFilterTree->SetBranchAddress(ampName.c_str(), ampSigPtr[chanItr]);
   }


   // === grab data from BatRoot filterTree and RQ tree ===

   if(inputFilterTree != NULL) 
      inputFilterTree->GetEntry(0);


   // === store data in the passed vectors ===

   for(int chanItr=0; chanItr < BatCalibTypes::kZIPFLIPNAllChan; chanItr++)
   {
      delaySig.push_back(*delaySigPtr[chanItr]);
      ampSig.push_back(*ampSigPtr[chanItr]);
   }


   // === cleanup

   if(inputFilterTree != NULL)
      inputFilterTree->Delete();

   delaySigPtr.clear();
   ampSigPtr.clear();


   return;
}

//a special function to get the optimal filter resolutions from the filter trees
void BatCalibIOManager::FillOFResolution(int detNum, int detType, vector<double>& delaySig, vector<double>& ampSig)
{


   fIsOFresFilled = true;
   

   // === clear the vectors ===
   
   if(delaySig.size() > 0) delaySig.clear();
   if(ampSig.size() > 0)   ampSig.clear();


   // ==== Get channel names ====

   unsigned int nbChannels;
   vector<string> chanNames;

   if (detType==BatCalibTypes::kiZIPSoudanTriFold) {
  
      nbChannels = BatCalibTypes::kiZIPSoudanNAllChan;
      chanNames.assign(BatCalibTypes::kiZIPSoudanChannelNames,BatCalibTypes::kiZIPSoudanChannelNames+nbChannels);
  
   } else if (detType==BatCalibTypes::kFLIPDetType     ||
              detType==BatCalibTypes::kZIPDetType      ||
              detType==BatCalibTypes::kmZIPDetType     ||
              detType==BatCalibTypes::kCDMSliteIDetType ||
              detType==BatCalibTypes::kDualEndcapDetType ) {

      nbChannels = BatCalibTypes::kZIPFLIPNAllChan;
      chanNames.assign(BatCalibTypes::kZIPFLIPChannelNames,BatCalibTypes::kZIPFLIPChannelNames+nbChannels);

   } else if (detType==BatCalibTypes::kEndcapDetType) { 
   
      nbChannels = BatCalibTypes::kEndcapNAllChan;
      chanNames.assign(BatCalibTypes::kEndcapChannelNames,BatCalibTypes::kEndcapChannelNames+nbChannels);

   } else {

    cout <<"BatCalibIOManager::FillOFResolution WARNING! Detector type is unknown, will not calculate OFres, detType= "<< detType << "!" <<  endl;
    return;
   }

 
   // === open the file ===

   TChain* inputFilterTree = new TChain(Form("infoDir/filterTreeZip%d", detNum));

   //The next 3 lines of ugliness are all ROOT's fault 
   //There is no way to check validity of an invalid TChain without
   //triggering an error statement.   
   //To supporess the error, Rene Brun suggests to do it this way

   gErrorIgnoreLevel=3001; //suppress the ROOT error - this variable is a ROOT global 
   int readSuccess = inputFilterTree->Add(fInputDataPathName.c_str(), 0);
   int nEntries = inputFilterTree->GetEntries();
   gErrorIgnoreLevel = 2; //to reset


   if(readSuccess == 0 || nEntries == 0) 
      {  
        cout <<"BatCalibIOManager::FillOFResolution WARNING! Requested resolution not stored!"
	     << endl;
        fIsOFresFilled = false;
        return;
      }


   std::vector<double*> delaySigPtr;
   std::vector<double*> ampSigPtr;
   
   //setup input and output for all phonon channels
   for(unsigned int chanItr=0; chanItr < nbChannels; chanItr++)
   {

      //input
      string prefix = chanNames[chanItr];
      string delName = prefix+"delaysig";
      string ampName = prefix+"ampsig";

   
      delaySigPtr.push_back(new double);
      ampSigPtr.push_back(new double);

     
      inputFilterTree->SetBranchAddress(delName.c_str(), delaySigPtr[chanItr]);
      inputFilterTree->SetBranchAddress(ampName.c_str(), ampSigPtr[chanItr]);
   }


   // === grab data from BatRoot filterTree and RQ tree ===

   if(inputFilterTree != NULL) 
      inputFilterTree->GetEntry(0);


   // === store data in the passed vectors ===

   for(unsigned int chanItr=0; chanItr < nbChannels; chanItr++)
   {
      delaySig.push_back(*delaySigPtr[chanItr]);
      ampSig.push_back(*ampSigPtr[chanItr]);
   }


   // === cleanup

   if(inputFilterTree != NULL)
      inputFilterTree->Delete();

   delaySigPtr.clear();
   ampSigPtr.clear();


   return;
}




// ==================== Output Management Functions ===============================

//this is a little ugly, but seems to work ok
void BatCalibIOManager::CreateAndWriteFilterTree()
{
  
   fFileWriter->cd("calibInfoDir");

   //loop over detectors and only store a filterTree for those zips that are on
   for(int detNum = 1; detNum <= fOptions.GetMaxZIPs(); detNum++)
   {
      
      // check if we want to process this detector at all
      if( !fOptions.DoZipProcessing(detNum) ) { continue; }

      // FIXME, this is temporary until the bug in merging script is fixed! - i.e shortly after 1/11/10 
      if( !fOptions.DoZipAlgorithm(detNum, "CalibrateOFRes") ) { continue; }

      // === create the filterTree  ===

      TTree *filterTree = new TTree(Form("calibFilterTreeZip%d", detNum),Form("z%d filter information tree", detNum));

      // === open the input trees ===

      TChain* inputFilterTree = new TChain(Form("infoDir/filterTreeZip%d", detNum));
      
      //The next 3 lines of ugliness are all ROOT's fault 
      //There is no way to check validity of an invalid TChain without
      //triggering an error statement.   
      //To supporess the error, Rene Brun suggests to do it this way

      gErrorIgnoreLevel=3001; //suppress the ROOT error - this variable is a ROOT global
      int readSuccess = inputFilterTree->Add(fInputDataPathName.c_str(), 0);
      int nEntries = inputFilterTree->GetEntries();
      gErrorIgnoreLevel = 2; //to reset

      //skip if trees doesn't exist
      if(readSuccess == 0 || nEntries == 0) { continue; }

      
      // === copy the tags and production dates from the input to the output file ===

      //not sure if there's a way around guessing maximum length of these fields
      char filterTag[60];
      char templateTag[40];
      char noiseCodeGitTag_cdmsbats[1024];
      char noiseCodeGitTag_batcommon[1024];
      char filterProductionDate[20];      

      //extract from input
      inputFilterTree->SetBranchAddress("filterTag", filterTag);
      inputFilterTree->SetBranchAddress("noiseCodeGitTag_cdmsbats", noiseCodeGitTag_cdmsbats);
      inputFilterTree->SetBranchAddress("noiseCodeGitTag_batcommon", noiseCodeGitTag_batcommon);
      inputFilterTree->SetBranchAddress("templateTag", templateTag);      
      inputFilterTree->SetBranchAddress("filterProductionDate", filterProductionDate);      

      //setup the output
      filterTree->Branch("filterTag", &filterTag, "filterTag/C");
      filterTree->Branch("noiseCodeGitTag_cdmsbats", &noiseCodeGitTag_cdmsbats, "noiseCodeGitTag_cdmsbats/C");
      filterTree->Branch("noiseCodeGitTag_batcommon", &noiseCodeGitTag_batcommon, "noiseCodeGitTag_batcommon/C");
      filterTree->Branch("templateTag", &templateTag, "templateTag/C");
      filterTree->Branch("filterProductionDate", &filterProductionDate, "filterProductionDate/C");

      // === grab data from BatRoot filter tree ===

      inputFilterTree->GetEntry(0);
      
      // === fill/write tree ===
      
      filterTree->Fill();
      filterTree->Write();

      // === cleanup ===

      inputFilterTree->Delete();
      
   }
   

   return;
}

void BatCalibIOManager::CreateAndWriteProcessingInfoTree(string date, string gitTag_cdmsbats, string gitTag_batcommon)
{
   //create the processingTree

   fFileWriter->cd("calibInfoDir");
   TTree *processingTree = new TTree("calibProcessingTree","Processing information tree");


   // ==== store date and git tag for BatCalib ==== 
  
   // date
   char dateChar[date.size()+1];
   strcpy(dateChar,date.c_str()); 

   // git tag
   char BatCalibGitTag_cdmsbats[gitTag_cdmsbats.size()+1];
   strcpy(BatCalibGitTag_cdmsbats,gitTag_cdmsbats.c_str()); 
   char BatCalibGitTag_batcommon[gitTag_batcommon.size()+1];
   strcpy(BatCalibGitTag_batcommon,gitTag_batcommon.c_str()); 

   // ==== store date and git tag for RQ input file ==== 
  
   char storedDate[20] = "missing";
   char storedGitTag_cdmsbats[1024] = "missing";
   char storedGitTag_batcommon[1024] = "missing";

   TChain* inputProcessingTree = new TChain("infoDir/processingTree");
   
   //The next 3 lines of ugliness are all ROOT's fault 
   //There is no way to check validity of an invalid TChain without
   //triggering an error statement.   
   //To supporess the error, Rene Brun suggests to do it this way

   gErrorIgnoreLevel=3001; //suppress the ROOT error - this variable is a ROOT global 
   int readSuccess = inputProcessingTree->Add(fInputDataPathName.c_str(), 0);
   int nEntries = inputProcessingTree->GetEntries();
   gErrorIgnoreLevel = 2; //to reset

   if(readSuccess == 0 || nEntries == 0)
   {
      cout <<"WARNING! Processing tree not stored in RQ file, unable to copy BatRootGitTag_<component>"
	   << endl;
   }
   else
   {
      inputProcessingTree->SetBranchAddress("date", storedDate);
      inputProcessingTree->SetBranchAddress("BatRootGitTag_cdmsbats", storedGitTag_cdmsbats);
      inputProcessingTree->SetBranchAddress("BatRootGitTag_batcommon", storedGitTag_batcommon);
      inputProcessingTree->GetEntry(0);
   }


   // === make branches and fill/write tree ===

   processingTree->Branch("BatCalibFileDate", &dateChar, "BatCalibFileDate/C");
   processingTree->Branch("BatCalibGitTag_cdmsbats", &BatCalibGitTag_cdmsbats, "BatCalibGitTag_cdmsbats/C");
   processingTree->Branch("BatCalibGitTag_batcommon", &BatCalibGitTag_batcommon, "BatCalibGitTag_batcommon/C");

   processingTree->Branch("BatRootFileDate", &storedDate, "BatRootFileDate/C");
   processingTree->Branch("BatRootGitTag_cdmsbats", &storedGitTag_cdmsbats, "BatRootGitTag_cdmsbats/C");
   processingTree->Branch("BatRootGitTag_batcommon", &storedGitTag_batcommon, "BatRootGitTag_batcommon/C");

   processingTree->Fill();
   processingTree->Write(); 


   // === cleanup ===

   inputProcessingTree->Delete();

   return;
}


//generic function for either calibrated RRQ's or position corrected RRQ's.
// just change the RRQ_DATA_PREFIX in the options file)

void BatCalibIOManager::ConstructOutputRRQTree(map<string, double>* rrqList, const string& treeName)
{
   //create a tree for writing rrq's

   fFileWriter->cd("rrqDir");
   fOutputRRQTree = new TTree(treeName.c_str(), treeName.c_str());

   
   //Loop over elements in the RRQ list and make a branch for each

   map<string,double>::iterator rrqListItr = rrqList->begin();
   for( ; rrqListItr!=rrqList->end(); rrqListItr++)
   {
      string name = rrqListItr->first;
      fOutputRRQTree->Branch(name.c_str(), &(rrqListItr->second), (name+"/D").c_str());

//      cout <<"Adding Branch: " << name+"/D" <<", w/ initial value= " << rrqListItr->second << endl;
   }

   
   return;
}

void BatCalibIOManager::FillOutputRRQTree()
{

   fFileWriter->cd("rrqDir");
   fOutputRRQTree->Fill();

   return;
}

void BatCalibIOManager::WriteOutputRRQTree()
{
   //reopen the file for updating within this session
   
   fFileWriter->cd("rrqDir");

   //Write the tree to the file
   fOutputRRQTree->Write("", TObject::kOverwrite); 

   //Cleanup
   fOutputRRQTree->Delete();
   
   return;
}
