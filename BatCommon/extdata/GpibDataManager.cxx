///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: GpibDataManager
//Authors: B. Serfass
//Description:  This class read the GPIB file, store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////


#include "GpibDataManager.h"

using namespace std;

////////////////////////////////////////////////////////


     
GpibDataManager::GpibDataManager() :
   fTimeAfterFlash(-999999),
   fTimeBiasOnAfterFlash(-999999),
   fTimeLastFlash(-999999),
   fTimeLastStart(-999999),
   fStoreRQs(true)
 {   
   
  //Construct the RQ list
   ConstructRQList();

 }


 
GpibDataManager::~GpibDataManager() 
{ 

}
    


//  =================  RQ list  =================



//This method constructs the RQ list that is handed off to BatOutputManager
//It also sets the default value for the RQ to initVal.
void GpibDataManager::ConstructRQList()
{
   double initVal = -999999.;

   fRQList.insert(pair<string,double>("FlashTime", initVal));
   fRQList.insert(pair<string,double>("BiasFlashTime", initVal));  
   fRQList.insert(pair<string,double>("TimeSincePause", initVal));  
   return;
}



void GpibDataManager::ResetRQList()
{
  
  double initVal = -999999.;
  
  map<string,double>::iterator rqListItr = fRQList.begin();
  for( ; rqListItr!=fRQList.end(); rqListItr++)
    {
      rqListItr->second = initVal;
    }
  

   return;
}





//  =================  Calc timing from GPIB   =================

void GpibDataManager::DoCalc(const double& eventTime)
{

 
 // --------------------- 
 // find last flash time 
 // and last start or resume
 // ---------------------
 double timeStamp = 0;
 
 map<double,string>::iterator mapIt = fMapString.begin();
 
 for(; mapIt!=fMapString.end(); mapIt++)
  {
    timeStamp =  mapIt->first;
  
    if(timeStamp>eventTime) break;
 
    if (timeStamp>fTimeLastFlash &&  mapIt->second=="0100")
      fTimeLastFlash = timeStamp;


    if (timeStamp>fTimeLastStart &&  (mapIt->second=="1031" || mapIt->second=="1011"))
      fTimeLastStart = timeStamp;    
  } 
 
 if (fTimeLastStart > 0)  
   fTimeLastStart = eventTime - fTimeLastStart;
 
 // --------------------- 
 // Find Time Bias After 
 // Flash
 // ---------------------
 if (fTimeLastFlash > 0)  
  {

   // time after flash
   fTimeAfterFlash = eventTime - fTimeLastFlash;
     
      
   timeStamp =0; // reset variable 
   
   double timeBiasOn = 0;
   bool isBiasOn= false; // Assume bias is off after falshing....
     
   fTimeBiasOnAfterFlash = 0;

   map<double,string>::const_iterator mapFlashIt = fMapString.find(fTimeLastFlash);
   
   for(; mapFlashIt!=fMapString.end(); mapFlashIt++)
     { 
       timeStamp = mapFlashIt->first;
       
       if(timeStamp>eventTime) break;

       // bias turned on when "config" or "resume"
       if (mapFlashIt->second=="1001" || mapFlashIt->second=="1031") {
	 if (!isBiasOn) {
	   timeBiasOn = timeStamp; // save timeStamp
	   isBiasOn = true;
	 }
       }

       // bias  turned off when "stop" or "pause"
       if (mapFlashIt->second=="1000" || mapFlashIt->second== "1021") {
	 if (isBiasOn) {
	   isBiasOn = false;
	   fTimeBiasOnAfterFlash = fTimeBiasOnAfterFlash + timeStamp - timeBiasOn;
	 }
       }
     }
   
   // check status of bias after last change
   if (isBiasOn) 
     fTimeBiasOnAfterFlash = fTimeBiasOnAfterFlash + eventTime-timeBiasOn;
   
  }


 // fill RQs
  
  if(fStoreRQs) {
      fRQList["FlashTime"]  = fTimeAfterFlash;
      fRQList["BiasFlashTime"]  = fTimeBiasOnAfterFlash;
      fRQList["TimeSincePause"] = fTimeLastStart;
    }
 
 return;
}


  

//  =================  Read configuration file  =================




void  GpibDataManager::ReadFile(string filename)
{

 // -------- check if file exist ----------


 ifstream file(filename.c_str());
 if (!file) 
  
 { 
    cerr << "GpibDataManager::ReadFile ERROR! '" << filename <<"' not found!" << endl;
    exit(1);
 
 } else {
     cout << "\n**** Reading GPIB file: " << filename << endl;
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
    if (lineTokens.size() != 2)
     {
       cerr << "GpibDataManager::ReadFile:  ERROR! I don't understand line "<< lineNumber <<"!"<< endl;
       exit(1);
     }


    // Extract informations from lineTokens
   
    // lineTokens[0] = time
    // lineTokens[1] = codes
    //    1001 - config
    // 	  1011 - run (start)
    //    1021 - pause
    //    1031 - resume
    //    1000 - stop
    //    1099 - abort
    //    0100 - start bake
    //    0111 - end bake
  



    //  time  stamp
     string timeStr =  trim(lineTokens[0]);
     double time = atof(timeStr.c_str()); 
    
     // code 
     string tagStr = trim(lineTokens[1]);
   
    // store informations
     SetParameter(time,tagStr,  true); 

   }
}



// ........................................................



vector<string>  GpibDataManager::Tokenize(string aStr)
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


string  GpibDataManager::trim (string str)
{

 const char whitespace[] = " \n\t\v\r\f";
 str.erase(0,str.find_first_not_of(whitespace));
 str.erase(str.find_last_not_of(whitespace) +1U);

 return str;
}





//  =================  Set Parameter  =================



void GpibDataManager::SetParameter(double time, string tag, bool overwriteFlag)
{
  // Set parameters in map according 
   
  map<double,string>::iterator itMap = fMapString.find(time);

  // if parameter exist and overwrite false -> exit

  if (!(itMap == fMapString.end()) && overwriteFlag==false)
   {
     cerr << "GpibDataManager::SetParameter: ERROR! Time '"<< time << "'  already tagged, please check your code!" << endl;
     exit(1);  
   } 


 // add parameter
  if (itMap == fMapString.end()) { 
    fMapString.insert(pair<double,string>(time,tag));
   } else {
    itMap->second = tag;
   }

} 


