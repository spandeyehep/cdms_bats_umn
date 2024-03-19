///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: DmmDataManager
//Authors: B. Serfass
//Description:  This class read the DMM file, store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
#include <cstdlib>


#include "DmmDataManager.h"


////////////////////////////////////////////////////////



DmmDataManager::DmmDataManager(const map< int, int >& detectorMap) : 
  fStoreRQs(true), 
  fDetectorMap(detectorMap)
{ 
  
   //Construct the RQ list
   ConstructZipRQList();
}

DmmDataManager::DmmDataManager() 
{ 


}


 
DmmDataManager::~DmmDataManager() 
{ 

}
      


//  =================  RQ list  =================


//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void DmmDataManager::ConstructZipRQList()
{
  double initVal = -999999.;

   
  // loop ZIPs
  map< int, int >::iterator it;
  for(it = fDetectorMap.begin(); it!=fDetectorMap.end(); it++)
   {
        
   // detector number
   int detNum = it->first;


   // map RQs (same for each detector)
   map<string, double> mapRQ;
     
   mapRQ.insert(pair<string,double>("PASqOffset", initVal));
   mapRQ.insert(pair<string,double>("PBSqOffset", initVal));
   mapRQ.insert(pair<string,double>("PCSqOffset", initVal));
   mapRQ.insert(pair<string,double>("PDSqOffset", initVal));
   mapRQ.insert(pair<string,double>("TrigThreshQHi", initVal));
   mapRQ.insert(pair<string,double>("TrigThreshQLo", initVal));
   mapRQ.insert(pair<string,double>("TrigThreshPHi", initVal));
   mapRQ.insert(pair<string,double>("TrigThreshPLo", initVal));
   mapRQ.insert(pair<string,double>("TrigThreshWisp", initVal));
   
   fZipRQList.insert(pair<int, map<string, double> >(detNum, mapRQ));
   }

   return;
}

void DmmDataManager::ResetRQList()
{
  
   ResetZipRQList();
   return;
}


void DmmDataManager::ResetZipRQList()
{
  
  double initVal = -999999.;
  
  map<int, map<string,double> >::iterator zipRQListItr = fZipRQList.begin();
  for( ; zipRQListItr!=fZipRQList.end(); zipRQListItr++)
   {
     map<string,double>::iterator rqListItr = (zipRQListItr->second).begin();
     for( ; rqListItr!=(zipRQListItr->second).end(); rqListItr++)
      {
       rqListItr->second = initVal;
      }
   }

   return;
}



//  =================  DO calc  =================



void DmmDataManager::DoCalc(const int& detNum, const double& eventTime)
 {
  // get value for all parameters and fill RQs
 
  // first Get map of the detector "detNum" 
  map< int, map<string,double> >::const_iterator zipMapItr = fZipMapOfMapDouble.find(detNum);

  // check map exist
  if(zipMapItr == fZipMapOfMapDouble.end())
    { 
      cerr <<"DmmDataManager::DoCalc: ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
      exit(1);
    } 

   map<string,double> zipMap = zipMapItr->second;
 
  
  // get last DMM time
  double lastDmmTime = GetLastDmmTime(zipMap, eventTime);

  // check if lastDmmtime exist

  if (lastDmmTime==0)
          return;


  // convert int to string
  const char base[]="@";
  char AtTimeStamp[20];
  sprintf(AtTimeStamp,"%s%f",base,lastDmmTime);



 
  // retrieve values ans store RQS

  if(fStoreRQs) 
    {
     // retrieve RQ list for "detNum"
     map<int, map<string,double> >::iterator zipRQListItr = fZipRQList.find(detNum);
    
     map<string,double>::iterator rqListItr = (zipRQListItr->second).begin();
     for( ; rqListItr!=(zipRQListItr->second).end(); rqListItr++)
      {
        string RQName = rqListItr->first;
        string keyName = RQName +  AtTimeStamp ;
        map<string,double>::const_iterator  zipMapItr = zipMap.find(keyName);
       
        // fill RQ if found value else try to find last  Dmm Time for the particular RQ name
        if (zipMapItr != zipMap.end()) {
            rqListItr->second = zipMapItr->second;
        }
        else
        {  
            map<string,double> valMap = GetVal(zipMap,RQName,eventTime);
            rqListItr->second = valMap.find(RQName)->second;

         }

       }
    }
  
  return;

}


//  =================  Get functions  =================



double DmmDataManager::GetTriggerThreshold(const int& detNum, const string& trigName, const double& eventTime) const
 {
   string keyName = "TrigThresh" + trigName;

   // get TriggerThreshold value
  map<string,double> valMap = GetVal(detNum,keyName,eventTime);
  return valMap.find(keyName)->second;
 }


// ........................................................



double DmmDataManager::GetPhononOffset(const int& detNum, const string& channel, const double& eventTime) const
 {


  // check that channel only PA,PB,PC,PD
  if (channel.find("PA")==string::npos &&
      channel.find("PB")==string::npos &&
      channel.find("PC")==string::npos &&
      channel.find("PD")==string::npos)
    {
      cerr << "DmmDataManager::GetPhononOffset: ERROR! Channel ID '"<< channel <<"' wrong (needs to be PA,PB,PC,or PD)" << endl;
      exit(1);
    }     
   
  // construct parameter name
  string  keyName = channel+"SqOffset";

  // get value
  map<string,double> valMap = GetVal(detNum,keyName,eventTime);
  return valMap.find(keyName)->second;
}


// ........................................................



double DmmDataManager::GetLastDmmTime(const map<string,double> &zipMap, const double& eventTime) const
{
   // get last Dmm time any parameters
   double nearestTime = 0;  
   
   // loop map of the particular 

   map<string,double>::const_iterator zipMapItr = zipMap.begin();
     
   for( ; zipMapItr != zipMap.end(); zipMapItr++)
    {

     // key name (includes time)
     string key = zipMapItr->first;
    
     // now get the time stamp 
     string::size_type  posTime = key.find("@");
     string timeStampStr = key.substr(posTime+1); 
     double timeStamp =   atof(timeStampStr.c_str());
    

     if (timeStamp<=eventTime && timeStamp>nearestTime)
                       nearestTime = timeStamp;
                
     //  information are stored sequential in time... 
      if (timeStamp>eventTime) break;

    }

 return nearestTime;
}



// ........................................................



map<string,double> DmmDataManager::GetVal(const int& detNum, const string& keyName, const double& eventTime) const
 {

 // -- retrieve the map for detector "detNum" ---
  map< int, map<string,double> >::const_iterator zipMapItr = fZipMapOfMapDouble.find(detNum);



 // check map exist

 if(zipMapItr == fZipMapOfMapDouble.end())
   { 
      cerr <<"DmmDataManager::GetVal("<< keyName <<"): ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
      exit(1);
   } 

  map<string,double> zipMap = zipMapItr->second;
  return GetVal(zipMap,keyName,eventTime);
}




// ........................................................



map<string,double> DmmDataManager::GetVal(const map<string,double> &zipMap, const string& keyName, const double& eventTime) const
{
 
  double value(-999999.);
  double nearestTime = 0;         
  string keyNameTime = keyName + "time";

  map<string,double>::const_iterator zipMapItr = zipMap.begin();
     
  for( ; zipMapItr != zipMap.end(); zipMapItr++)
   {

     string key = zipMapItr->first;
      
     // check if parameter exist
     string::size_type found = key.find(keyName);
     if (found!=string::npos)
       { 
        // now find the time stamp
         string::size_type  posTime = key.find("@");
         string timeStampStr = key.substr(posTime+1);
         double timeStamp =   atof(timeStampStr.c_str());
     
       
         if (timeStamp<=eventTime && timeStamp>nearestTime)
          {
            nearestTime = timeStamp;
            value = zipMapItr->second;
           }
       
         //  information are stored sequential in time... 
         if (timeStamp>eventTime) break;

       }
    }

  if (nearestTime==0)
   {
       cout <<"DmmDataManager::GetVal("<< keyName <<"): ERROR! don't find the nearest DMM time or parameter name '"<< keyName << "' wrong!" << endl;
       exit(1);
   }


 // store both value and time in a map

  map<string,double> parMap;
  parMap.insert(pair<string,double>(keyName,value));
  parMap.insert(pair<string,double>(keyNameTime,nearestTime)); 
    
  return parMap;

}




//  =================  Read DMM file  =================



void  DmmDataManager::ReadFile(const string&  filename)
{

 // -------- check if file exist ----------
 fFileExists = true;

 ifstream file(filename.c_str());
 if (!file) 
  { 
    fFileExists = false;
    cout <<"DmmDataManager::ReadFile:  WARNING!  no DMM file found or file corrupted"<< endl;
    return;
 
  } else {
     cout << "**** Reading DMM file " << filename << endl;
  }
    



  // ------- a few declarations  ------------

  // line informations
  string line;
  int lineNumber = 0;
    
 // --------- read line by line  ------------
 
 while (getline(file,line))
  {
    lineNumber++;
    
    // skip empty line
    if (! line.length())   continue;  // skip empty lines
 
    //  put informations into vector (string)
    vector<string> lineTokens = Tokenize(line);

    //check vector size
    if (lineTokens.size() != 4)
     {
       cerr << "DmmDataManager::ReadFile: ERROR! I don't understand line "<< lineNumber <<"!"<< endl;
       exit(1);
     }


    // Extract informations from lineTokens
   
    // lineTokens[0] = time
    // lineTokens[1] = detector code (401-430)
    // lineTokens[2] = channel and variable code 
    //    2001 - Trigger Threshold - QHi
    //	  2002 - Trigger Threshold - QLo
    //	  2003 - Trigger Threshold - PHi
    //	  2004 - Trigger Threshold - PLo
    //	  2005 - Trigger Threshold - Whisper
    //	  1001 - Phonon Offset A
    //	  1002 - Phonon Offset B
    //    1003 - Phonon Offset C
    //    1004 - Phonon Offset D
    // lineTokens[3] = value



    //  time  stamp
     string timeStampStr =  trim(lineTokens[0]);
     


    // Detector number (last 2 digits of 4xx)
      string detectorNumberStr = trim(lineTokens[1]).substr(1,2);
  
     // convert to integer (takes care to convert "01" to "1")
     int  detectorNumber =   atoi(detectorNumberStr.c_str()); 
  
   

     // channel and variable code
     string varCodeStr = trim(lineTokens[2]);
     int varCode =   atoi(varCodeStr.c_str()); 

     string keyName;

     switch (varCode)
       {
          case 1001: 
              keyName ="PASqOffset";
              break;
          case 1002: 
              keyName ="PBSqOffset";
              break;   
          case 1003: 
              keyName ="PCSqOffset";
              break;
          case 1004: 
              keyName ="PDSqOffset";
              break;
          case 2001: 
              keyName ="TrigThreshQHi";
              break;
          case 2002: 
              keyName ="TrigThreshQLo";
              break;
          case 2003: 
              keyName ="TrigThreshPHi";
              break;
          case 2004: 
              keyName ="TrigThreshPLo";
              break;
          case 2005: 
              keyName ="TrigThreshWisp";
              break;
           default:
               cerr <<"DmmDataManager::ReadFile: ERROR! sorry, I don't recognize line  " << lineNumber << " !" << endl;
               exit(1); 
        }
  


     
    // value
     string valueStr = trim(lineTokens[3]);
     double value = atof(valueStr.c_str());


    // store informations
    string parameterKey = keyName+"@"+timeStampStr;
    SetDoubleParameter(detectorNumber, parameterKey,value,true);
   
   }
}


// ........................................................


vector<string>  DmmDataManager::Tokenize(string aStr)
{

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



// ........................................................



string  DmmDataManager::trim (string str)
{

 const char whitespace[] = " \n\t\v\r\f";
 str.erase(0,str.find_first_not_of(whitespace));
 str.erase(str.find_last_not_of(whitespace) +1U);

 return str;
}


//  =================  Set functions   =================



void DmmDataManager::SetDoubleParameter(int detNum, const string& keyName, double val, bool overwriteFlag)
{

 // -- retrieve the map for detector "detNum" ---

  map< int, map<string,double> >::iterator zipMapItr = fZipMapOfMapDouble.find(detNum);


 // --- case ZIP map doesn't exist yet ---

  if(zipMapItr == fZipMapOfMapDouble.end())
    {  

      map<string, double> newMap;
      newMap.insert(pair<string,double>(keyName,val));
      fZipMapOfMapDouble.insert(pair<int, map<string, double> >(detNum, newMap));
  
      return;     
    } 



 // ---- case ZIP map exist ---

 // check if parameter has been set already
 if((zipMapItr->second).count(keyName) == 1 && overwriteFlag == false)
    {   
      cerr <<"DmmDataManager:SetDoubleParameter: ERROR! parameter " << keyName << " for detector "<< detNum << " already set!"<< endl;
      exit(1);
    }


 // set parameter
  (zipMapItr->second)[keyName] = val;
   

  return;

}

