///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: TemplateDataManager
//Authors: B. Serfass
//Description:  This class read the charge template file, construct the phonon template,  store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Dec. 5, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////


#include "TemplateDataManager.h"

// Standard library
#include <math.h>


// library
#include "PulseTools.h"


////////////////////////////////////////////////////////


      


TemplateDataManager::TemplateDataManager()  
 {   

 }


 
TemplateDataManager::~TemplateDataManager() 
{ 

}
      

// __________________ Get template functions ______________________


vector<double> TemplateDataManager::GetTemplate(int detNum,const string& channel) const
{
  
  // check whether the map has been filled
  if(fZipMapOfMapVectDouble.size() == 0)
  {
     cerr <<"TemplateDataManager::GetTemplate:  ERROR! No Templates found for detector " << detNum << " channel " << channel << endl;
     exit(1);
  }

  // get ZIP map
  map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
 
 // check whether matching map exists
  if(zipMap == fZipMapOfMapVectDouble.end())
   { 
     cerr <<"TemplateDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 

 return ListManager::GetParameter(zipMap->second,channel);

}




vector<double> TemplateDataManager::GetDoubleExpForm(double riseTime, double fallTime, int nBins, int preTrigger, double sampleRate) const
 {
   // construct a double  expential pulse shape
   vector<double> pulse;
   for (int ibin=0; ibin<preTrigger-1; ibin++)
    {
      double fval = 0.0;
      pulse.push_back(fval);
    }

   for (int ibin=0; ibin<nBins-preTrigger+1; ibin++)
    {
       double x = ibin/sampleRate;
       double fval = exp(-x/fallTime) - exp(-x/riseTime);
       pulse.push_back(fval);
    }

   return pulse;
 }


vector<double> TemplateDataManager::GetZerosForm(int nBins) const
 {
   // construct a double  expential pulse shape
   vector<double> pulse;
   for (int ibin=0; ibin<nBins; ibin++)
    {
      double fval = 0.0;
      pulse.push_back(fval);
    }

   return pulse;
 }


vector<double> TemplateDataManager::GetDoubleExpForm(double riseTime, double fallTime, int nBins, double sampleRate) const
 {
   // construct a double  expential pulse shape
   vector<double> pulse;
   for (int ibin=0; ibin<nBins; ibin++)
    {
      double x = ibin/sampleRate;
      double fval = exp(-x/fallTime) - exp(-x/riseTime);
      pulse.push_back(fval);
    }

   return pulse;
 }



string TemplateDataManager::GetTemplateTag(int detNum) const
{
   // check whether the map has been filled
  if(fZipMapOfMapVectDouble.size() == 0)
  {
     string notUsedTag = "TemplatesNotUsed";
     return notUsedTag;

  }
  
  // get ZIP map
  map< int, map<string,string> >::const_iterator zipMap = fZipMapOfMapString.find(detNum);
 
  // check that matching map exists
  if(zipMap ==  fZipMapOfMapString.end())
  { 
    string notUsedTag = "TemplatesNotUsed";
    return notUsedTag;
  } 


  return ListManager::GetParameter(zipMap->second,"template_tag");

 }





TGraphErrors* TemplateDataManager::GetCovarianceMatrix(int detNum,const string& channel) const
{
  
  TGraphErrors *graph = NULL;
  string matrixName = channel + "cov_pd"; 


  // check whether the map has been filled
  if(fZipMapOfMapGraphErrors.size() == 0) {
    //cout <<"WARNING in TemplateDataManager::GetCovarianceMatrix: No Covariance Matrix found! "
    // <<"Returning NULL pointer" << endl;
    return graph;
  }
  
  // get ZIP map
  map< int, map<string,TGraphErrors* > >::const_iterator zipMap = fZipMapOfMapGraphErrors.find(detNum);
 
 // check whether matching map exists
  if(zipMap != fZipMapOfMapGraphErrors.end())
   { 
     if (ListManager::HasParameter(zipMap->second,matrixName))
        graph = ListManager::GetParameter(zipMap->second,matrixName);
   }

 
 return graph;

}

 

//  ___________________ read configuration file ______________________



string  TemplateDataManager::GetFileName(int detNum,  const string& templateDir, const string& inputSeries)
{

  string filename;

 // convert intput Serie into double 
 
 double seriesNumber;
 
 string::size_type posUnderscore = inputSeries.find("_",0);
 if (!(posUnderscore == string::npos))
  {
    string seriesNumberStr = inputSeries.substr(0,posUnderscore) + inputSeries.substr(posUnderscore+1);
    seriesNumber = atof(seriesNumberStr.c_str());

   } else {
     cerr << "TemplateDataManager::GetFileName: ERROR! Don't understand input serie '" << inputSeries <<"'!" << endl;
     exit(1);
  }  
 

 // name of the template.list file
 string listFileName = Form("Z%d_Templates.list",detNum);
 string listFullFileName = templateDir + "/lists/" + listFileName;

 ifstream file(listFullFileName.c_str());
  
 if (!file) 
  { 
    cerr << "TemplateDataManager::GetFileName: ERROR! file '" << listFullFileName <<"' for detector "<< detNum << " not found..." << endl;
    exit(1);
  }  

 
 // Read line by line 

 int lineNumber = 0;
 string line;
 
 while (getline(file,line))
  {

   lineNumber++;

   if (! line.length())   continue;  // skip empty lines
   if (trim(line)[0] == '%')    continue; // skip comments 
   
   // remove comments from the line 
   string::size_type posComment = line.find("%",0);
    if (!(posComment == string::npos))
         line = line.substr(0,posComment);
  
   // store information into tokens
   vector<string> lineTokens = Tokenize(line);
   
   // check if input series is within the two series in the file
   double serie1 = atof(lineTokens[0].c_str());
   double serie2 = atof(lineTokens[1].c_str());
 
   if (seriesNumber>=serie1 && seriesNumber<=serie2)
     {
       filename = lineTokens[2];
       break;
     }
   }   

 file.close();

 if (filename.empty())
   { 
     cerr << "TemplateDataManager::GetFileName: ERROR! No template file name found in detector " << detNum << " list for dataset " << inputSeries <<" !"<< endl;
     exit(1);
   }

 return filename;

}



void  TemplateDataManager::ReadAllFiles(const string& templateDir, const string& inputSeries, 
				        const map<int, int>& detectorTypeMap, 
					const map<int, string>& detectorPolarityMap, 
                                        const UserDataManager& userData)
{


  // loop all detectors, read/store templates
  map<int, string>::const_iterator it  = detectorPolarityMap.begin(); 

  for( ;it!=detectorPolarityMap.end(); it++)
  {
    // Detector number
    int detNum = it->first;

    // polarity   
    string polarity = it->second; 

    // type: string that describe template location
    // default: empty string
    // cdmsliteI: "cdmsliteI";
    // broken phonon: "brokenP"; 
    //     if cdmslite + brokenP => "cdmliteI_brokenP"

   


    int detType = detectorTypeMap.find(detNum)->second;
  
    string dirType;
    if (detType==21) dirType = "cdmsliteI";


    string statusStr; //FIXME: for now suppose only one such temple 
    map<string, double> detStatusRQList;
    if (userData.DoRead("DET_STATUS_FILE"))
          detStatusRQList = userData.GetZipRQList(detNum);  
    map<string, double>::iterator mapIter;
    for(mapIter = detStatusRQList.begin(); mapIter != detStatusRQList.end(); ++mapIter) {
        string chanName = mapIter->first;
        
        if (chanName[0] == 'P'  && mapIter->second >1) 
                          statusStr = "brokenP"; 
    }
    

    if (dirType.empty()) dirType = statusStr;
    else if (!dirType.empty() && !statusStr.empty()) 
             dirType = dirType + "_" + statusStr; 
   
    
    //FIXME eventually separate this for charge and phonon templates
    if(!userData.CalcPhononTemplate(detNum) || !userData.CalcChargeTemplate(detNum))
    {
      ReadFile(detNum,dirType, polarity, templateDir, inputSeries);
    }
  }

  return;
}




void  TemplateDataManager::ReadFile(int detNum, const string& dirType, const string& polarity, const string& templateDir,const string& inputSeries)
{

 // read template, then store into a map for further use
 
 string filename = templateDir + "/files/" + GetFileName(detNum,templateDir,inputSeries) + ".root";
     
 TFile file(filename.c_str(),"READ");

 if (!file.IsOpen()) 
   { 
     cerr << "TemplateDataManager::ReadFile: ERROR! file '" << filename <<"' not found..." << endl;
     exit(1);
 
   } else {
     cout << "**** Reading Template file " << filename << endl;
   }
    

 // ROOT file  iterator 
 TKey *aKey;
 TIter fileKeysItr(file.GetListOfKeys());

 // check if directory is available
 bool isDirAll = false;
 bool isDirPolarity = false;

 string zipName = Form("zip%d",detNum);

 string dirNameTemp = zipName;
 if (!dirType.empty()) dirNameTemp = dirNameTemp + "_" + dirType;
 
 string dirNameTempPolarity = dirNameTemp + "_" + polarity;


 // loop to check directory names
 while ((aKey = (TKey *) fileKeysItr()))
   { 
      // Get object
      TObject* obj = aKey->ReadObj();
      if (!strcmp(obj->ClassName(),"TDirectoryFile")) {
          TDirectory *dir = (TDirectory* ) obj;       
          string dirName = dir->GetName();
        
          if (dirName.compare(dirNameTempPolarity)==0) isDirPolarity = true;
          if (dirName.compare(dirNameTemp)==0) isDirAll = true;
       }
    }
  

 if (!isDirAll && !isDirPolarity) {
   cerr << "TemplateDataManager::ReadFile: ERROR! No directory '"<< dirNameTemp
        << "' or '" << dirNameTempPolarity 
        << "' available for detector " << detNum <<" !"<< endl;
   exit(1);
 }


 if (isDirAll && isDirPolarity) {
   cerr << "TemplateDataManager::ReadFile: ERROR! Two directories '"<< dirNameTemp
        << "' and '" << dirNameTempPolarity << "' available for detector " << detNum <<" !"<< endl;
   cerr << "Not sure which one to use!  Please, fix template file before running BatNoise!" << endl;
   exit(1);
 }


 // rename directory
 if (isDirPolarity)
    dirNameTemp = dirNameTempPolarity;


 // loop over list of objetcs in the file and get templates
 fileKeysItr.Reset(); 
 while ((aKey = (TKey *) fileKeysItr()))
 {  
  // Get object
  TObject* obj = aKey->ReadObj();

  // check if object is a directory
  if (!strcmp(obj->ClassName(),"TDirectoryFile"))
   { 
     TDirectory *dir = (TDirectory* ) obj;       
     string dirName = dir->GetName();

     int compareDir = dirName.compare(dirNameTemp);
     if (compareDir==0) {
      
  
      // loop objects in info directory
      TKey* aKey;;
      TIter keysItr(dir->GetListOfKeys()); 
      while ((aKey = (TKey *) keysItr()))
       { 
        // get object  
        TObject* obj = aKey->ReadObj();
         
        // check if object class name
        if(!strcmp(obj->ClassName(),"TH1D")) { // found histogram   

           TH1D *hist = (TH1D*) obj;
           string histname = hist->GetName();
            
           // transform into vector
           vector<double> vect = PulseTools::TH1D2Vector(*hist);
     
           // store into zip map
           SetVectorDoubleParameter(detNum,histname,vect,true);
          
         }  else if(!strcmp(obj->ClassName(),"TTree")) { // found info tree
             
           TTree* tree = (TTree*) obj;
           char tag[50]; 
  
           // FIXME - more info available in template file
           tree->SetBranchAddress("tag",tag);
           tree->GetEntry(0);

           string template_tag = tag;
           SetStringParameter(detNum,"template_tag",template_tag, true);

         }  else if (!strcmp(obj->ClassName(),"TGraphErrors")) { // found Graph
                     
            TGraphErrors *graph = (TGraphErrors*) obj;
            string graphname = graph->GetName();
           
            // store into zip map
            SetTGraphErrorsParameter(detNum,graphname,graph,true);
          

         }
        }
      }
    } 
  }

 file.Close();

 return;

}




string  TemplateDataManager::trim (string str)
{

 const char whitespace[] = " \n\t\v\r\f";
 str.erase(0,str.find_first_not_of(whitespace));
 str.erase(str.find_last_not_of(whitespace) +1U);

 return str;
}




vector<string>  TemplateDataManager::Tokenize(string aStr)
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






// ...................................................................................


template <class Type>
void TemplateDataManager::SetTypeParameter(map<int,map<string,Type> >& aMapOfMapT, int detNum, const string& varName, Type val, bool overwriteFlag)
{

 // -- retrieve the map for detector "detNum" ---

   typename map<int,map<string,Type> >::iterator zipMap = aMapOfMapT.find(detNum);


 // --- case ZIP map doesn't exist yet ---

  if(zipMap == aMapOfMapT.end())
    {  

      map<string, Type> newMap;
      newMap.insert(pair<string,Type>(varName,val));
      aMapOfMapT.insert(pair<int, map<string, Type> >(detNum, newMap));
  
      return;     
    } 



 // ---- case ZIP map exist ---

 // check if parameter has been set already
 if((zipMap->second).count(varName) == 1 && overwriteFlag == false)
    {   
      cerr <<"TemplateDataManager:SetTypeParameter: ERROR! parameter " << varName << " for detector "<< detNum << " already set!"<< endl;
      exit(1);
    }


 // set parameter
  (zipMap->second)[varName] = val;
   

  return;

}



//  trick to be able to have the template function inside the source instead of the header...
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

template void TemplateDataManager::SetTypeParameter<int>(map<int, map<string,int> >& aMapOfMapT, int detNum, const string& varName, int val, bool overwriteFlag);
template void TemplateDataManager::SetTypeParameter<string>(map<int, map<string,string> >& aMapOfMapT, int detNum, const string& varName, string val, bool overwriteFlag);
template void TemplateDataManager::SetTypeParameter<vector<double> >(map<int, map<string,vector<double> > >& aMapOfMapT, int detNum, const string& varName, vector<double>  valVect, bool overwriteFlag);

