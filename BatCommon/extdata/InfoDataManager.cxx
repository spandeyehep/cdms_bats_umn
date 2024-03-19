///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: InfoDataManager
//Authors: B. Serfass
//Description:  This class read the INFO file, store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////



#include "InfoDataManager.h"



////////////////////////////////////////////////////////


      


InfoDataManager::InfoDataManager()  
 {   

 }


 
InfoDataManager::~InfoDataManager() 
{ 

}
    


// ____________________ Get functions __________________________




double InfoDataManager::GetSampleRate(const int& detNum, const string& sensorType) const
{

  
  string sensorBase;
  if (sensorType=="phonon")
     sensorBase="P";
  else if (sensorType=="charge")
     sensorBase="Q";
  else {
     cerr << "InfoDataManager::GetSampleRate: ERROR!  don't recognize sensor type '" << sensorType << endl;
     exit(1);
    }


  string keyBase = "_SAMPLERATE";
  string keyName = sensorBase + keyBase;
 


   
// get ZIP map

  map< int, map<string,double> >::const_iterator zipMap = fZipMapOfMapDouble.find(detNum);
 
 // check map exist
  if(zipMap == fZipMapOfMapDouble.end())
   { 
     cerr <<"InfoDataManager::GetSampleRate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 


  
 // loop through the ZIP map
 map<string,double>::const_iterator zipMapItr = (zipMap->second).begin();
     
 for( ; zipMapItr != (zipMap->second).end(); zipMapItr++)
   {

      string key = zipMapItr->first;
      
     // check if parameter found
      string::size_type found = key.find(keyName);
      if (found!=string::npos)
       { 
         double value = zipMapItr->second;
         return value;
       }
         
    }

  cerr <<"InfoDataManager::GetSampleRate:  ERROR! Didn't find Parameter "<<  keyName << " for detector "<< detNum << endl;
  exit(1);

}



int InfoDataManager::GetPreTrigger(const int& detNum, const string& sensorType) const
{

  
  string sensorBase;
  if (sensorType=="phonon")
     sensorBase="P";
  else if (sensorType=="charge")
     sensorBase="Q";
  else {
     cerr << "InfoDataManager:GetPreTrigger: ERROR!  don't recognize sensor type '" << sensorType << endl;
     exit(1);
    }


  string keyBase = "_PRETRIGGER";
  string keyName = sensorBase + keyBase;
 


   
// get ZIP map

  map< int, map<string,double> >::const_iterator zipMap = fZipMapOfMapDouble.find(detNum);
 
 // check map exist
  if(zipMap == fZipMapOfMapDouble.end())
   { 
     cerr <<"InfoDataManager::GetPreTrigger:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 


  
 // loop through the ZIP map and find the the ISR time 
 map<string,double>::const_iterator zipMapItr = (zipMap->second).begin();
     
 for( ; zipMapItr != (zipMap->second).end(); zipMapItr++)
   {

      string key = zipMapItr->first;
      
     // check if parameter found
      string::size_type found = key.find(keyName);
      if (found!=string::npos)
       { 
         int value = (int) zipMapItr->second;
         return value;
       }
         
    }
 cerr <<"InfoDataManager::GetPreTrigger:  ERROR! Didn't find Parameter "<<  keyName << " for detector "<< detNum << endl;
  exit(1);


}





int InfoDataManager::GetPostTrigger(const int& detNum, const string& sensorType) const
{

  
  string sensorBase;
  if (sensorType=="phonon")
     sensorBase="P";
  else if (sensorType=="charge")
     sensorBase="Q";
  else {
     cerr << "InfoDataManager:GetPostTrigger: ERROR!  don't recognize sensor type '" << sensorType << endl;
     exit(1);
    }


  string keyBase = "_POSTTRIGGER";
  string keyName = sensorBase + keyBase;
 


   
// get ZIP map

  map< int, map<string,double> >::const_iterator zipMap = fZipMapOfMapDouble.find(detNum);
 
 // check map exist
  if(zipMap == fZipMapOfMapDouble.end())
   { 
     cerr <<"InfoDataManager::GetPostTrigger:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 


  
 // loop through the ZIP map and find the the ISR time 
 map<string,double>::const_iterator zipMapItr = (zipMap->second).begin();
     
 for( ; zipMapItr != (zipMap->second).end(); zipMapItr++)
   {

      string key = zipMapItr->first;
      
     // check if parameter found
      string::size_type found = key.find(keyName);
      if (found!=string::npos)
       { 
         int value = (int) zipMapItr->second;
         return value;
       }
         
    }
 

  cerr <<"InfoDataManager::GetPostTrigger:  ERROR! Didn't find Parameter "<<  keyName << " for detector "<< detNum << endl;
  exit(1);


}





  

//  ___________________ read configuration file ______________________




void  InfoDataManager::ReadFile(string filename)
{

 // check if file exist


 ifstream file(filename.c_str());
 if (!file) 
  
 { 

    cerr << "InfoDataManager::ReadFile: ERROR! File '" << filename <<"' not found!" << endl;
    exit(1);
 
 } else {
    cout << "**** Reading INFO file " << filename << endl;
 }

 //  a few declarations 

 string line;
 
 vector<string> digitizerNameVect;
 vector<string> digitizerValVect;

 string type; // run type
 string key ;         // parameter name 
 string detector ;    // detector(s) name
 string value ;       // value of the parameter

 const char whitespace[] = " \n\t\v\r\f";

 

 int lineNumber = 0;


 // read line by line 

 while (getline(file,line))
 {

   lineNumber++;

    
   // -------- Get "Type" --------

   if (!(line.find("Type of data",0) == string::npos))
    { 
      // find "Type" line , now get the value
      type = line.substr(line.find_first_of(":")+1);
      type = trim(type);
   
      continue;
    }

     

   // -------  Digitizer section ----------

  
   if (!(line.find("Digitizers",0) == string::npos)) 
    { 
     // found the digitizer section
    

      // now read line by line the digitizer section to find the name of the parameters
 
      while (getline(file,line))
       {
         lineNumber++;

       
         // find line with "Pretrigger"  (appears only once in the digitizer section) and 
       
         if ( line.find("Pretrigger",0) !=  string::npos)
          { 
            // fill vector with names 
            digitizerNameVect = Tokenize(trim(line));
            break; // ok, we have our parameter names vector
      
          } 
       } // end while loop 
      
               
      // make sure we have a vector of trigger information names (filled above) before going further
      if (digitizerNameVect.empty()) continue;
         

      // now go to next line and loop through all the remaining digitizer lines 
      getline(file,line);
      lineNumber++;

      bool terminate = false;
      while (!terminate && file)
       {
               
        //  fill a vector with "words" of the current line (should be digitizer values)
        vector<string>  digitizerValueVect = Tokenize(trim(line));
         
     
        // find if the first character of the firs word is a digit
        string firstWord =  digitizerValueVect[0];

        // check that the first "word" is a number
        if (isdigit(firstWord[0])) 
          {
             // this line has the digitizer values corresponding to names in digitizerNameVect 
             
             // go to next line, which has to be a line with "Channel"...
             getline(file,line);
             lineNumber++;
             
             // ... well, let's make sure it is the right line
             if (line.find("Channel")==string::npos)
                 { 
                 cerr << "InfoDataManager::ReadFile: ERROR!  I don't understand line "<< lineNumber <<"!"<< endl;
                 exit(1);
               }

             // fill a vector with "words" of the current line
             vector<string> channelNameVect = Tokenize(trim(line));
             
         

             // for later use, to record only phonon and charge values once
             bool recordedP = false;
             bool recordedQ = false;

             // loop through all the lines starting with "Channel"
             while ( (channelNameVect[0]=="Channel") && file)
              {  
              
                // third word should give detector/channel number
                string detectorStr = channelNameVect[2];
           
             
                if (detectorStr.find("Z",0)!=string::npos)
                  { 
                    // ZIP digitizer
                
                    // get detector number
                    string  number ="0123456789";
                    string::size_type pos1 = detectorStr.find_first_of(number,0);
                    string::size_type pos2 = detectorStr.find_first_not_of(number,pos1)-1;
                    string detectorNumberStr = detectorStr.substr(pos1,1+pos2-pos1);
                    
                    // convert to integer (this also takes care of converting "01" to "1")
                    int  detectorNumber =   atoi(detectorNumberStr.c_str());
                       
                                  

                    if (detectorStr.find_first_of("pabcdABCD",0)!=string::npos  && !recordedP)
                       { 
                        recordedP = true;
                        
                       
                        for (uint i=0; i<digitizerNameVect.size();i++)
                          { 
                            string key =  digitizerNameVect[i];
                            string val =  digitizerValueVect[i];
                           
                          
                           if (key=="Pretrigger") 
                            {                      
                             double valDouble = atof(val.c_str());
                             SetDoubleParameter(detectorNumber, "P_PRETRIGGER",valDouble,true);
                            }

                           if (key=="Posttrigger") 
                            {
                             double valDouble = atof(val.c_str());
                             SetDoubleParameter(detectorNumber, "P_POSTTRIGGER",valDouble,true);
                            }

                           if (key=="SampleRate") 
                            {
                             double valDouble = atof(val.c_str());
                             SetDoubleParameter(detectorNumber, "P_SAMPLERATE",valDouble,true);
                            }

                          }
                        }

                      if (detectorStr.find_first_of("qQ",0)!=string::npos && !recordedQ)
                       { 
                        recordedQ = true;
                        
                        for (uint i=0; i<digitizerNameVect.size();i++)
                          { 
                            string key =  digitizerNameVect[i];
                            string val =  digitizerValueVect[i];
                        
                            
                           if (key=="Pretrigger") 
                            {
                             double valDouble = atof(val.c_str());
                             SetDoubleParameter(detectorNumber, "Q_PRETRIGGER",valDouble,true);
                            }

                           if (key=="Posttrigger") 
                            {
                             double valDouble = atof(val.c_str());
                             SetDoubleParameter(detectorNumber, "Q_POSTTRIGGER",valDouble,true);
                            }

                           if (key=="SampleRate") 
                            {
                             double valDouble = atof(val.c_str());
                             SetDoubleParameter(detectorNumber, "Q_SAMPLERATE",valDouble,true);
                            }


                           }
                        }   


                   } else if (detectorStr.find("V",0)!=string::npos) {
                     // Veto digitizers
                    // TEMP: do nothing 

                   } else {
                     
                      cerr << "InfoDataManager::ReadFile: ERROR!  I don't understand '" << detectorStr <<"', line "<< lineNumber <<"!"<< endl;
                      exit(1);
                   }
 
                  // new line 
                 getline(file,line);
                 lineNumber++;   

                 // fill again vector with words in "Channel" line and continue the loop until no more "channel" line...       
                 channelNameVect = Tokenize(trim(line));
         
             }  // loop "Channel"
           

            } else { 
                
               // not a "digit", so terminate loop
               terminate = true;
            }
                 
      
           if ((! line.length()) | (line.find_first_not_of(whitespace) == string::npos)) 
                 terminate = true;
        

         } // loop digitizer lines
     
      } // end "Digitizer" section
   

    }// end loop throught info file
  
}




// ........................................................

vector<string>  InfoDataManager::Tokenize(string aStr)
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


string  InfoDataManager::trim (string str)
{

 const char whitespace[] = " \n\t\v\r\f";
 str.erase(0,str.find_first_not_of(whitespace));
 str.erase(str.find_last_not_of(whitespace) +1U);

 return str;
}








// ..........................................................................................

void InfoDataManager::SetDoubleParameter(const int& detNum, const string& varName, double val, bool overwriteFlag)
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
      cerr <<"InfoDataManager:SetDoubleParameter: ERROR! parameter " << varName << " for detector "<< detNum << " already set!"<< endl;
      exit(1);
    }


 // set parameter
  (zipMap->second)[varName] = val;
   

  return;

}


     
