///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: IsrDataManager
//Authors: B. Serfass
//Description:  This class read the ISR file, store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

// standard library
#include <cmath>
#include <cstdlib>

// cdmsbats library
#include "IsrDataManager.h"
#include "BatRootTypes.h"


////////////////////////////////////////////////////////


      


IsrDataManager::IsrDataManager(const int& maxZIPs) : 
    fStoreRQs(true)
{ 
  //Construct the RQ list
  ConstructEventRQList();
  ConstructZipRQList(maxZIPs);
}


//a default constructor that does nothing
IsrDataManager::IsrDataManager() 
{ 

}
   

IsrDataManager::~IsrDataManager() 
{ 
}


//  =================  RQ list  =================



// Event RQ list

void IsrDataManager::ConstructEventRQList()
{
   double initVal = -999999.;

   fEventRQList.insert(pair<string,double>("LastISRTime", initVal));
   return;
}



//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void IsrDataManager::ConstructZipRQList(const int& maxZIPs)
{
  double initVal = -999999.;

  vector<string> chanQList = GetChanList("chargechan");
  vector<string> chanPList = GetChanList("phononchan");
  
  // loop ZIPs
  for(int zipItr=0; zipItr<maxZIPs; zipItr++)
   {

   // map RQs (same for each detector)
   map<string, double> mapRQ;
      
   // charge quantities
   for (uint listItr=0;listItr<chanQList.size();listItr++)
    {     
      string bias = chanQList[listItr] + "bias";
      string biastime = chanQList[listItr] + "biastime";
      mapRQ.insert(pair<string,double>(bias,initVal));
      mapRQ.insert(pair<string,double>(biastime,initVal));
    }
  
   // phonon quantities
    for (uint listItr=0;listItr<chanPList.size();listItr++)
    {     
      string bias = chanPList[listItr] + "bias";
      mapRQ.insert(pair<string,double>(bias,initVal));
    
    }

   fZipRQList.insert(pair<int, map<string, double> >(zipItr+1, mapRQ));

   }

   return;
}


void IsrDataManager::ResetRQList()
{
  ResetEventRQList();
  ResetZipRQList();
  return;
}


void IsrDataManager::ResetEventRQList()
{
  double initVal = -999999.;

  // Reset Event RQs
 
  map<string,double>::iterator rqListItr = fEventRQList.begin();
  for( ; rqListItr!=fEventRQList.end(); rqListItr++)
    {
      rqListItr->second = initVal;
    }
  
}


void IsrDataManager::ResetZipRQList()
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




//  =================  CALC ISR quantities  =================



void IsrDataManager::DoCalcLastIsrTime(const double& eventTime)
{
  //  Get LastIsrTime
  double lastisrtime = GetLastIsrTime(eventTime);


  if(fStoreRQs)
      fEventRQList["LastISRTime"]  = lastisrtime;

  return;
}



void IsrDataManager::DoCalcBias(const int& detNum, const double& eventTime)
 {
   // Get channel list

   vector<string> chanQList = GetChanList("chargechan");
   vector<string> chanPList = GetChanList("phononchan");


   // Get ZIP map
   map< int, map<string,double> >::const_iterator zipMapItr = fZipMapOfMapDouble.find(detNum);
  
   // check map exist
   if(zipMapItr == fZipMapOfMapDouble.end())
   { 
     cerr <<"IsrDataManager:DoCalcBias: ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 
  
   map<string,double> zipMap =  zipMapItr->second;


   // Get RQ list 
   map<int, map<string,double> >::iterator zipRQList = fZipRQList.find(detNum);

  

   // loop phonon channels and get bias informations
   for (uint listItr=0;listItr<chanPList.size();listItr++)
    {
      string  keyName = chanPList[listItr]+"bias";
      map<string,double> valMap = GetVal(zipMap,keyName,eventTime);
  
      // Store RQs
      if (fStoreRQs)
           (zipRQList->second)[keyName] = valMap.find(keyName)->second;
    }



   // loop charge channels and get bias/biastime
    for (uint listItr=0;listItr<chanQList.size();listItr++)
     {
       string channel =  chanQList[listItr]; 
 
       string  keyName = channel+"bias";
       string  keyNameTime = channel+"biastime";
     
       // get Qenableb informations
     
       string QenabledName = channel + "enabled";
       string QenabledTimeName = QenabledName + "time";
       map<string, double> QenabledMap =  GetQenabled(detNum,channel, eventTime);
       double Qenabled = QenabledMap.find(QenabledName)->second;
       double Qenabledtime = QenabledMap.find(QenabledTimeName)->second;
 
       // get bias informations
       
       map<string,double> valMap = GetVal(zipMap,keyName,eventTime);

       double bias = valMap.find(keyName)->second;  
       double biastime = eventTime - min(valMap.find(keyNameTime)->second, Qenabledtime);

       if (Qenabled==0)
        {
          bias = 0;
          biastime = -biastime;
        }
      
        if (fStoreRQs)
         {
          (zipRQList->second)[keyName] = bias;
          (zipRQList->second)[keyNameTime] = biastime;
         }
       }
 
 return;

}

  


//  =================  Get functions  =================

bool IsrDataManager::IsConfigured(const int& detNum) const
{
  bool configured = false;

 // get ZIP map
 map< int, map<string,double> >::const_iterator zipMapItr = fZipMapOfMapDouble.find(detNum);

 if(zipMapItr != fZipMapOfMapDouble.end())
    configured = true;
 
 return configured;
}
   




double IsrDataManager::GetLastIsrTime(const double& eventTime) const
{

 double nearestIsrTime = 0;         
 
 //loop over ISR time vector
 for(uint isrTimeID = 0; isrTimeID < fISRtime.size(); isrTimeID++)
   {
     double ISRtime = fISRtime[isrTimeID];
     if (ISRtime<=eventTime && ISRtime>nearestIsrTime)
             nearestIsrTime = ISRtime;
   }

 double lastISRtime = eventTime - nearestIsrTime;
 return lastISRtime;
}
   

// ........................................................


double  IsrDataManager::GetBias(const int& detNum,const string& channel, const double& eventTime) const
{

  string  keyName = channel+"bias";
  string  keyNameTime = keyName + "time";

  double bias(-999999.); 

 // check Qenable for charge channel
 if (channel.find("Q")!=string::npos)
  {
    // get Qenableb informations
    string QenabledName = channel + "enabled";
    map<string, double> QenabledMap =  GetQenabled(detNum,channel, eventTime);
    int Qenabled = (int) QenabledMap.find(QenabledName)->second;
    if (Qenabled==0)
                return 0;
  }


  // get bias informations
  map<string,double> valMap = GetVal(detNum,keyName,eventTime);
  bias = valMap.find(keyName)->second;
  return bias;
}


// ........................................................



double  IsrDataManager::GetBiasTime(const int& detNum,const string& channel, const double& eventTime) const
{

 string  keyName = channel+"bias";
 string  keyNameTime = keyName + "time";

 double biastime = -999999.; 

 // charge channel
 if (channel.find("Q")!=string::npos)
  {
    // get Qenableb informations
    string QenabledName = channel + "enabled";
    string QenabledTimeName = QenabledName + "time";
    map<string, double> QenabledMap =  GetQenabled(detNum,channel, eventTime);
    int Qenabled = (int) QenabledMap.find(QenabledName)->second;
    double Qenabledtime = QenabledMap.find(QenabledTimeName)->second;

    // get bias informations
    map<string,double> valMap = GetVal(detNum,keyName,eventTime);
    double biastimeTemp = valMap.find(keyNameTime)->second;
    biastime = min(biastimeTemp, Qenabledtime);
    if (Qenabled==0)
           biastime = -biastime;
   }

  // phonon channel
  if (channel.find("P")!=string::npos)
   {
     // get bias informations
     map<string,double> valMap = GetVal(detNum,keyName,eventTime);
     biastime = valMap.find(keyNameTime)->second;
   }


  return (eventTime - biastime);
}


// ........................................................


map<string,double>  IsrDataManager::GetQenabled(const int& detNum,const string& channel, const double& eventTime) const
{

  if (channel.find("P")!=string::npos)
   {
     cerr<<"IsrDataManager::GetQenabled: ERROR! only for QI or QO!" << endl;
     exit(1);
   }


  string  keyName = channel+"enabled";
  return GetVal(detNum,keyName,eventTime);
 }


// ........................................................



double  IsrDataManager::GetDriverGain(const int& detNum, const string& channel, const double& eventTime) const
{

 // get gain
 string  keyName = channel+"gain";
 map<string,double> valMap = GetVal(detNum,keyName,eventTime);
 double gain = valMap.find(keyName)->second;

 // get bias
 double bias = GetBias(detNum,channel,eventTime);

 if (bias<0)
     gain = -gain;

 return gain;


}


// ........................................................



map<string,double> IsrDataManager::GetVal(const int& detNum, const string& keyName, const double& eventTime) const
{
  
  // get ZIP map
  map< int, map<string,double> >::const_iterator zipMapItr = fZipMapOfMapDouble.find(detNum);

  // check map exist
  if(zipMapItr == fZipMapOfMapDouble.end())
   { 
     cerr <<"IsrDataManager:GetVal("<<keyName<<"): ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   }
 
   map<string,double> zipMap = zipMapItr->second;
   return GetVal(zipMap,keyName,eventTime);

}


// ........................................................



map<string,double> IsrDataManager::GetVal(map<string,double> &zipMap, const string& keyName, const double& eventTime) const
{
 
  double value(-999999.);
  double nearestIsrTime = 0;         
  string keyNameTime = keyName + "time";

 
  // find the parameter with nearest ISR time

  map<string,double>::const_iterator zipMapItr = zipMap.begin();
  for( ; zipMapItr != zipMap.end(); zipMapItr++)

   {
      string key = zipMapItr->first;

     // check if isr info
      string::size_type found = key.find(keyName);
      if (found!=string::npos)
       { 
        // now find the time stamp
         string::size_type  posTime = key.find("@");
         string timeStampStr = key.substr(posTime+1); 
         double timeStamp =   atof(timeStampStr.c_str());
        

         // and store nearest time/value
         if (timeStamp<eventTime && timeStamp>nearestIsrTime)
          {
            nearestIsrTime = timeStamp;
            value = zipMapItr->second;
           }
         

        } 
    }

   if (nearestIsrTime==0)
    {
       cerr <<"IsrDataManager::GetVal: ERROR! don't find the nearest ISR time!" << endl;
       exit(1);
    }

   // store both value and time in a map


  map<string,double> parMap;
  parMap.insert(pair<string,double>(keyName,value));
  parMap.insert(pair<string,double>(keyNameTime,nearestIsrTime)); 
    
  return parMap;

}


// ........................................................



vector<string> IsrDataManager::GetChanList(const string& multID)
{
   vector<string> chanList;
  
   //all charge channels
   if(multID == "chargechan")
   {
      for(int nameItr=0; nameItr < BatRootTypes::kZipFlipNChargeChan; nameItr++)
      {
	 chanList.push_back(BatRootTypes::kZipFlipChargeChan[nameItr]);
      }
   }

   //all phonon channels
   if(multID == "phononchan")
   {
      for(int nameItr=0; nameItr < BatRootTypes::kZipFlipNPhononChan; nameItr++)
      {
	 chanList.push_back(BatRootTypes::kZipFlipPhononChan[nameItr]);
      }
   }

   return chanList;
}



  
//  =================  Read configuration file  =================




void  IsrDataManager::ReadFile(const string& filename)
{

  // ------ open and check if file exist -------
 
  ifstream file(filename.c_str());
  if (!file) 
  
  { 
    cerr << "IsrDataManager::ReadFile: ERROR! File '" << filename <<"' not found..." << endl;
    exit(1); 
  } else {
     cout << "**** Reading ISR file " << filename << endl;
  }
    

 
  // ------- a few declarations  ------------

  // line informations
  string line;
  int lineNumber = 0;
     
   

  // --------- Get ISR file version ------------
    
  double isr_version;


  // read is version (should be the first line)
  getline(file,line); 
  lineNumber++;
  string isr_versionStr = trim(line);

 
  // check if this is a digit
  if (!isdigit(isr_versionStr[0]))
   {
      cerr << "IsrDataManager::ReadFile: ERROR! Don't understand line "<< lineNumber <<"!"<< endl;
      exit(1);
    }

  isr_version = atof(isr_versionStr.c_str());
  

  // ----------  Get detectors slot / board version ------------

  // map of detectors in function of slot/board numbers
  map<string,string>  slotDetectorStrMap;
  map<string,string>  detectorBoardStrMap;
  

  // find configuration line (start with "+")

 while (getline(file,line))
 {

   lineNumber++;
   if (trim(line)[0] == '+') 
    {
      string configline = trim(line);
            
      // find letter 'Z' and first comma
      string::size_type zpos = configline.find_first_of("Z");
      string::size_type firstcomma = configline.find_first_of(",");
      string::size_type secondcomma = configline.find_first_of(",",firstcomma+1);

      if ( !(zpos== string::npos) && !(firstcomma==string::npos)  && !(secondcomma==string::npos))
       {
         // get detector number string
         string detNumberStr = trim(configline.substr(zpos+1,firstcomma-zpos-1));
             
         // get slot number string
         string slotNumberStr = trim(configline.substr(firstcomma+1,secondcomma-firstcomma-1));
         

         // get board number string 
         string boardNumberStr = trim(configline.substr(secondcomma+1));


         // check if first characted is a digit (TEMP... will need more check) 
        
         if ( ! isdigit(detNumberStr[0]) || !isdigit(boardNumberStr[0]) || !isxdigit(slotNumberStr[0]))
            { 
              cerr <<"IsrDataManager::ReadFile: ERROR! Don't recognize line " << lineNumber << " !" << endl;
              exit(1);
            }


          // save information into maps (check if already exist)
          if (!ListManager::HasParameter(slotDetectorStrMap,slotNumberStr) && !ListManager::HasParameter(detectorBoardStrMap,detNumberStr))
           {
           
            ListManager::SetParameter(slotDetectorStrMap,slotNumberStr,detNumberStr);
            ListManager::SetParameter(detectorBoardStrMap,detNumberStr,boardNumberStr);
       
           } else {
         
              cerr <<"IsrDataManager::ReadFile: ERROR! Line " << lineNumber << ": detector configuration already set!" << endl;
              exit(1);
           }

            
         }  else {
          cerr <<"IsrDataManager::ReadFile: ERROR! Don't recognize line " << lineNumber << " !" << endl;
          exit(1);
        }       
    }
 }



 // ---------- Get Configuration values ---------------

 // we have the detector slot/board numbers...
 // go back to first line to read  the actual information

 file.clear();
 file.seekg(0,ios::beg);
 lineNumber= 0;



 // declate time stamp string 
  string timeStampStr;


 while (getline(file,line))       //  read line by line 
 {
   lineNumber++;
  
   // skip comments and empty lines
   if (! line.length())   continue;
   if ( (line[0] == '*') ||  (line[0] == ';'))  continue;


   // ----- Get time stamp ---------


   if ( (line.find('%')!=string::npos) || (line.find('@')!=string::npos))
     { 
   
      vector<string> lineTokens = Tokenize(line);
      timeStampStr =  lineTokens[1];    
   

      // check if time stamp start with a digit
      if (!isdigit(timeStampStr[0]))
       {
        cerr <<"IsrDataManager::ReadFile: ERROR! Don't recognize line " << lineNumber << " !" << endl;
        exit(1);
       }

      // save into vector
      double timeStamp =  atof(timeStampStr.c_str());
      fISRtime.push_back(timeStamp);
       
      continue;
 
     }

 
   // check if we have a time stamp;
  
   if (timeStampStr.empty() || timeStampStr=="0")
         continue;

      

   // ------- get configuration values --------


    if (line.find('!')!=string::npos) {
     
       // split the line into tokens
       vector<string> lineTokens = Tokenize(line);
   
       // get GPIB command 
       string commandStr = lineTokens[1].substr(2,1);  

       // get "channel" string  and convert into decimal 
       string channelStr = lineTokens[1].substr(3,1);
       int channelDec;
       sscanf(channelStr.c_str(),"%x",&channelDec);

       // only interested in the gain/bias informations
       if (!(commandStr == "1" || commandStr == "7" || ((commandStr=="e" || commandStr == "E") && (channelDec==2 || channelDec==4))))
       	continue;


       // get  slot number string
       string slotNumberStr = lineTokens[1].substr(0,2);
       if (slotNumberStr[0]=='0')
                slotNumberStr =   slotNumberStr.substr(1,1);


       // get detector number stored in slotDetectorStrMap using slot number
       string detNumberStr;
       if (slotDetectorStrMap.count(slotNumberStr)>0)
        {
          detNumberStr = ListManager::GetParameter(slotDetectorStrMap,slotNumberStr);
                    
        } else {
            cerr <<"IsrDataManager::ReadFile: ERROR! Don't find the detector number !" << endl;
            exit(1);
       } 
         

       // get value  and convert into decimal  
       string valueStr = lineTokens[2];
       int valueDec;
       sscanf(valueStr.c_str(),"%x",&valueDec);

       // DAC volatage in volts
      
       double voltage;
       voltage = (double) valueDec/4096*10-5;
     
       // declaration of parameters to be stored
       bool doStore;
       double value;
       string key;
       int channel;
       string channelName;
   
           
       if ((commandStr=="1") || (commandStr == "7"))
        {
          // charge/phonon bias  command

          if (channelDec> 9)
           {          
            value = voltage/4990;
            channel = channelDec -7;
            doStore = true;
            
            switch (channel)
             {
               case 3: 
                  channelName ="PA";
                  break;
               case 4: 
                  channelName ="PB";
                  break;
               case 5: 
                  channelName ="PC";
                  break;
               case 6: 
                  channelName ="PD";
                  break;
               default:
                  cerr <<  channel << endl;
                  cerr <<"IsrDataManager::ReadFile: ERROR! Don't recognize channel number " << lineNumber << " !" << endl;
                  exit(1);
              }


          
           } else if (channelDec==0) {
          
             value = voltage*2;
             channel = 2;
             channelName = "QO";
             doStore = true;
          
           } else if (channelDec==1) {
          
             value = voltage*2;
             channel = 1;
             channelName = "QI";
             doStore = true;
        
           } else  {
             
             doStore = false;
    
           }


          if (doStore)
           {
             if (channel>2)
              {

               int detectorNumber = atoi(detNumberStr.c_str());
               string parameterKey = channelName+"bias"+"@"+timeStampStr;
            
               SetDoubleParameter(detectorNumber, parameterKey,value,true);
                         
              } else {
                
                int detectorNumber = atoi(detNumberStr.c_str());
               string parameterKey = channelName+"bias"+"@"+timeStampStr;
               SetDoubleParameter(detectorNumber, parameterKey,value,true);
             
             }
              
           } 

         } else if ((commandStr=="e") || (commandStr == "E")) {
            
           // Phonon gain
           if (channelDec==2) 
            { 
             for (int ichannel =1; ichannel<5; ichannel++)
               {       

                 // channel name

                 switch (ichannel)
                   {
                   case 1: 
                     channelName ="PA";
                     break;
                   case 2: 
                     channelName ="PB";
                     break;
                   case 3: 
                     channelName ="PC";
                     break;
                   case 4: 
                     channelName ="PD";
                     break;
                   }



                 // gain value

                 string valueTempStr;
                 int valueTempDec;
                
                 // polarity
                 valueTempStr  = valueStr.substr(4-ichannel,1);
                 sscanf(valueTempStr.c_str(),"%x",&valueTempDec);
                 double polarity  = (double) (-2)*floor((double)valueTempDec/8.)+1;
         
                 //tens
                 double valueTempDouble  = (double) valueTempDec;
                 valueTempDouble = (double) fmod(valueTempDouble,8.0);
                 int tens =  (int)max(10*floor((double)valueTempDouble/4.),1.0);
               
                 // gains
                 int valueTempInt = (int) fmod(valueTempDouble,4.0);
                 double  gains;


                 switch (valueTempInt)
                   {
                    case 0:
                      gains = 1;
                      break;
                    case 1:
                      gains = 14.3;
                      break;
                    case 2:
                      gains = 2;
                      break;
                    case 3:
                      gains = 5;
                      break;
                    default:
                      cerr <<"IsrDataManager::ReadFile: ERROR! Don't recognize gain  " << lineNumber << " !" << endl;
                      exit(1);
                   }


                  value = polarity*tens*gains;

                  // store gain

                  int detectorNumber = atoi(detNumberStr.c_str());
                  string parameterKey = channelName+"gain"+"@"+timeStampStr;
                  SetDoubleParameter(detectorNumber, parameterKey,value,true);
             
               }

            // charge gain
           } else if (channelDec==4)  {

             for (int ichannel =2; ichannel>0; ichannel--)
               {

                 // channel name 

                 switch (ichannel)
                   {
                   case 1: 
                     channelName ="QO";
                     break;
                   case 2: 
                     channelName ="QI";
                     break;
                   }
  
                // gain value 

                 string valueTempStr;
                 int valueTempDec;
                
                 // polarity
                 valueTempStr  = valueStr.substr(1+ichannel,1);
                 sscanf(valueTempStr.c_str(),"%x",&valueTempDec);
                 double polarity  = (double) (-2)*floor((double)valueTempDec/8.)+1;
         
             
                 //tens
                 double valueTempDouble  = (double) valueTempDec;
                 valueTempDouble = (double) fmod(valueTempDouble,8.0);
                 int tens = (int)max(10*floor((double)valueTempDouble/4.),1.0);
               
                 // gain
                 int valueTempInt = (int) fmod(valueTempDouble,4.0);
                 double  gains;

                 
                 switch (valueTempInt)
                   {
                    case 0:
                      gains = 1;
                      break;
                    case 1:
                      gains = 14.3;
                      break;
                    case 2:
                      gains = 2;
                      break;
                    case 3:
                      gains = 5;
                      break;
                    default:
                      cerr <<"IsrDataManager::ReadFile: ERROR! Don't recognize gain  " << lineNumber << " !" << endl;
                      exit(1);
                    }

                  value = polarity*tens*gains;

                   // store gain
                   int detectorNumber = atoi(detNumberStr.c_str());
                   string parameterKey = channelName+"gain"+"@"+timeStampStr;
                   SetDoubleParameter(detectorNumber, parameterKey,value,true);
                  

                   // find Q enableb
                    
                   int enabled = -999;

                   valueTempStr  = valueStr.substr(1,1);
                   sscanf(valueTempStr.c_str(),"%x",&valueTempDec);
                                               
                   if (ichannel==1)
                    { 
                       double valueTempDouble  = (double) valueTempDec;
                       int valueTempInt = (int) fmod(valueTempDouble,2.0);

                       if (valueTempInt==1) {
                            enabled= 0;
                        } else {
                            enabled= 1; 
                        }

                     } else if (ichannel==2) {
                         
                        double valueTempDouble  = (double) floor((double)valueTempDec/2.);
                        int valueTempInt = (int) fmod(valueTempDouble,2.0); 

                         if (valueTempInt==1) {
                            enabled= 0;
                          } else {
                            enabled= 1; 
                          }
                     }


                    // store qenabled
                    parameterKey = channelName+"enabled"+"@"+timeStampStr;
                    SetDoubleParameter(detectorNumber, parameterKey,(double) enabled,true);
                  
               }
           }
       }
  
    } // end command line 

  } // end loop line
}


// ........................................................



vector<string>  IsrDataManager::Tokenize(string aStr)
{

  vector<string> tokens;
  string token;

  const char delimiter[] = " ,\n\t\v\r\f";

  string::size_type lastPos = aStr.find_first_not_of(delimiter,0);
  string::size_type pos = aStr.find_first_of(delimiter,lastPos);
 
  while (string::npos !=pos || string::npos != lastPos)
   {
     // find a token, add it to vector (in lower case) 
     token = trim(aStr.substr(lastPos, pos - lastPos));
   //  transform(token.begin(),token.end(),token.begin(),(int(*)(int)) tolower);
     tokens.push_back(token);
    
     // skip delimiter
     lastPos = aStr.find_first_not_of(delimiter,pos);
     
     // find next non-delimiter
     pos = aStr.find_first_of(delimiter,lastPos);
   }
 
  return tokens;
}





// ........................................................


string  IsrDataManager::trim (string str)
{

 const char whitespace[] = " \n\t\v\r\f";
 str.erase(0,str.find_first_not_of(whitespace));
 str.erase(str.find_last_not_of(whitespace) +1U);

 return str;
}



  
//  =================  Set functions   =================


void IsrDataManager::SetDoubleParameter(const int& detNum, const string& varName, double val, bool overwriteFlag)
{

 // -- retrieve the map for detector "detNum" ---

  map< int, map<string,double> >::iterator zipMap = fZipMapOfMapDouble.find(detNum);


 // --- case ZIP map doesn't exist yet ---

  if(zipMap == fZipMapOfMapDouble.end())
    {  

      map<string, double> newMap;
      newMap.insert(pair<string,double>(varName,val));
      fZipMapOfMapDouble.insert(pair<int, map<string, double> >(detNum, newMap));
  
      return;     
    } 



 // ---- case ZIP map exist ---

 // check if parameter has been set already
 if((zipMap->second).count(varName) == 1 && overwriteFlag == false)
    {   
      cerr <<"IsrDataManager:SetDoubleParameter: ERROR! parameter " << varName << " for detector "<< detNum << " already set!"<< endl;
      exit(1);
    }


 // set parameter
  (zipMap->second)[varName] = val;
   

  return;

}


     
     

