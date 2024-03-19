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
//Description:      Added Remaining functions to get the data from the ROOT noise files
//                  Added data container fZipNoiseMap to contain detector information and the active channels
//                  in the detector
//Modifications:    April 5, 2012
//Modified by:      Anthony Villano (villaa@physics.umn.edu)
//Description:      Added function to get TGraphErrors for NSOF 
///////////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef FILTERDATAMANAGER_H
#define FILTERDATAMANAGER_H

// C library
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>


// ROOT library
#include "TComplex.h"

// CDMSBATS 

// DATA CLASSES
#include "SprseMatrix.h"

// ADMINISTRATION
#include "ListManager.h"
#include "DetectorConfigManager.h"

using namespace std;

class FilterDataManager
{


   public:

   //  ===== constructor/destructor ====

      FilterDataManager(const map< int, int>& detectorMap);
      FilterDataManager(); //default does nothing
      ~FilterDataManager();      
     
 
   //  =====  Read file ====

       void     ReadFile(string filename);
     

   //  ===== Get functions =====
        
      // detNum=1-30,  channel = "PA", "PB","PC","PD", "QI", "QO","QIX", "QOX"
  
       SprseMatrix     GetCOV_PD(int detNum, const string& channel) const;				//Covariance matrix for OptimalFilterNF [ANV]
       SprseMatrix     GetCOV_BASE(int detNum, const string& channel) const;				//Covariance matrix for OptimalFilterNF [ANV]
       SprseMatrix     GetCOV_BASE_HIST(int detNum, const string& channel) const;			//Covariance matrix for OptimalFilterNF converted from histogram [ANV]
       vector<double>   GetNoisePSD(int detNum, const string& channel) const;                           //NoisePSD
       vector<double>   GetNoiseFFT(int detNum, const string& channel) const;                           //NoiseFFT
       vector<double>   GetNoiseFFTsq(int detNum, const string& channel) const;                         //NoiseFFTsq
       vector<TComplex> GetTemplateConjNoiseFFT(int detNum, const string& channel) const;               //OptimalFilter
       vector<double>   GetOptimalFilterRe(int detNum, const string& channel) const;                    //OptimalFilterRe
       vector<double>   GetOptimalFilterIm(int detNum, const string& channel) const;                    //OptimalFilterIm
       double           GetNormFFT(int detNum, const string& channel) const;                            //NormFFT
       double           GetSigToNoiseSq(int detNum, const string& channel) const;                       //SigToNoiseSq
       double           GetSampleRate(int detNum, const string& channel) const;                         //SampleRate
       vector<double>   GetTemplateTime(int detNum,const string& channel) const;                        //TemplateTime
       vector<double>   GetTemplateFFTRe(int detNum, const string& channel) const;                      //TemplateFFTRe
       vector<double>   GetTemplateFFTIm(int detNum, const string& channel) const;                      //TemplateFFTIm
       vector<TComplex> GetTemplateFFT(int detNum, const string& channel) const;                        //TemplateFFT
       vector<double>   GetQXtalkInverseMatrix(int detNum, const string& side    ) const;                //QInverse
       double           GetAmpSigma(int detNum, const string& channel) const;
       double           GetDelaySigma(int detNum, double sampleRate, const string& channel) const;
    
       double           GetTemplateMax(int detNum, const string& channel) const;
       
       //  detNum=1-30

       string           GetTemplateTag(int detNum) const;                                               //templateTag
       string           GetFilterTag(int detNum) const;                                                 //filterTag
       string           GetFilterProductionDate(int detNum) const;                                      //date
       string           GetNoiseCodeGitTag(int detNum,string component="cdmsbats") const;                          //gitTag
    
       //  detType (Channel)
    
       map< int, vector<string> >   GetDetTypeMap();

   //  ===== DoCalc =====
       void     DoCalc(const DetectorConfigManager& detConfigManager);
 
   //  ===== RQ list =====
     
       void                             ConstructZipRQList();
       map< int, map< string, double> >   GetDoubleRQList() const { return fDoubleRQList; }
       map<string, double>              GetDoubleRQList(const int& detNum) const { return (fDoubleRQList.find(detNum))->second; }
   
       map<int, map<string, string> > GetStringRQList() const { return fStringRQList; }
       map<string, string> GetStringRQList(const int& detNum) const { return (fStringRQList.find(detNum))->second; }
 
    // ===== Get Map =====
    
       map< int, map< string, vector<double> > >   GetVectDoubleMap() const { return fZipMapOfMapVectDouble; }

   private:

     // set functions   

        void    SetDoubleParameter(int detNum, const string& varName, double val, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapDouble,detNum, varName,val,overwriteFlag); };
   
        void    SetStringParameter(int detNum, const string& varName, string val, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapString,detNum, varName,val,overwriteFlag); };
   	

        void    SetVectorDoubleParameter(int detNum, const string& varName, vector<double> valVect, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapVectDouble,detNum, varName,valVect,overwriteFlag) ;};
     
        void    SetVectorComplexParameter(int detNum, const string& varName, vector<TComplex> valVect, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapVectComplex,detNum, varName,valVect,overwriteFlag) ;};
       
        void    SetSprseMatrixParameter(int detNum, const string& varName, SprseMatrix valVect, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapSprseMatrix,detNum, varName,valVect,overwriteFlag) ;};
      
        template<class Type> void SetTypeParameter(map<int, map<string,Type> > &aMapOfMapT, int detNum, const string& varName, Type val, bool overwriteFlag);



      // data containers

      map< int, map<string,string> >                fZipMapOfMapString;
      map< int, map<string,double> >                fZipMapOfMapDouble;
      map< int, map<string,vector<double> > >       fZipMapOfMapVectDouble; 
      map< int, map<string,vector<TComplex> > >     fZipMapOfMapVectComplex; 
      map< int, map<string,SprseMatrix > >          fZipMapOfMapSprseMatrix; 
      map< int, vector<string> >                    fZipNoiseMap;


      // RQ list 
      vector<string>                    GetChanList(const string& multID);
      map< int, map<string,double> >    fDoubleRQList;
      map< int, map<string,string> >    fStringRQList;
     
      // detector list
      map< int, int >       fDetectorMap;
  
    

};




#endif /* FILTERDATAMANAGER_H */
