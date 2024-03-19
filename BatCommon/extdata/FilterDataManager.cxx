///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: FilterDataManager
//Authors: B. Serfass
//Description:  This class read the Filter file (noise/template file), store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:    August 5, 2011
//Modified by:      Carlos Eduardo Martinez Amaya (carlos@owl.phy.queensu.ca)
//Description:      Implemented the functions to get the data from the ROOT noise files, including the one to
//                  fill the data container fZipNoiseMap which contains the detector number and the active channels
//                  in each detector
//Modifications:    April 5, 2012
//Modified by:      Anthony Villano (villaa@physics.umn.edu)
//Description:      Added function to get TGraphErrors for NSOF 
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "FilterDataManager.h"

// ROOT library
#include "TH1D.h"
#include "TTree.h"
#include "TFile.h"
#include "TKey.h"
#include "TGraphErrors.h"


// CDMS library
#include "PulseTools.h"
#include "BatRootTypes.h"
#include "ChannelMapHelper.h"

////////////////////////////////////////////////////////


     
// constructor
FilterDataManager::FilterDataManager(const map< int, int >& detectorMap)  
{   

  //Construct the RQ list
  fDetectorMap = detectorMap;
  ConstructZipRQList();
   

}

// default constructor
FilterDataManager::FilterDataManager()
{   


}
 


//destructor 
FilterDataManager::~FilterDataManager() 
{ 

}



//  =================  RQ list  =================


void FilterDataManager::ConstructZipRQList()
{
   double initValDouble = -999999.;
   string initValString = "none";
 
  
   // double RQ list    
   vector<string> chanQList = GetChanList("chargechan");
   vector<string> chanPList = GetChanList("phononchan");
 
   // loop ZIPs
   map< int, int >::iterator it;
   for(it = fDetectorMap.begin(); it!=fDetectorMap.end(); it++)
    {
        
     // detector number
     int detNum = it->first;
      
     // --- double parameters ---
 
     // map RQs 
     map<string, double> mapDoubleRQ;
      
     // charge quantities
     for (uint listItr=0;listItr<chanQList.size();listItr++)
      {     
      string ampsigName = chanQList[listItr] + "ampsig";
      string delaysigName = chanQList[listItr] + "delaysig";
      
      mapDoubleRQ.insert(pair<string,double>(ampsigName,initValDouble));
      mapDoubleRQ.insert(pair<string,double>(delaysigName,initValDouble));
     }
  
    // phonon quantities
    for (uint listItr=0;listItr<chanPList.size();listItr++)
     {   
      string ampsigName = chanPList[listItr] + "ampsig";
      string delaysigName = chanPList[listItr] + "delaysig";
      mapDoubleRQ.insert(pair<string,double>(ampsigName,initValDouble));
      mapDoubleRQ.insert(pair<string,double>(delaysigName,initValDouble));
   
     }

     fDoubleRQList.insert(pair<int, map<string, double> >(detNum, mapDoubleRQ));
    

     // --- string parameters ---

     // map RQs 
     map<string, string> mapStringRQ;

     mapStringRQ.insert(pair<string,string>("templateTag", initValString));
     mapStringRQ.insert(pair<string,string>("filterTag", initValString)); 
     mapStringRQ.insert(pair<string,string>("filterProductionDate", initValString));   
     mapStringRQ.insert(pair<string,string>("noiseCodeGitTag_cdmsbats", initValString)); 
     mapStringRQ.insert(pair<string,string>("noiseCodeGitTag_batcommon", initValString)); 
 
     fStringRQList.insert(pair<int, map<string, string> >(detNum, mapStringRQ));
   
  } // end Zip list


  return;
}


      
vector<string> FilterDataManager::GetChanList(const string& multID)
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


//  =================  Get functions =================

/////////////////// Matrix objects ///////////
/////////////////// COV_PD ///////////////////
SprseMatrix FilterDataManager::GetCOV_PD(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,SprseMatrix > >::const_iterator zipMap = fZipMapOfMapSprseMatrix.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapSprseMatrix.end())
    { 
        cerr <<"FilterDataManager::GetCOV_PD:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"cov_pd"; 
    return ListManager::GetParameter(zipMap->second,keyName);

}
/////////////////// PSD_BASE ///////////////////
SprseMatrix FilterDataManager::GetCOV_BASE(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,SprseMatrix > >::const_iterator zipMap = fZipMapOfMapSprseMatrix.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapSprseMatrix.end())
    { 
        cerr <<"FilterDataManager::GetCOV_BASE:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"cov_base"; 
    return ListManager::GetParameter(zipMap->second,keyName);

}
/////////////////// PSD_BASE (from TGraphErrors converted from hist) ///////////////////
SprseMatrix FilterDataManager::GetCOV_BASE_HIST(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,SprseMatrix > >::const_iterator zipMap = fZipMapOfMapSprseMatrix.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapSprseMatrix.end())
    { 
        cerr <<"FilterDataManager::GetCOV_BASE_HIST:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"NoisePSDMat"; 
    return ListManager::GetParameter(zipMap->second,keyName);

}
/////////////////// NoisePSD ////////////////
vector<double> FilterDataManager::GetNoisePSD(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetNoisePSD:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"NoisePSD"; 
    return ListManager::GetParameter(zipMap->second,keyName);
    
}
//////////////////////////////////////////////

/////////////////// NoiseFFT ////////////////
vector<double> FilterDataManager::GetNoiseFFT(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetNoiseFFT:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"NoiseFFT"; 
    return ListManager::GetParameter(zipMap->second,keyName);
    
}
//////////////////////////////////////////////

/////////////////// NoiseFFTsq ////////////////
vector<double> FilterDataManager::GetNoiseFFTsq(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetNoiseFFTsq:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"NoiseFFTsq"; 
    return ListManager::GetParameter(zipMap->second,keyName);
    
}
//////////////////////////////////////////////

/////////////////// OptimalFilter ////////////////
vector<double> FilterDataManager::GetOptimalFilterRe(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetNoiseFFTsq:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"OptimalFilterRe"; 
    return ListManager::GetParameter(zipMap->second,keyName);
    
}
/////////
vector<TComplex> FilterDataManager::GetTemplateConjNoiseFFT(int detNum, const string& channel) const
{
    
    
    string keyNameIm = channel+"OptimalFilterIm";
    vector<double> OFImVect;
    
    string keyNameRe = channel+"OptimalFilterRe";
    vector<double> OFReVect;
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    OFImVect = ListManager::GetParameter(zipMap->second,keyNameIm);
    OFReVect = ListManager::GetParameter(zipMap->second,keyNameRe); 
    
    int nBins =  OFImVect.size();
    vector<TComplex> OptimalFilterVect;
    for(int binItr = 0; binItr < nBins; binItr++)
    { 
        TComplex OptimalFilter(OFReVect[binItr],OFImVect[binItr]);
        OptimalFilterVect.push_back(OptimalFilter);
    }
    
    
    return OptimalFilterVect ;
    
}
/////////
vector<double> FilterDataManager::GetOptimalFilterIm(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetNoiseFFTsq:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"OptimalFilterIm"; 
    return ListManager::GetParameter(zipMap->second,keyName);
    
}
//////////////////////////////////////////////////

/////////////////// NormFFT ////////////////
double FilterDataManager::GetNormFFT(int detNum, const string& channel) const
{
    
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"NormFFT";  
    
    return (ListManager::GetParameter(zipMap->second,keyName))[0];
    
}
////////////////////////////////////////////

/////////////////// SigToNoiseSq ////////////////
double FilterDataManager::GetSigToNoiseSq(int detNum, const string& channel) const
{
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"SigToNoiseSq";  
    return (ListManager::GetParameter(zipMap->second,keyName))[0];
    
}
////////////////////////////////////////////////

//This routine deprecated with implementation of DetectorConfigRecord
/////////////////// SampleRate ////////////////
// double FilterDataManager::GetSampleRate(int detNum, const string& channel) const
// {


//   // get ZIP map

//   map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);

//  // check map exist
//   if(zipMap == fZipMapOfMapVectDouble.end())
//    { 
//      cerr <<"FilterDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
//      exit(1);
//    } 


//  string keyName = channel+"SampleRate";  

//  return (ListManager::GetParameter(zipMap->second,keyName))[0];

// }
////////////////////////////////////////////////

/////////////////// TemplateTime ////////////////
vector<double> FilterDataManager::GetTemplateTime(int detNum, const string& channel) const
{
  
  // get ZIP map

  map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
 
  // check map exist
  if(zipMap == fZipMapOfMapVectDouble.end())
   { 
     cerr <<"FilterDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 


  string keyName = channel+"TemplateTime";
  return ListManager::GetParameter(zipMap->second,keyName);

}

double FilterDataManager::GetTemplateMax(int detNum, const string& channel) const
{
    double max = -999999.;
    
    vector<double> myTemplate = GetTemplateTime(detNum, channel);
    int nbins = myTemplate.size();  
    
    for (int i=0; i<nbins; i++) {
        
        if (myTemplate[i] > max) {
            max = myTemplate[i];
        }
        
    }
    
    return max;
    
}
////////////////////////////////////////////////

/////////////////// TemplateFFT ////////////////
vector<double> FilterDataManager::GetTemplateFFTRe(int detNum, const string& channel) const
{
    
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"TemplateFFTRe";  
    
    return (ListManager::GetParameter(zipMap->second,keyName));
    
}
/////////
vector<TComplex> FilterDataManager::GetTemplateFFT(int detNum, const string& channel) const
{
    
    
    string keyNameIm = channel+"TemplateFFTIm";
    vector<double> TemplateImVect;
    
    string keyNameRe = channel+"TemplateFFTRe";
    vector<double> TemplateReVect;
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    TemplateImVect = ListManager::GetParameter(zipMap->second,keyNameIm);
    TemplateReVect = ListManager::GetParameter(zipMap->second,keyNameRe); 
    
    int nBins =  TemplateImVect.size();
    vector<TComplex> TemplateFFTVect;
    for(int binItr = 0; binItr < nBins; binItr++)
    { 
        TComplex TemplateFFT(TemplateReVect[binItr],TemplateImVect[binItr]);
        TemplateFFTVect.push_back(TemplateFFT);
    }
    
    
    return TemplateFFTVect ;
    
}
/////////
vector<double> FilterDataManager::GetTemplateFFTIm(int detNum, const string& channel) const
{
    
    
    // get ZIP map
    
    map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
    
    // check map exist
    if(zipMap == fZipMapOfMapVectDouble.end())
    { 
        cerr <<"FilterDataManager::GetTemplate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
        exit(1);
    } 
    
    
    string keyName = channel+"TemplateFFTIm";  
    
    return (ListManager::GetParameter(zipMap->second,keyName));
    
}
////////////////////////////////////////////////



/////////////////// QInverse ////////////////
vector<double> FilterDataManager::GetQXtalkInverseMatrix(int detNum, const string& side) const
{

  // get ZIP map

  map< int, map<string,vector<double> > >::const_iterator zipMap = fZipMapOfMapVectDouble.find(detNum);
 
 // check map exist
  if(zipMap == fZipMapOfMapVectDouble.end())
   { 
     cerr <<"FilterDataManager::GetQXtalkInverseMatrix:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 

 string keyName;
 if (side == "S1" || side == "S2")
   keyName = "Q" + side + "inverse";
 else
   keyName = "Qinverse";
 

 return ListManager::GetParameter(zipMap->second,keyName);


}
//////////////////////////////////////////////



/////////////////// AmpSigma ////////////////
double FilterDataManager::GetAmpSigma(int detNum, const string& channel) const
{
   double ampSigma = 0.;

   // get template FFT
   vector<TComplex> templateFFT = GetTemplateFFT(detNum, channel);

   // get Noise FFTSQ
   vector<double> noiseFFTsq = GetNoiseFFTsq(detNum,channel);

   int nBins = noiseFFTsq.size();
   for(int binItr=1; binItr < nBins; binItr++)
     { //ignoring DC component (infinity)
   
       ampSigma += pow(TComplex::Abs(templateFFT[binItr]), 2)/noiseFFTsq[binItr];
     }
   if (ampSigma>0.) ampSigma = 1/sqrt(ampSigma);
  return ampSigma;
}
//////////////////////////////////////////////


/////////////////// DelaySigma ////////////////
double FilterDataManager::GetDelaySigma(int detNum, double sampleRate, const string& channel) const
{
   double delaySigma = 0.;

  
   // get template and Noise  FFT
   vector<TComplex> templateFFT = GetTemplateFFT(detNum, channel);
   vector<double> noiseFFTsq = GetNoiseFFTsq(detNum,channel);


   // number ADC bins
   int nBins = noiseFFTsq.size();
  
   // sample Rate
   //TempLLH   double sampleRate = GetSampleRate(detNum, channel); 

   // construct vector of frequencies
   vector<double> freqVect;
   for(int ibin=0;ibin <nBins; ibin++)
    {
      double freq;
      if (ibin<nBins/2)
           freq = ibin*sampleRate/nBins;
 
      if (ibin>=nBins/2)
           freq = (ibin-nBins)*sampleRate/nBins;

      freqVect.push_back(freq);
    }

   for(int binItr=1; binItr < nBins; binItr++)
     {//ignoring DC component (infinity)
   
       delaySigma +=  pow(2.0*TMath::Pi()*freqVect[binItr],2)*pow(TComplex::Abs(templateFFT[binItr]), 2)/noiseFFTsq[binItr];
     }

   if (delaySigma > 0.) delaySigma = 1/sqrt(delaySigma);
  return delaySigma;
}
///////////////////////////////////////////////



///////////////// Tagging Information /////////////////
string FilterDataManager::GetTemplateTag(int detNum) const
 {
   // get ZIP map
   map< int, map<string,string> >::const_iterator zipMap = fZipMapOfMapString.find(detNum);
 
  // check map exist
  if(zipMap ==  fZipMapOfMapString.end())
   { 
     cerr <<"FilterDataManager::GetTemplateTag:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 


  return ListManager::GetParameter(zipMap->second,"templateTag");

 }

///////////////

string FilterDataManager::GetFilterTag(int detNum) const
 {
   // get ZIP map
   map< int, map<string,string> >::const_iterator zipMap = fZipMapOfMapString.find(detNum);
 
  // check map exist
  if(zipMap ==  fZipMapOfMapString.end())
   { 
     cerr <<"FilterDataManager::GetFilterTag:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 


  return ListManager::GetParameter(zipMap->second,"filterTag");

 }

///////////////

string FilterDataManager::GetFilterProductionDate(int detNum) const
 {
   // get ZIP map
   map< int, map<string,string> >::const_iterator zipMap = fZipMapOfMapString.find(detNum);
 
  // check map exist
  if(zipMap ==  fZipMapOfMapString.end())
   { 
     cerr <<"FilterDataManager::GetFilterProductionDate:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 


  return ListManager::GetParameter(zipMap->second,"filterProductionDate");

 }

///////////////

string FilterDataManager::GetNoiseCodeGitTag(int detNum, string component) const
 {
   // get ZIP map
   map< int, map<string,string> >::const_iterator zipMap = fZipMapOfMapString.find(detNum);
 
  // check map exist
  if(zipMap ==  fZipMapOfMapString.end())
   { 
     cerr <<"FilterDataManager::GetNoiseCodeGitTag:  ERROR! Map for detector "<< detNum << " doesn't exist!"<< endl;
     exit(1);
   } 


  string tagname;
  if(component=="cdmsbats" || component=="batcommon")
    tagname="noiseCodeGitTag_"+component;
  else
    tagname="noiseCodeGitTag_cdmsbats";
  return ListManager::GetParameter(zipMap->second,tagname);

 }
///////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  =================  Get Detector (Channel) Type =================

map< int, vector<string> >   FilterDataManager::GetDetTypeMap()
{
    map< int, map< string, vector<double> > >   zipNoiseMap = GetVectDoubleMap();
    
    map< int, map< string, vector<double> > >::iterator zipMapIt;
    map< string, vector<double> >::iterator zipDetec;
    
    // loop through all detectors
    for (zipMapIt = zipNoiseMap.begin() ; zipMapIt != zipNoiseMap.end() ; zipMapIt++) 
    {
        int detNum = (*zipMapIt).first;                                         // Iterator to get the detector number
        map<string, vector<double> >    variablesMap = (*zipMapIt).second;      // Map to get the Detector Types variables
        vector<string>                  newVect;                                // Vector to contain Detector Types
        string                          key ("NoisePSD");
        
        for (zipDetec = variablesMap.begin(); zipDetec != variablesMap.end() ; zipDetec++) 
        {
            string  detType;
            string  detTypeTemp ((*zipDetec).first);
            size_t  found;
            
            found = detTypeTemp.find(key);
            
            if (found != string::npos) 
            {
                detType = detTypeTemp.erase(found,key.size());
                newVect.insert (newVect.begin(),detType);
            }
        }
        
        fZipNoiseMap.insert( pair<int, vector<string> >(detNum,newVect) );
    }
    
    return fZipNoiseMap;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  =================  Calc parameters =================

void  FilterDataManager::DoCalc(const DetectorConfigManager& detConfigManager)
{

   // loop ZIPs
  
   map< int, int >::iterator it;
   for(it = fDetectorMap.begin(); it!=fDetectorMap.end(); it++)
   {
 

    // detector number
    int detNum = it->first;
    int detType = it->second;

    // sample rates
    double phononSampleRate = detConfigManager.GetSampleRate(detNum, "phonon"); 
    double chargeSampleRate = detConfigManager.GetSampleRate(detNum, "charge");

    // ------- double ZIP RQ list -------

    // get double RQ list
    map<int, map<string,double> >::iterator zipDoubleRQmap = fDoubleRQList.find(detNum);
   

    // Get channel list
    vector<string> chanQList;
    ChannelMapHelper::FillChargeChannelList(detType, chanQList);

    vector<string> chanPList;
    ChannelMapHelper::FillPhononChannelList(detType, chanPList);


    // charge quantities
    for (uint listItr=0;listItr<chanQList.size();listItr++)
     {     
      string ampsigName = chanQList[listItr] + "ampsig";
      string delaysigName = chanQList[listItr] + "delaysig";
      (zipDoubleRQmap->second)[ampsigName] = GetAmpSigma(detNum, chanQList[listItr]);
      (zipDoubleRQmap->second)[delaysigName] = GetDelaySigma(detNum, chargeSampleRate, chanQList[listItr]);
     }   
      
   // phonon quantities
    for (uint listItr=0;listItr<chanPList.size();listItr++)
     {     
      string ampsigName = chanPList[listItr] + "ampsig";
      string delaysigName = chanPList[listItr] + "delaysig";
      (zipDoubleRQmap->second)[ampsigName] = GetAmpSigma(detNum, chanPList[listItr]);
      (zipDoubleRQmap->second)[delaysigName] = GetDelaySigma(detNum, phononSampleRate, chanPList[listItr]);
     }


   // ------- string RQ list -------

   // get string ZIP RQ list
   map<int, map<string,string> >::iterator zipStringRQmap = fStringRQList.find(detNum);
   
   // templae string
   string templateTagStr =GetTemplateTag(detNum);
   (zipStringRQmap->second)["templateTag"] = templateTagStr;

   // filter tag
   string filterTagStr =GetFilterTag(detNum);
   (zipStringRQmap->second)["filterTag"] =filterTagStr;

   // filter production date
   string filterProductionDateStr =GetFilterProductionDate(detNum);
   (zipStringRQmap->second)["filterProductionDate"] =filterProductionDateStr;

   // noise code git tag
   string noiseCodeGitTagStr_cdmsbats =GetNoiseCodeGitTag(detNum,"cdmsbats");
   string noiseCodeGitTagStr_batcommon =GetNoiseCodeGitTag(detNum,"batcommon");
   (zipStringRQmap->second)["noiseCodeGitTag_cdmsbats"] =noiseCodeGitTagStr_cdmsbats;
   (zipStringRQmap->second)["noiseCodeGitTag_batcommon"] =noiseCodeGitTagStr_batcommon;

  
 }   

return;

}

//  ================= read configuration file  =================


void  FilterDataManager::ReadFile(string filename)
{

// open file
 
TFile file(filename.c_str(),"READ");

if (!file.IsOpen()) 
 { 
    cerr << "FilterDataManager::ReadFile: ERROR! file '" << filename <<"' not found..." << endl;
    exit(1);
 
  } else {
     cout << "\n**** Reading Filter file: " << filename << endl;
  }
    

// loop over list of objetcs in the file

TKey *aKey;
TIter fileKeysItr(file.GetListOfKeys());
while ((aKey = (TKey *) fileKeysItr()))
{  

  // Get object
  TObject* objDir = aKey->ReadObj();

  // check if object is a directory
  if (!strcmp(objDir->ClassName(),"TDirectoryFile"))
   { 
    
     TDirectory *dir = (TDirectory* ) objDir;       
     string dirName = dir->GetName();


     
     if (dirName=="infoDir")  {
	
      // loop objects in info directory
      TKey* aKey;;
      TIter keysItr(dir->GetListOfKeys()); 
      while ((aKey = (TKey *) keysItr()))
       { 
        // get object  
        TObject* obj = aKey->ReadObj();
         
        // check if TTree object
        if(!strcmp(obj->ClassName(),"TTree"))
         { 

          TTree* tree = (TTree*) obj;
          
          string treeName = tree->GetName();
          string detNumStr = treeName.substr(7);
     	  int detNum =   atoi(detNumStr.c_str());

          // get info
	  char templateTagChar[50];
  	  char filterTagChar[50];
          char filterProductionDateChar[50];
          char noiseCodeGitTagChar_cdmsbats[1024];
          char noiseCodeGitTagChar_batcommon[1024];
	  
          tree->SetBranchAddress("templateTag",templateTagChar);
          tree->SetBranchAddress("filterTag",filterTagChar);
 	      tree->SetBranchAddress("date",filterProductionDateChar);
 	      tree->SetBranchAddress("gitTag_cdmsbats",noiseCodeGitTagChar_cdmsbats);
 	      tree->SetBranchAddress("gitTag_batcommon",noiseCodeGitTagChar_batcommon);
	      tree->GetEntry(0);

	  // store info
          string templateTag = templateTagChar;
          string filterTag = filterTagChar;
          string filterProductionDate = filterProductionDateChar;
          string noiseCodeGitTag_cdmsbats = noiseCodeGitTagChar_cdmsbats;
          string noiseCodeGitTag_batcommon = noiseCodeGitTagChar_batcommon;
        
          SetStringParameter(detNum,"templateTag",templateTag, true); 
          SetStringParameter(detNum,"filterTag",filterTag, true);
          SetStringParameter(detNum,"filterProductionDate",filterProductionDate, true);
          SetStringParameter(detNum,"noiseCodeGitTag_cdmsbats",noiseCodeGitTag_cdmsbats, true);
          SetStringParameter(detNum,"noiseCodeGitTag_batcommon",noiseCodeGitTag_batcommon, true);
	
	 }

        //clean-up
        delete obj;

        } // end loop objects

       } else {

       if (dirName.find("pulsedumps")!=string::npos) continue;
             

       string detNumStr = dirName.substr(3);
       int detNum =   atoi(detNumStr.c_str());
     

       // loop objetcs in the ZIP directory
       TKey* aKey;
       TIter keysItr(dir->GetListOfKeys()); 
       while ((aKey = (TKey *) keysItr()))
        { 
         // get object  
         TObject* obj = aKey->ReadObj();
         
          // TH1D object
          if(!strcmp(obj->ClassName(),"TH1D"))
            {
              TH1D* histoTemp = (TH1D*) obj;
              string histoName = histoTemp->GetName();
              vector<double> vectTemp = PulseTools::TH1D2Vector(*histoTemp);
              SetVectorDoubleParameter(detNum,histoName,vectTemp,true);

	      //get a copy of the noisePSD in SprseMatrix form here for OptimalFilterPhononNS [ANV]
              //string keyName = channel+"NoisePSD"; 
	      if(histoName == "PTNoiseFFTsq"){
                //DC component removed by NoiseBuilder reinstate as max for invertability [ANV]
                double max = PulseTools::MaxADC(vectTemp);
                vectTemp[0] = max;

                //convert to TGraphErrors, convert to matrix and save
                TGraphErrors *graphTemp = PulseTools::Vector2TGraphErrors(vectTemp,1.0);
		graphTemp->SetName("PTNoisePSDMat");
                string graphName = graphTemp->GetName();
                SprseMatrix smatTemp = SprseMatrix(graphTemp,vectTemp.size(),vectTemp.size(),false);
                SetSprseMatrixParameter(detNum,graphName,smatTemp,true);
	        //TGraphErrors gets copied in SprseMatrix, so get rid of it
		//since it's not explicitly in the filter file. 
	        delete graphTemp;
	      }
            }  
	   // TGraphErrors object [ANV] Save Downcast version here if necessary
          if(!strcmp(obj->ClassName(),"TGraphErrors"))
            {
              TGraphErrors* graphTemp = (TGraphErrors*) obj;
              string graphName = graphTemp->GetName();
	      //FIXME being explicit here about matrix size is a problem should find a way to fix [ANV]
              SprseMatrix smatTemp = SprseMatrix(graphTemp,4096,4096,false);
              SetSprseMatrixParameter(detNum,graphName,smatTemp,true);
            }  
        
        // clean-up
        delete obj;
 
        }
       }
      }
     else
      {
       cerr <<"FilterDataManager::ReadFile:  ERROR! Problem reading filter file. Check the file!"<< endl;
       exit(1);
      }

 // clean-up
 delete objDir;

 } // end list of objects in file

 file.Close();

}

//  ================= Set functions  ==================


template <class Type>
void FilterDataManager::SetTypeParameter(map<int,map<string,Type> >& aMapOfMapT, int detNum, const string& varName, Type val, bool overwriteFlag)
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
      cerr <<"FilterDataManager:SetTypeParameter: ERROR! parameter " << varName << " for detector "<< detNum << " already set!"<< endl;
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

template void FilterDataManager::SetTypeParameter<double>(map<int, map<string,double> >& aMapOfMapT, int detNum, const string& varName, double val, bool overwriteFlag);
template void FilterDataManager::SetTypeParameter<string>(map<int, map<string,string> >& aMapOfMapT, int detNum, const string& varName, string val, bool overwriteFlag);
template void FilterDataManager::SetTypeParameter<vector<double> >(map<int, map<string,vector<double> > >& aMapOfMapT, int detNum, const string& varName, vector<double>  valVect, bool overwriteFlag);
template void FilterDataManager::SetTypeParameter<vector<TComplex> >(map<int, map<string,vector<TComplex> > > & aMapOfMapT, int detNum, const string& varName, vector<TComplex>  valVect, bool overwriteFlag);
template void FilterDataManager::SetTypeParameter<SprseMatrix>(map<int, map<string,SprseMatrix> > & aMapOfMapT, int detNum, const string& varName, SprseMatrix  valVect, bool overwriteFlag);

