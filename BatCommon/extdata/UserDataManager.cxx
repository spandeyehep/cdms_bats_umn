////////////////////////////////////////////////////////////////////////////////////
//Class Name: UserDataManager
//Authors: B. Serfass
//Description:  This class reads the user settings files, store the informations,
//and manage access to to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//Feb. 2011 - Renamed this class from ConfigDataManager to UserDataManager, to avoid
//            confusion with the new classes DetectorConfigData and
//            DetectorConfigManager [LLH].
//
////////////////////////////////////////////////////////////////////////////////////

#include "UserDataManager.h"
#include "ChannelMapHelper.h"

// Standard library
#include <math.h>
#include <algorithm>

////////////////////////////////////////////////////////

UserDataManager::UserDataManager() {}
UserDataManager::~UserDataManager() {}

// ___________________ Get functions _______________________


// ...................... EXTERNAL FILES  ...........................

bool UserDataManager::DoRead(const string& varName, bool required) const { 
 
   // check if parameter exist
   if (!ListManager::HasParameter(fMapVectInt,"READ_"+varName)) {
     if(required){
       cerr << "  "<<endl;
       cerr << "UserDataManager::DoRead: ERROR!  You need to add a line in your processing "<< endl;
       cerr << "setttings file (in UserSettings/BatRootSettings/processing) with:" << endl;
       cerr << "  "<<endl;
       cerr << "     READ " <<  varName  << "  =   0 (or 1 if you want to read the file)" << endl;
       cerr << "  "<<endl;
       exit(1);
     }
     else{
       cout << "  "<<endl
	    << "UserDataManager::DoRead: Setting 'READ "<< varName 
	    <<"' not found; defaulting to false. "<<endl;
       return false;
     }
  }

  // Now get value 
  vector<int> intVect = ListManager::GetParameter(fMapVectInt,"READ_"+varName); 
  return (bool) intVect[0]; 
}



// ....................... DETECTOR INFORMATIONS ............................


bool UserDataManager::HasDoubleParameter(int detNum,const string& varName)
{
  bool hasParameter = false;
  
  map< int, map<string,vector<double> > >::iterator itMap =
    fZipMapOfMapVectDouble.find(detNum);
  
  if (itMap!=fZipMapOfMapVectDouble.end())
    hasParameter = ListManager::HasParameter(itMap->second,varName);
  
  return hasParameter;
}


bool UserDataManager::HasIntParameter(int detNum,const string& varName)
{
  bool hasParameter = false;
  
  map< int, map<string,vector<int> > >::iterator itMap =
    fZipMapOfMapVectInt.find(detNum);
  
  if (itMap!=fZipMapOfMapVectInt.end())
    hasParameter = ListManager::HasParameter(itMap->second,varName);
  
  return hasParameter;
}

bool UserDataManager::HasStringParameter(int detNum,const string& varName)
{
  bool hasParameter = false;
  
  map< int, map<string,vector<string> > >::iterator itMap =
    fZipMapOfMapVectString.find(detNum);
  
  if (itMap!=fZipMapOfMapVectString.end())
    hasParameter = ListManager::HasParameter(itMap->second,varName);
  
  return hasParameter;
}










int UserDataManager::GetTowerNumber(int detNum) const {
  string towerName = ListManager::GetParameter(fZipMapOfMapString.find(detNum)->second,"DET_TOWER");
  string towerNumberStr = towerName.substr(1+towerName.find("T",0));
  return atoi(towerNumberStr.c_str());
}



bool UserDataManager::IsGe(int detNum) const {
 string detType = ListManager::GetParameter(fZipMapOfMapString.find(detNum)->second,"DET_SUBSTRATE");
 
 return (detType=="Ge");
}

bool UserDataManager::IsSi(int detNum) const {
 string  detType = ListManager::GetParameter(fZipMapOfMapString.find(detNum)->second,"DET_SUBSTRATE");
 
 return (detType=="Si");
}

// ...................  Template .....................


bool UserDataManager::CalcChargeTemplate(int detNum) const {
 string keyName = "CALC_CHARGE_TEMPLATE";
 map<string, vector<int> > zipMap =  fZipMapOfMapVectInt.find(detNum)->second;

 //check if specified in config file
 if (ListManager::HasParameter(zipMap,keyName)) { 
    vector<int> intVect = ListManager::GetParameter(zipMap,keyName);
    return (bool) intVect[0];
  } else {
    return false;
  }
}

bool UserDataManager::CalcPhononTemplate(int detNum) const 

{
 string keyName = "CALC_PHONON_TEMPLATE";
 map<string, vector<int> > zipMap =  fZipMapOfMapVectInt.find(detNum)->second;

 //check if specified in config file
 if (ListManager::HasParameter(zipMap,keyName))
  { 
    vector<int> intVect = ListManager::GetParameter(zipMap,keyName);
    return (bool) intVect[0];
  } else {
    return true;
  }
}





bool UserDataManager::DoZipProcessing(int detNum) const { 
  if(fZipMapOfMapVectInt.count(detNum) > 0) {
    vector<int> intVect =  ListManager::GetParameter(fZipMapOfMapVectInt.find(detNum)->second,"DO_PROCESSING");  
    return (bool) intVect[0];
  } else {
    cout <<"\nERROR in UserDataManager::DoZipProcessing!  Requested detector number " << detNum
	 <<" does not exist in the processing config file!"
	 << endl;
    exit(1);
  }

  return (0);
}


bool UserDataManager::WriteZipRQ(int detNum) const { 
  if(fZipMapOfMapVectInt.count(detNum) > 0) {
    vector<int> intVect =  ListManager::GetParameter(fZipMapOfMapVectInt.find(detNum)->second,"WRITE_RQ");  
    return (bool) intVect[0];
  } else {
    cout <<"\nERROR in UserDataManager::WriteZipRQ!  Requested detector number" << detNum
	 <<" does not exist in the processing config file!"
	 << endl;
    exit(1);
  }

  return (0);
}


bool UserDataManager::DoAlgorithm(int detNum, const string& sensorType,
				  const string& name) const {
  if (sensorType!="phonon" &&  sensorType!="charge" && sensorType!="PT" &&
      sensorType!="PSIDES") {
    cerr << "UserDataManager::DoAlgorithm: ERROR!  Sensor type '"<< sensorType << "' not recognized!" << endl;
    exit(1);
  }

  string keyName = sensorType + "_alg_" + name;
  map<string, vector<int> > zipMap =  fZipMapOfMapVectInt.find(detNum)->second;
  
  //check if specified in config file
  if (ListManager::HasParameter(zipMap,keyName)) { 
    vector<int> intVect = ListManager::GetParameter(zipMap,keyName);
    return (bool) intVect[0];
  } else {
    return false;
  }
}

bool UserDataManager::DoZipAlgorithm(int detNum, const string& name) const {
  string keyName = "ALG_" + name;
  map<string, vector<int> > zipMap =  fZipMapOfMapVectInt.find(detNum)->second;
  
  //check if specified in config file
  if (ListManager::HasParameter(zipMap,keyName))
    { 
      vector<int> intVect = ListManager::GetParameter(zipMap,keyName);
      return (bool) intVect[0];
    } else {
    return false;
  }
}


list<string> 
UserDataManager::GetListAlgorithms(int detNum, const string& sensorType) const {
  if (sensorType!="phonon" &&  sensorType!="charge" && sensorType!="PT" &&
      sensorType!="PSIDES") {
    cerr << "UserDataManager::GetListAlgorithms: ERROR!  Sensor type '"<< sensorType << "' not recognized!" << endl;
    exit(1);
  }

  // get ZIP map
  map<string, vector<int> > zipMap =  fZipMapOfMapVectInt.find(detNum)->second;
  
  string keyBase = sensorType + "_alg_" ;
  int lengthBase = keyBase.length();
  
  // create list of analysis classes
  list<string> listAlgorithms;
  
  // loop through map and get name 
  map<string, vector<int> >::const_iterator itMap;
  
  for(itMap = zipMap.begin(); itMap != zipMap.end(); itMap++) {
    string keyRecorded = itMap->first;
      
    // check if there is a key starting with string keyBase
    string::size_type posBase = keyRecorded.find(keyBase);
    if (posBase!=string::npos) { 
      // check if DoAlgorithm set to 1 
      bool boolValue = (bool) itMap->second[0];
      if (boolValue) {
	// now include the name in the 
	string analysisName = keyRecorded.substr(posBase+lengthBase);
	listAlgorithms.push_back(analysisName);
      }
    } 
  }
  
  // add "BasicPulseCalc", we do not permit this to be turned off in config file
  listAlgorithms.push_back("BasicPulseCalc");
  
  // rq generation handles slightly differently for SIM mode
  listAlgorithms.push_back("SimulateFromRandoms");

  return listAlgorithms;
}

list<string> 
UserDataManager::GetListAlgorithms(const string& detName,
				   const string& sensorType) const {
  string detNumStr = detName.substr(3);
  int detNum = atoi(detNumStr.c_str());

  return  GetListAlgorithms(detNum, sensorType);     
}


double 
UserDataManager::GetRiseTime(int detNum, int detType, const string& channel) const {
  string chanName = channel;

  // get channel index
  uint index = ChannelMapHelper::GetChannelIndexBySide(detType, chanName);

  // get risetime vector
  vector<double> vect = ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,"P_RISE_TIME");
 
  // check length of vector
  if (vect.size()==1) {
      return vect[0];
  } else if (vect.size()>index) {
      return vect[index];
  }

  // should not come until here
  cerr << "UserDataManager::GetRiseTime: ERROR getting the risetime!" << endl;
  exit(1);
}


double 
UserDataManager::GetFallTime(int detNum,  int detType, const string& channel) const {
  string chanName = channel;
  
  // get channel index
  uint index = ChannelMapHelper::GetChannelIndexBySide(detType, chanName);
  
  // get falltime vector
  vector<double> vect = ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,"P_FALL_TIME");
  
  // check length of vector
  if (vect.size()==1) {
    return vect[0];
  } else if (vect.size()>index) {
    return vect[index];
  }

  // should not come until here
  cerr << "UserDataManager::GetFallTime: ERROR getting the falltime!" << endl;
  exit(1);
}


double UserDataManager::GetTailFitFallTime(int detNum, int detType, const string& channel) const {

  string chanName = channel;
  
  // get channel index
  uint index = ChannelMapHelper::GetChannelIndexBySide(detType, chanName);
  
  // get calibration vector
  vector<double> vect = ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,"P_TAIL_FIT_TAU");
  
  
  // check length of vector
  if (vect.size()==1) {
      return vect[0];
  } else if (vect.size()>index) {
    return vect[index];
  }

  // should not come until here
  cerr << "UserDataManager::GetTailFitFallTime: ERROR getting tail fit fall time!" << endl;
  exit(1);
}


double UserDataManager::GetTailFitFallTimeSigma(int detNum, int detType, const string& channel) const {

  string chanName = channel;
  
  // get channel index
  uint index = ChannelMapHelper::GetChannelIndexBySide(detType, chanName);
  
  // get calibration vector
  vector<double> vect = ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,"P_TAIL_FIT_TAU_SIGMA");
  
  
  // check length of vector
  if (vect.size()==1) {
      return vect[0];
  } else if (vect.size()>index) {
    return vect[index];
  }

  // should not come until here
  cerr << "UserDataManager::GetTailFitFallTimeSigma: ERROR getting tail fit fall time sigma!" << endl;
  exit(1);
}




double UserDataManager::GetRelativeCalibration(int detNum, int detType,
					       const string& channel) const {
  string chanName = channel;
  
  // get channel index
  uint index = ChannelMapHelper::GetChannelIndexBySide(detType, chanName);
  
  // get calibration vector
  vector<double> vect = ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,"P_RELATIVE_CALIBRATION");
  
  
  // check length of vector
  if (vect.size()==1) {
      return vect[0];
  } else if (vect.size()>index) {
    return vect[index];
  }

  // should not come until here
  cerr << "UserDataManager::GetRelativeCalibration: ERROR getting the relative calibration!" << endl;
  exit(1);
}

 
vector<double> UserDataManager::GetOverallCalibration(int detNum, const string& parName, vector<string> brokenChannels) const {
 
  
  // Key name 
  string keyName=parName;
  

  // Key name broken channels
  if (!brokenChannels.empty()) {
    sort(brokenChannels.begin(),brokenChannels.end());
    string keyBrokenChannels = brokenChannels[0];
    for (uint jj=1;jj<brokenChannels.size();jj++) 
            keyBrokenChannels = keyBrokenChannels + "_" + brokenChannels[jj];
  
    keyName = keyName + "_" + keyBrokenChannels;
  }
   

  // check parameter available 
  map<string, vector<double> > zipMap =  fZipMapOfMapVectDouble.find(detNum)->second;

  //check if specified in config file
  if (ListManager::HasParameter(zipMap,keyName)) { 
     return ListManager::GetParameter(zipMap,keyName);

  } else if (ListManager::HasParameter(zipMap,parName)) { 
   
     cout << "WARNING! UserDataManager::GetOverallCalibration:  No calibration '" << parName
          << "' with broken channels found for detector " << detNum << ", "
          <<" using regular calibration instead!"
          << endl;
 
     return ListManager::GetParameter(zipMap,parName);

  } else {

    cout <<"ERROR! UserDataManager::GetOverallCalibration:  Parameter '" << parName
         << "' not found for detector " << detNum << ", check your calibration file"
         << endl;
   
    exit(1);   
  }       

}







double UserDataManager::GetQCalibration(int detNum, int detType,
                 const string& channel) const {
  string chanName = channel;
  
  // get channel index
  uint index = ChannelMapHelper::GetChannelIndexBySide(detType, chanName);
  
  // get calibration vector
  vector<double> vect = ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,"Q_CALIBRATION_OF");
  
  
  // check length of vector
  if (vect.size()==1) {
      return vect[0];
  } else if (vect.size()>index) {
    return vect[index];
  }

  // should not come until here
  cerr << "UserDataManager::GetQCalibration: ERROR getting the charge OF calibration!" << endl;
  exit(1);
}



// A bit of a hack... since I don't understand the full details of the how the time-dependent
// calculation gets implemented in BatCalib, and since I have even less of an idea of how to
// access this information in BatRoot, we're going to be dumb and put approximate,
// time-independent calibrations directly in the BatRoot config file and then read them out. [AJA]
double UserDataManager::GetOverallPTCalibrationTI(int detNum, int detType) const {
  // get calibration vector
  vector<double> vect = ListManager::GetParameter(fZipMapOfMapVectDouble.find(detNum)->second,"PT_CALIBRATION_NF");

  return vect[0];
}


bool UserDataManager::DoNoiseMonitorProcessing(const string& chName) const
{
  //first check if it's enabled globally
  if(!ListManager::HasParameter(fMapVectInt,"DO_PROCESSING_NOISEMONITOR") ||
     ListManager::GetParameter(fMapVectInt,"DO_PROCESSING_NOISEMONITOR")[0] == 0)
    return false;
  if(chName=="") //only care about global
    return true;
  //now get the list of noise monitor detectors and see if detNum is in there
  vector<string> detList = GetVectStringParameter("NOISEMONITOR_CHANNELS");
  return std::count(detList.begin(), detList.end(), chName) > 0;
}

bool UserDataManager::WriteNoiseMonitorRQ(const string& chName) const
{
  //first check if it's enabled globally
  if(!ListManager::HasParameter(fMapVectInt,"WRITE_RQ_NOISEMONITOR") || 
     ListManager::GetParameter(fMapVectInt,"WRITE_RQ_NOISEMONITOR")[0] == 0)
    return false;
  if(chName=="") //only care about global
    return true;
  //now get the list of noise monitor detectors and see if detNum is in there
  vector<string> detList = GetVectStringParameter("NOISEMONITOR_CHANNELS");
  return std::count(detList.begin(), detList.end(), chName) > 0;
}







// Flag to use channel for Noise trace selection (through PT/QT construction)
bool UserDataManager::UseChannelNoiseSelection(int detNum, int detType,
					       const string& channel) const 


{
 // return true by default
  bool useChannel = true; 
    
  // get channel index
  string channelName = channel;
  uint index = ChannelMapHelper::GetChannelIndexBySide(detType, channelName);
  
  // get map
   map<string,vector<int> > zipMap =  fZipMapOfMapVectInt.find(detNum)->second;

  //  phonon
  string keyName;

  if(channelName[0] == 'P')
      keyName = "P_CHAN_NOISE_SELECT";
  else if (channelName[0] == 'Q')
      keyName = "Q_CHAN_NOISE_SELECT";
  else
      return useChannel;


    
  //check if specified in config file
  if (ListManager::HasParameter(zipMap,keyName)) {
        vector<int> vect = ListManager::GetParameter(fZipMapOfMapVectInt.find(detNum)->second,keyName);
        if (vect.size()==1) {
              useChannel = (bool)  vect[0];          
        } else if (vect.size()>index) {
              useChannel = (bool) vect[index];
        }

   }

   return useChannel;

}


// Detector Status
int UserDataManager::GetDetectorStatus(int detNum, const string& channel, const string&  inputSeries)
{
   // default channel is good
   int status = 0; 

   //check if specified in config file, return 0 if not available
    map<string,vector<string> > zipVectString;
   if (fZipMapOfMapVectString.find(detNum)== fZipMapOfMapVectString.end())
   	return status;


   // Loop through Map 
   int counter = 0;
   map<string,vector<string> >::const_iterator zipVectStringItr = (fZipMapOfMapVectString.find(detNum)->second).begin();
   for( ; zipVectStringItr!= (fZipMapOfMapVectString.find(detNum)->second).end(); zipVectStringItr++) {
      
      // Get key name
      string keyName = zipVectStringItr->first;
    
      // check correspond to channel
      if (keyName.find("DET_STATUS_" + channel) ==string::npos) continue;

      // Get parameters and convert string to number
      vector<string> valStr =  zipVectStringItr->second;
      int statusTemp = atoi(valStr[2].c_str());

      
      double seriesMin;
      string::size_type posUnderscore = valStr[0].find("_",0);
      if (!(posUnderscore == string::npos)) {
        string seriesMinStr = valStr[0].substr(0,posUnderscore) + valStr[0].substr(posUnderscore+1);
        seriesMin = atof(seriesMinStr.c_str());
      } else 
      seriesMin = atof(valStr[0].c_str());
   

      double seriesMax;
      posUnderscore = valStr[1].find("_",0);
      if (!(posUnderscore == string::npos)) {
       string seriesMaxStr = valStr[1].substr(0,posUnderscore) + valStr[1].substr(posUnderscore+1);
       seriesMax = atof(seriesMaxStr.c_str());
      } else 
      seriesMax = atof(valStr[1].c_str());
   

      double seriesCurrent;
      posUnderscore = inputSeries.find("_",0);
      if (!(posUnderscore == string::npos)) {
       string inputSeriesStr = inputSeries.substr(0,posUnderscore) + inputSeries.substr(posUnderscore+1);
       seriesCurrent = atof(inputSeriesStr.c_str());
      } else 
      seriesCurrent = atof(inputSeries.c_str());
  
 
      // check if input Series is between seriesMin 
      // and seriesMax
   
      if (seriesCurrent>=seriesMin && seriesCurrent<=seriesMax) {
        if (counter==0)
           return statusTemp;
        else {
           cout <<"\nERROR in UserDataManager::GetDetectorStatus!  Multiple status information for  detector " << detNum
	        << ", channel " << channel << "! Check status file..." << endl;
	   exit(1);
         }    

        counter++;   
       }
    }

 return status;
}
  
// Modification of Raw data
bool  UserDataManager::DoModifyRawData() const 
{
   
   string keyName = "MODIFY_RAWDATA";

   //check if specified in config file
   if (ListManager::HasParameter(fMapVectInt,keyName))
    { 
     vector<int> intVect = ListManager::GetParameter(fMapVectInt,keyName);  
     return (bool) intVect[0] ;
    } else {
     return false;
    }
}



// Modification map

map<int,string> UserDataManager::GetRawDataModificationMap() const
{ 
  // define map
  map<int,string> modificationMap;
  
  // Loop map
  map<int, map<string,vector<string> > >::const_iterator zipMapOfMapVectStringItr = fZipMapOfMapVectString.begin();
  for( ; zipMapOfMapVectStringItr!=fZipMapOfMapVectString.end(); zipMapOfMapVectStringItr++) {

    // detector number
    int detNum = zipMapOfMapVectStringItr->first;
     
    // loop map for detector "detNum"
    map<string,vector<string> >::const_iterator zipVectStringItr = (zipMapOfMapVectStringItr->second).begin();
    for( ; zipVectStringItr!=(zipMapOfMapVectStringItr->second).end(); zipVectStringItr++) {

      string keyName = zipVectStringItr->first;
      if (keyName=="MODIFICATION_TYPE") {
        string modificationType = (zipVectStringItr->second)[0];
        modificationMap.insert(pair<int,string>(detNum,modificationType));
      }
    }
   }


  return modificationMap;

}

      




//  ___________________ read configuration file ______________________

void  UserDataManager::ReadFile(string filename, bool overwriteFlag) {
  // check if file exist
  ifstream file(filename.c_str());
  if (!file) { 
    cerr << "UserDataManager::ReadFile: ERROR! file '" << filename <<"' not found..." << endl;
    exit(1);
  }

  cout << "\n**** Reading user settings file: " << filename << endl;
   
  // store file names for RQ output
  fFileNameList.push_back(filename);
  
  //  a few declarations 
  string line;
  int lineNumber = 0;
  while (getline(file,line)) {
    lineNumber++;
    ProcessLine(line, overwriteFlag, filename, lineNumber);
  }
  // construct RQ list 
  ConstructConfigRQList();
  
  return;
}

void UserDataManager::ProcessLine(string line, bool overwriteFlag,
				  string filename, int lineNumber){
  if (! line.length())   return;  // skip empty lines
  if (line[0] == '#')    return; // skip comments 
    
  string key ;         // parameter name 
  string keyType ;     // parameter description 
  string detectorStr ;    // detector number(s) string  (DETECTOR #,#-#) 
  
  const char whitespace[] = " \n\t\v\r\f";
  string::size_type  posNotEmpty;
  
  // check cases for which there are whitespaces before any character 
  posNotEmpty = line.find_first_not_of(whitespace);
  if (trim(line.substr(0,posNotEmpty+1))=="#") return;
  if (posNotEmpty == string::npos) return;
    
    
  // find position of "=" 
  string::size_type  posEqual = line.find('=');
    
  if (posEqual == string::npos) { 
    cerr << "UserDataManager::ReadFile: ERROR!  syntax error line "<< lineNumber <<": need to add a '=' character" << endl;
    exit(1);
  }
  
  // split line before/after the  position of "="
  string strBeforeEqual = line.substr(0,posEqual);  
  string strAfterEqual  = line.substr(posEqual+1);  

  // ---------- String before "equal" -----------
  string::size_type posDetector = strBeforeEqual.find("DETECTOR",0);

  // key string (key + key type)
  string keyLineStr;
  if ( !(posDetector == string::npos) ) {  
    // ZIP detector parameters
    // split line before/after position 'DETECTOR' 
    string strBeforeDetector = strBeforeEqual.substr(0,posDetector);
    string strAfterDetector  = strBeforeEqual.substr(posDetector);
      
    // detector 
    detectorStr = trim(strAfterDetector);
      
    // string with keys
    keyLineStr = trim(strBeforeDetector);
  } else {
    // string with keys
    keyLineStr = trim(strBeforeEqual);
  }

  // tokenize the keyLineStr to get key and keyType 
  vector<string>  strTokenVect = Tokenize(keyLineStr);

  if (strTokenVect.size()==1) {
    keyType = strTokenVect[0];
  } else if (strTokenVect.size()==2) {
    keyType = strTokenVect[0];
    key = strTokenVect[1];
  } else {
    cerr << "UserDataManager::ReadFile: ERROR!  syntax error line "<< lineNumber <<": too many parameters!" << endl;
    exit(1);
  }
   
  //  extract detector list 
  list<int> detectorList;
    
  if ( !(detectorStr.empty()) )
    detectorList = ReadDetectorList(detectorStr, lineNumber);

  // ---------- String "after equal"  --------------
  if (trim(strAfterEqual).empty()) {
    cerr << "UserDataManager::ReadFile: ERROR!  syntax error line "<< lineNumber <<": need a value after '=' !" << endl;
    exit(1);
  }

  // remove comments and get value
  string::size_type posComment = strAfterEqual.find("#",0);
  if (!(posComment == string::npos))
    strAfterEqual = strAfterEqual.substr(0,posComment);
 
  // tokenize the strAfterEqual
  vector<string> valueStrVect = Tokenize(strAfterEqual);
 
  // check that there is more than 0 element
  if (valueStrVect.empty()) {
    cerr << "UserDataManager::ReadFile: ERROR!  No number found line "<< lineNumber <<"!" << endl;
    exit(1);
  }

  //  check "$" exist exist, replace enviremental variables 
  string::size_type posDollar = valueStrVect[0].find("$",0);

  if (!(posDollar == string::npos)) {
    string::size_type slashPos = valueStrVect[0].find_first_of("/",posDollar);
      
    string EnvVar;
    string restPath;
      
    if (!(slashPos == string::npos)) {
      EnvVar = valueStrVect[0].substr(posDollar+1,slashPos-posDollar-1);
      restPath = valueStrVect[0].substr(slashPos);
    } else {      
      EnvVar = valueStrVect[0].substr(posDollar+1);
    }

    if(getenv(EnvVar.c_str()) == NULL) {
      cerr <<"UserDataManager::ReadFile: ERROR! environmental variable " << EnvVar << " not set." << endl;
      exit(1);
    } 

    valueStrVect[0] = getenv(EnvVar.c_str()) + restPath;
  }  
    
  //see if it's an include statement
  if(keyType == "INCLUDE_FILE"){
    if(key != "UserSettings"){
      cerr<<"UserDataManager::ReadFile: ERROR! Only known key for "
	  <<"INCLUDE_FILE type is 'UserSettings'" <<endl;
      exit(1);
    }
    //including files only works if we're allowed to overwrite
    overwriteFlag = true;
    //the included file should have a path relative to this one, so find ours
    size_t last_slash = filename.find_last_of('/');
    string path = "";
    if(last_slash != std::string::npos)
      path = filename.substr(0,last_slash+1);
    for(size_t i=0; i<valueStrVect.size(); ++i)
      ReadFile(path+valueStrVect[i], overwriteFlag);
  }
    
  // --------- now store the informations ----------- 
  StoreParameter(key, keyType, detectorList ,valueStrVect, lineNumber,overwriteFlag);
  
  key.clear();
  keyType.clear();
  detectorList.clear();
  valueStrVect.clear();
  detectorStr.clear();
}  // end ProcessLine
 

// ........................................................

list<int> UserDataManager::ReadDetectorList(string detector,int lineNumber) {
  // Get the list of detector from the string such as "DETECTOR #-# , #"

  // check if there is any number specify
  string digits = "0123456789";
  string::size_type posNumber = detector.find_first_of(digits);
  
  if (posNumber == string::npos) { 
    cerr << "UserDataManager::ReadDetectorList: ERROR!  line " << lineNumber << ": need to specify detector number!" <<  endl;
    exit(1);
  }
  
  // get number string without "DETECTOR" string
  string detectorNumber = trim(detector.substr(posNumber));
  
  // let's create first a list without the commas so only #-# and # 
  list<string> DetNumberStrList;
  
  // check if "," exist
  if (detectorNumber.find(",",0)!= string::npos) {   
    // loop through all occurences of "," and extract number in between or before the comma
    for (string::size_type ii = 0, posComma; (posComma=detectorNumber.find(",",ii)) != string::npos; ii = posComma+1) {
      //extract string inbetween ","
      string strInBetweenComma = detectorNumber.substr(ii,posComma-ii);
      DetNumberStrList.push_back(trim(strInBetweenComma));   
      
      // if last comma, then extract last string
      if (posComma == detectorNumber.find_last_of(","))
	DetNumberStrList.push_back(trim(detectorNumber.substr(posComma+1)));
    }
  } else {
    DetNumberStrList.push_back(detectorNumber);
  }
  
  // will now fill a vector<int> with all the detector numbers
  list<int> DetNumberIntList;
  
  // need loop through the list and separate case with "-" or not
  list<string>::iterator it;
  for (it = DetNumberStrList.begin(); it !=DetNumberStrList.end(); it++) {
    string strDet = *it;
    string::size_type posDash = strDet.find("-",0);
    
    if (posDash != string::npos) {
      // extract string before and after "-"
      string strDetFirst = strDet.substr(0,posDash);
      string strDetLast = strDet.substr(posDash+1);
      
      // convert to integer
      int  intDetFirst =   atoi(strDetFirst.c_str());
      int  intDetLast =    atoi(strDetLast.c_str()); 
      
      // check if first>last
      if (intDetFirst>intDetLast) {
	cerr<<"UserDataManager::ReadDetectorList: ERROR!  line "  << lineNumber << ": Syntax needs to be: det #a - det #b, WITH  a<b  ! " << endl;
	exit(1);
      }
      
      // fill vector
      for (int i = intDetFirst; i<= intDetLast; i++)
	DetNumberIntList.push_back(i);
    } else {
      // no dash sign, only convert string to int and fill vector
      DetNumberIntList.push_back(atoi(strDet.c_str()));
    }
  }
  
  return DetNumberIntList;
}


// ........................................................

string UserDataManager::trim(string str) {
 const char whitespace[] = " \n\t\v\r\f";
 str.erase(0,str.find_first_not_of(whitespace));
 str.erase(str.find_last_not_of(whitespace) +1U);

 return str;
}

vector<string> UserDataManager::Tokenize(string aStr) {
  vector<string> tokens;
  string token;

  const char whitespace[] = " \n\t\v\r\f";

  string::size_type lastPos = aStr.find_first_not_of(whitespace,0);
  string::size_type pos = aStr.find_first_of(whitespace,lastPos);
 
  while (string::npos !=pos || string::npos != lastPos)
   {
     // find a token, add it to vector (in lower case) 
     token = trim(aStr.substr(lastPos, pos - lastPos));
   //  transform(token.begin(),token.end(),token.begin(),(int(*)(int)) tolower);
     tokens.push_back(token);
    
     // skip whitespace
     lastPos = aStr.find_first_not_of(whitespace,pos);
     
     // find next non-whitespace
     pos = aStr.find_first_of(whitespace,lastPos);
   }
 
  return tokens;
}

//  ___________________ store configuration parameters  ______________________

void UserDataManager::StoreParameter(string key,string keyType, list<int> detectorList,
				     vector<string> valueStrVect, int lineNumber, bool overwriteFlag) {
  if (detectorList.empty()) {     
    // path 
    if (keyType=="PATH" || keyType=="PREFIX") {
      // check that there is only one element
      if (valueStrVect.size()!=1) {
	cerr << "UserDataManager::StoreParameter: ERROR!  line " << lineNumber << ": must have only one string!"<< endl;
	exit(1);
      }
      
      // store path
      string keyRecord = keyType + "_" + key;
      
      if (valueStrVect[0].find_last_of("/") != valueStrVect[0].length()-1 && keyType=="PATH")
	valueStrVect[0] = valueStrVect[0] + "/";
      
      SetStringParameter(keyRecord,valueStrVect[0],overwriteFlag);
    }
    
    if (keyType=="READ" || keyType=="DO_PROCESSING" || keyType=="WRITE_RQ") {
      // check value are 0 or 1
      if ( !(valueStrVect[0]=="0" || valueStrVect[0]=="1")) {
	cerr << "UserDataManager::StoreParameter: ERROR!  line " << lineNumber << ": please, set '"<< key 
	     <<"'  to 0 or 1 !" << endl;
	exit(1);
      }
      
      // convert to int
      vector<int> valueIntVect;
      for (uint ii=0;ii<valueStrVect.size(); ii++)
	{
	  int intValue = atoi(valueStrVect[ii].c_str());
	  valueIntVect.push_back(intValue);
	}
      
      // store
      string keyRecord = keyType + "_" + key;
      SetVectIntParameter(keyRecord, valueIntVect,overwriteFlag);
    }
    
    if (keyType=="PARAMETER_INTEGER") {
      vector<int> valueIntVect;
      for (uint ii=0;ii<valueStrVect.size(); ii++) {
	int intValue = atoi(valueStrVect[ii].c_str());
	valueIntVect.push_back(intValue);
      }
      
      SetVectIntParameter(key,valueIntVect,overwriteFlag);
    }
    
    if (keyType=="PARAMETER_DOUBLE") {
      vector<double> valueDoubleVect;
      for (uint ii=0;ii<valueStrVect.size(); ii++) { 
	double doubleValue = atof(valueStrVect[ii].c_str());
	valueDoubleVect.push_back(doubleValue);
      }
      
      SetVectDoubleParameter(key,valueDoubleVect,overwriteFlag);
    }
    
    if (keyType=="PARAMETER_STRING")      
      SetVectStringParameter(key,valueStrVect,overwriteFlag);
    
  } else {	// detectorList NOT empty()
    list<int>::iterator iterList;       
    for (iterList = detectorList.begin(); iterList !=detectorList.end(); iterList++) {
      int detNum =  *iterList;   

      if (keyType=="DET_SUBSTRATE") {   
	if ( (valueStrVect[0]=="Ge") || (valueStrVect[0]=="Si")) {
	  SetStringParameter(detNum, keyType,valueStrVect[0],overwriteFlag);
	} else {
	  cerr << "UserDataManager::StoreParameter: ERROR!  line " << lineNumber << ": please, set "
	       << key << "  to 'Ge' or 'Si' !" << endl;
	  exit(1);
	}
      }

      if (keyType=="DET_TOWER") 
	SetStringParameter(detNum,keyType,valueStrVect[0],overwriteFlag);
	
      if (keyType =="DO_CHARGE_ALGORITHM" || keyType =="DO_PHONON_ALGORITHM" ||
	  keyType =="DO_PHONON_TOT" || keyType =="DO_PHONON_SIDES" ||
	  keyType =="DO_ALGORITHM" || keyType=="DO_PROCESSING" || keyType=="WRITE_RQ" ||
	  keyType=="CALC_CHARGE_TEMPLATE" || keyType=="CALC_PHONON_TEMPLATE") {
	// check value are 0 or 1
	if ( ! (valueStrVect[0]=="0"  || valueStrVect[0]=="1"))
	  {
	    cerr << "UserDataManager::StoreParameter: ERROR!  line " << lineNumber << ": please, set '"<< keyType 
		 <<"'  to 0 or 1 !!" << endl;
	    exit(1);
	  }

	// convert to int
	vector<int> valueIntVect;
	for (uint ii=0;ii<valueStrVect.size(); ii++) {
	  int intValue = atoi(valueStrVect[ii].c_str());
	  valueIntVect.push_back(intValue);
	}
 
	// store
	string keyRecord = keyType;

	if (keyType =="DO_CHARGE_ALGORITHM") 
	  keyRecord = "charge_alg_" + key;

	if ( keyType =="DO_PHONON_ALGORITHM")
	  keyRecord = "phonon_alg_" + key;
	
	if ( keyType =="DO_PHONON_TOT")
	  keyRecord = "PT_alg_" + key;
	
	if ( keyType =="DO_PHONON_SIDES")
	  keyRecord = "PSIDES_alg_" + key;
	
	if (keyType =="DO_ALGORITHM") 
	  keyRecord = "ALG_" + key;
           
	SetVectIntParameter(detNum, keyRecord, valueIntVect,overwriteFlag);
      }

      // Detector Status
      if (keyType =="DET_STATUS") {

	// check number input
	if (valueStrVect.size()!=3)
	  {
	    cerr << "UserDataManager::StoreParameter: ERROR!  DET_STATUS should have 3 parameters "
		 <<"(SERIESNUMBER_MIN, SERIESNUMBER_MAX, and STATUS)" << endl;
	    exit(1);
	  }

        // there might be multiple parameter so store with lineNumber
        string lineStr;
        ostringstream convert;
        convert << lineNumber;
        lineStr = convert.str();

	string keyRecord = "DET_STATUS_" + key  + "_" + lineStr;
	SetVectStringParameter(detNum,keyRecord,valueStrVect,overwriteFlag);
      }

      // other parameters
      if (keyType=="PARAMETER_INTEGER") {
	vector<int> valueIntVect;
	for (uint ii=0;ii<valueStrVect.size(); ii++) {
	  int intValue = atoi(valueStrVect[ii].c_str());
	  valueIntVect.push_back(intValue);
         }

        SetVectIntParameter(detNum,key,valueIntVect,overwriteFlag);
      }

      if (keyType=="PARAMETER_DOUBLE") {

         // double check if the values include P or Q channel
         // names
         vector<string> brokenChannels; 
         vector<double> valueDoubleVect;

         for (uint ii=0;ii<valueStrVect.size(); ii++) {
            string valueStr =  valueStrVect[ii];
            if (valueStr.compare(0,1,"P")==0)
                brokenChannels.push_back(valueStr);
	    else {
                double doubleValue = atof(valueStr.c_str());
                valueDoubleVect.push_back(doubleValue);
            }
         }

        if (!brokenChannels.empty()) {
          // sort vector
          sort(brokenChannels.begin(),brokenChannels.end());
          string keyBrokenChannels = brokenChannels[0];
          for (uint jj=1;jj<brokenChannels.size();jj++) 
            keyBrokenChannels = keyBrokenChannels + "_" + brokenChannels[jj];
  
          key = key + "_" + keyBrokenChannels;
        }
   
        SetVectDoubleParameter(detNum,key,valueDoubleVect,overwriteFlag);
      }
            
      if (keyType=="PARAMETER_STRING")     
	SetVectStringParameter(detNum,key,valueStrVect,overwriteFlag);
    } // end loop list detectors
  } // end if detector list 
};


// _________________  RQ list ________________________

void UserDataManager::ConstructZipRQList(const map<int,int>& detectorMap) {
 
  double initVal = -999999.;
  
  fZipRQList.clear(); //just in case this is called multiple times
  // loop ZIPs
  map< int, int >::const_iterator it;
  for(it = detectorMap.begin(); it!=detectorMap.end(); it++)
   {
     
     // detector number/type
     int detNum = it->first;
     int detType = it->second;
     
     // map RQs (same for each detector)
     map<string, double> mapRQ;

     // LEDs
     mapRQ.insert(pair<string,double>("LEDstatus", initVal));

     // Channels
     vector<string> channelNameList;
     ChannelMapHelper::FillAllChannelList(detType,channelNameList);
     int listSize = (int) channelNameList.size();
     
     for (int ii=0; ii<listSize; ii++) 
       mapRQ.insert(pair<string,double>(channelNameList[ii] + "status", initVal));
     
	
     fZipRQList.insert(pair<int, map<string, double> >(detNum, mapRQ));
   }


  return;
}


void UserDataManager::ConstructConfigRQList() {
  // ------ double parameters ------

  // veto/Trigger/processing  specific parameters
  map<string,vector<double> >::const_iterator mapVectDoubleItr = fMapVectDouble.begin();
  for( ; mapVectDoubleItr!=fMapVectDouble.end(); mapVectDoubleItr++) {
    fVectDoubleRQList.insert(pair<string,vector<double> >(mapVectDoubleItr->first, mapVectDoubleItr->second));
  }

  // ZIP specific parameter
  // loop zips map
  map<int, map<string,vector<double> > >::const_iterator zipMapOfMapVectDoubleItr = fZipMapOfMapVectDouble.begin();
  for( ; zipMapOfMapVectDoubleItr!=fZipMapOfMapVectDouble.end(); zipMapOfMapVectDoubleItr++) {
    int detNum = zipMapOfMapVectDoubleItr->first;
     
    // loop map for detector "detNum"
    map<string,vector<double> >::const_iterator zipVectDoubleItr = (zipMapOfMapVectDoubleItr->second).begin();
    for( ; zipVectDoubleItr!=(zipMapOfMapVectDoubleItr->second).end(); zipVectDoubleItr++) {
      string parName = zipVectDoubleItr->first;
      vector<double> valueVect = zipVectDoubleItr->second;
      
      // case only 1 value for the parameter "parName"
      if (valueVect.size()==1){
	// check if name exist
	if (!(fVectDoubleRQList.find(parName)==fVectDoubleRQList.end())) { 
	  // just add intoRQ List
	  (fVectDoubleRQList.find(parName)->second)[detNum-1]=valueVect[0];
	} else {
	  // initialize a vector
	  vector<double> zipVectDouble;
	  for(uint zipItr=0; zipItr<fZipMapOfMapVectDouble.size(); zipItr++) {
	    zipVectDouble.push_back(-999999.);
	  }
               
	  // add value into vector
	  zipVectDouble[detNum-1]= valueVect[0];
          
	  // insert vector into QRList
	  fVectDoubleRQList.insert(pair<string,vector<double> >(parName, zipVectDouble));
	}
      } else {	// case mode than 1 value  for the parameter "parName"   
	// loop vector valueVect
	for (uint valItr=0;valItr<valueVect.size();valItr++) {
	  //  form a new name
	  const char base[]="_";
	  char parNameTemp[20];
	  sprintf(parNameTemp,"%s%d",base,valItr+1);
	  string parNameNew = parName + parNameTemp;
	  
	  // check if name exist
	  if (!(fVectDoubleRQList.find(parNameNew)==fVectDoubleRQList.end())) { 
	    // just add intoRQ List
	    (fVectDoubleRQList.find(parNameNew)->second)[detNum-1]=valueVect[valItr];
	  } else {
	    // initialize a vector
	    vector<double> zipVectDouble;
	    for(uint zipItr=0; zipItr<fZipMapOfMapVectDouble.size(); zipItr++) {
	      zipVectDouble.push_back(-999999.);
	    }
	    
	    // add value into vector
	    zipVectDouble[detNum-1]= valueVect[valItr];
	    
	    // insert vector into QRList
	    fVectDoubleRQList.insert(pair<string,vector<double> >(parNameNew, zipVectDouble));
	  }
	} // end loop valueVect
      } // end case valueVect=1 or >1
    } // end map ZIP
  } // end zipMapofMap

  // ------ int parameters ------
  // veto/Trigger/processing  specific parameters

  map<string,vector<int> >::const_iterator mapVectIntItr = fMapVectInt.begin();
  for( ; mapVectIntItr!=fMapVectInt.end(); mapVectIntItr++) {
    fVectIntRQList.insert(pair<string,vector<int> >(mapVectIntItr->first, mapVectIntItr->second));
  }

  // ZIP specific parameter
  // loop zips map
  map<int, map<string,vector<int> > >::const_iterator zipMapOfMapVectIntItr = fZipMapOfMapVectInt.begin();
  for( ; zipMapOfMapVectIntItr!=fZipMapOfMapVectInt.end(); zipMapOfMapVectIntItr++) {
    int detNum = zipMapOfMapVectIntItr->first;
     
    // loop map for detector "detNum"
    map<string,vector<int> >::const_iterator zipVectIntItr = (zipMapOfMapVectIntItr->second).begin();
    for( ; zipVectIntItr!=(zipMapOfMapVectIntItr->second).end(); zipVectIntItr++) {
      string parName = zipVectIntItr->first;
      vector<int> valueVect = zipVectIntItr->second;
      
      // case only 1 value for the parameter "parName"
      if (valueVect.size()==1) {
	// check if name exist
	if (!(fVectIntRQList.find(parName)==fVectIntRQList.end())) { 
	  // just add intoRQ List
	  fVectIntRQList.find(parName)->second[detNum-1]=valueVect[0];
	} else {
	  // initialize a vector
	  vector<int> zipVectInt;
	  for(uint zipItr=0; zipItr<fZipMapOfMapVectInt.size(); zipItr++) {
	    zipVectInt.push_back(-999999);
	  }

	  // add value into vector
	  zipVectInt[detNum-1]= valueVect[0];
          
	  // insert vector into QRList
	  fVectIntRQList.insert(pair<string,vector<int> >(parName, zipVectInt));
	}
      } else {         // case mode than 1 value  for the parameter "parName"
	// loop vector valueVect
	for (uint valItr=0;valItr<valueVect.size();valItr++) {
	  //  form a new name
	  const char base[]="_";
	  char parNameTemp[20];
	  sprintf(parNameTemp,"%s%d",base,valItr+1);
	  string parNameNew = parName + parNameTemp;

	  // check if name exist
	  if (!(fVectIntRQList.find(parNameNew)==fVectIntRQList.end())) { 
	    // just add intoRQ List
	    fVectIntRQList.find(parNameNew)->second[detNum-1]=valueVect[0];
	  } else {
	    // initialize a vector
	    vector<int> zipVectInt;
	    for(uint zipItr=0; zipItr<fZipMapOfMapVectInt.size(); zipItr++) {
	      zipVectInt.push_back(-999999);
	    }
	    
	    // add value into vector
	    zipVectInt[detNum-1]= valueVect[valItr];
	    
	    // insert vector into QRList
	    fVectIntRQList.insert(pair<string,vector<int> >(parNameNew, zipVectInt));
	  }
	} // end loop valueVect
      } // end case valueVect=1 or >1
    } // end map ZIP
  } // end zipMapofMap
}



// _________________  Calc ZIP RQs  ________________________

void UserDataManager::DoCalcDetectorStatus(const string &inputSeries) {
  
  // check RQ list has been constructed
  if (fZipRQList.empty()) {
    cerr << "UserDataManager::DoCalcDetectorStatus: ERROR!  RQ list empty! Shouldn't be the case." <<
      "Disable DET_STATUS_FILE if no detectors are being processed"<< endl;
    exit(1);
  }
  
  // Loop RQ list a nd fill RQs
  map<int, map<string,double> >::iterator zipRQListItr = fZipRQList.begin();
  for( ; zipRQListItr!=fZipRQList.end(); zipRQListItr++)
    {
      int detNum = zipRQListItr->first;
      map<string,double>::iterator rqListItr = (zipRQListItr->second).begin();
      for( ; rqListItr!=(zipRQListItr->second).end(); rqListItr++)
	{
	  // get channel name
	  string RQname = rqListItr->first;
	  unsigned pos = RQname.find("status");
	  string channel = RQname.substr(0,pos);
	  
	  
	  // fill RQ
          rqListItr->second = (double) GetDetectorStatus(detNum,channel,inputSeries);
     
	}	    
    }
  
  return;
}


void  UserDataManager::FillBrokenChannelLists(){

 //
 // Fill list using RQs
 //

 // no filling list if status file not read
 if (fZipRQList.empty()) return;
 
 // Loop map  
 map<int, map<string, double> >::iterator mapIter;
 for (mapIter = fZipRQList.begin(); mapIter!=fZipRQList.end();++mapIter) 
    { 
  
      int detNum = mapIter->first;
      map<string, double> statusMap = mapIter->second;
  
      // define vectors
      vector<string> brokenVect;
      vector<string> brokenPhononVect;
      vector<string> brokenChargeVect;
      vector<string> brokenSideVect;

      // loop channels to get status
      map<string, double>::iterator chanIter;
      for (chanIter = statusMap.begin(); chanIter!=statusMap.end();++chanIter) {
        string RQname =  chanIter->first;
        string chanName = RQname.substr(0,RQname.find("status"));
      
        int status = (int) chanIter->second;
  
        // phonons
        if (chanName[0] == 'P' && status==2) {
           brokenPhononVect.push_back(chanName);
           brokenVect.push_back(chanName);
        }

        //charge
        if (chanName[0] == 'Q' && status>0) {

          // fill channel list
          brokenChargeVect.push_back(chanName);
          brokenVect.push_back(chanName);

          // side list
          string side = "S";
	  if (chanName.find("S1") !=string::npos) side = "S1";
	  if (chanName.find("S2") !=string::npos) side = "S2";
	
          bool isPresent = (find(brokenSideVect.begin(), brokenSideVect.end(), side) != brokenSideVect.end());
          if (!isPresent) brokenSideVect.push_back(side);

	} 
      }

      // fill map
      fBrokenChannelMap.insert(pair<int, vector<string> >(detNum, brokenVect));
      fBrokenPhononChannelMap.insert(pair<int, vector<string> >(detNum, brokenPhononVect));
      fBrokenChargeChannelMap.insert(pair<int, vector<string> >(detNum, brokenChargeVect));
      fBrokenChargeSideMap.insert(pair<int, vector<string> >(detNum, brokenSideVect));
 }
 
 return;

}




// _______________________ set parameters __________________________

template <class Type>
void UserDataManager::SetTypeParameter(map<string,Type>& aMapT, const string& varName,
				       Type val, bool overwriteFlag) {
  // Set parameters in map according to its type
  
  if (! ListManager::HasParameter(aMapT,varName) || overwriteFlag==true) {  
    ListManager::SetParameter(aMapT, varName, val);
  } else {
    cerr << "UserDataManager::SetTypeParameter: ERROR! Parameter '" << varName << "' already set!" << endl;
    exit(1);  
  }
} 


// ...................................................................................

template <class Type>
void UserDataManager::SetTypeParameter(map<int,map<string,Type> >& aMapOfMapT, int detNum,
				       const string& varName, Type val, bool overwriteFlag) {
  // -- retrieve the map for detector "detNum" ---
  typename map<int,map<string,Type> >::iterator zipMap = aMapOfMapT.find(detNum);

  // --- case ZIP map doesn't exist yet ---
  if(zipMap == aMapOfMapT.end()) {  
    map<string, Type> newMap;
    newMap.insert(pair<string,Type>(varName,val));
    aMapOfMapT.insert(pair<int, map<string, Type> >(detNum, newMap));
    
    return;     
  } 

  // ---- case ZIP map exist ---
  // check if parameter has been set already
  if((zipMap->second).count(varName) == 1 && overwriteFlag == false) {   
    cerr <<"UserDataManager:SetTypeZipParameter: ERROR! parameter " << varName << " for detector "<< detNum << " already set!"<< endl;
    exit(1);
  }

  // set parameter
  (zipMap->second)[varName] = val;
}


//  trick to be able to  put the template function inside the source instead of the header...
//  Here's a reference http://stackoverflow.com/questions/972152/how-to-create-a-template-function-within-a-class-c [ANV]
//  Basic idea -- if a SPECIFIC implementation is not instantiated or called within the .cxx code you compile into the object
//  (.o) file, this object file will lack the symbol defining this function and therefore whatever links against it will have
//  and undefined reference.  
//  Honestly, because of the above facts, I'm not sure it makes sense to think of these as "template functions" they are more like
//  overloaded functions in which you used a trick to save some typing.  -- if you don't have a member below with the explicit signature
//  that you are going to use, you can't use the function [ANV].
//  It's even more annoying because if you actually use a specific template signture in this .cxx file it will be included in the (.o) file
//  but it will be unstable in the sense that if you remove that call in the class implementation, then try to use that signature externally
//  you will have an undefined reference, AND you will go crazy looking for it because you didn't change the code which the compiler reports
//  as having the undefined reference. [ANV]

template void UserDataManager::SetTypeParameter<string>(map<string,string>& aMapT, const string& varName, string val, bool overwriteFlag);
template void UserDataManager::SetTypeParameter<string>(map<int, map<string,string> >& aMapOfMapT, int detNum, const string& varName, string val, bool overwriteFlag);

template void UserDataManager::SetTypeParameter<vector<int> >(map<string,vector<int> > & aMapT, const string& varName, vector<int>  valVect, bool overwriteFlag);
template void UserDataManager::SetTypeParameter<vector<int> >(map<int, map<string,vector<int> > >& aMapOfMapT, int detNum, const string& varName, vector<int>  valVect, bool overwriteFlag);

template void UserDataManager::SetTypeParameter<vector<double> >(map<string,vector<double> > & aMapT, const string& varName, vector<double>  valVect, bool overwriteFlag);
template void UserDataManager::SetTypeParameter<vector<double> >(map<int, map<string,vector<double> > >& aMapOfMapT, int detNum, const string& varName, vector<double>  valVect, bool overwriteFlag);

