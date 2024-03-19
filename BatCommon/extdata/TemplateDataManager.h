///////////////////////////////////////////////////////////////////////////////////////////////////////
//Class Name: TemplateDataManager
//Authors: B. Serfass
//Description:   This class read the charge template file, construct the phonon template,  store the informations, and manage access to 
//to the parameters  
//
//File Import By: B. Serfass
//Creation Date: Dec. 5, 2008
//
//Modifications:
//
///////////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef TEMPLATEDATAMANAGER_H
#define TEMPLATEDATAMANAGER_H

// Standard library
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// ROOT library
#include "TTree.h"
#include "TFile.h"
#include "TH1D.h"
#include "TKey.h"
#include "TGraphErrors.h"

// CDMSBATS 
#include "ListManager.h"
#include "UserDataManager.h"

using namespace std;

class TemplateDataManager
{


   public:

    // ---- constructor/destructor -----

      TemplateDataManager();
      ~TemplateDataManager();      
     

    // ---- Get functions -----

    // charge template "QI", "QO","QIX", "QOX"
     vector<double> GetTemplate(int detNum, const string& channel) const;
    
    // 2-exponential form
     vector<double> GetDoubleExpForm(double riseTime, double fallTime, int nBins, double sampleRate) const;
     vector<double> GetDoubleExpForm(double riseTime, double fallTime, int nBins, int preTrigger, double sampleRate) const;	
 
    // vector of zeros
      vector<double> GetZerosForm(int nBins) const;

    // template tag
      string GetTemplateTag(int detNum) const;


    // Get Covariance matrix
      TGraphErrors* GetCovarianceMatrix(int detNum, const string& channel) const;

 
    // ---- Read file -------

      void ReadAllFiles(const string& templateDir, const string& inputSeries, 
			const map<int, int>& detectorTypeMap, 
                        const map<int, string>& detectorPolarityMap,  
			const UserDataManager& userData);
    
      void ReadFile(int detNum, const string& dirType, const string& polarity, const string& templateDir, const string& inputSeries);
     

  


   private:

      // get file name
      string GetFileName(int detNum, const string& templateDir, const string& inputSeries);


     // set functions   

      void SetIntParameter(int detNum, const string& varName, int val, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapInt,detNum, varName,val,overwriteFlag); };
   
      void SetStringParameter(int detNum, const string& varName, string val, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapString,detNum, varName,val,overwriteFlag); };
   

      void SetVectorDoubleParameter(int detNum, const string& varName, vector<double> valVect, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapVectDouble,detNum, varName,valVect,overwriteFlag) ;};
    

      void SetTGraphErrorsParameter(int detNum, const string& varName, TGraphErrors *valVect, bool overwriteFlag)
                          { SetTypeParameter(fZipMapOfMapGraphErrors,detNum, varName,valVect,overwriteFlag) ;};
       
      template<class Type> void SetTypeParameter(map<int, map<string,Type> > &aMapOfMapT, int detNum, const string& varName, Type val, bool overwriteFlag);



      // useful functions
      string trim(string str);
      vector<string> Tokenize(string aStr);
   


      // data containers
      map< int, map<string,int > >  fZipMapOfMapInt; 
      map< int, map<string,string > >  fZipMapOfMapString; 
      map< int, map<string,vector<double> > >  fZipMapOfMapVectDouble; 
      map< int, map<string,TGraphErrors* > >  fZipMapOfMapGraphErrors; 
     


     
     

    

};




#endif /* TEMPLATEDATAMANAGER_H */
