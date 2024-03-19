//////////////////////////////////////////////////////////////////////// 
///////////////////////////////
//Class Name: ListManager
//Authors: B. Serfass
//Description:  A class for adding and retreiving values from a list. 
//
//File Import By: B. Serfass
//Creation Date: Nov. 17, 2008
//
//Modifications:
//
//////////////////////////////////////////////////////////////////////// 
///////////////////////////////



//////////////////////////////////////////////////////////////////////////
//									//
// ListManager usage:							//
//									//
//   1) in your class, define a map <string, any types>:    		//
//	map<string,double> myDoubleMap;			        	//		
//									//
//									//
//									//
//   2) set parameter (via ListManager)					//
//      double value = 1.2;						//
//      string name = "parameter_name";					//						
//      ListManager::SetParameter(myDoubleMap,name,value);		//
//									//
//      Not that:							//
//        -  if parameter doesn't exist --> create it, and set value	//
//        -  if parameter' exist --> replace value 			//
//									//
//									//
//   3) to test if a parameter exist:					//
//      bool check = ListManager::HasParameter(myDoubleMap,name);	//
// 									//
//      check = true if exist, false if not				//
//  									//
//									//
//   4) to retrieve the value						//
//      double value = ListManager::GetParameter(myDoubleMap,name);	//
//									//
//////////////////////////////////////////////////////////////////////////



#ifndef LISTMANAGER_H
#define LISTMANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include <stdint.h>
#include <cstdlib>

using namespace std;

typedef unsigned int uint;

class ListManager 
{
     public:

         
      template<class T>  static void SetParameter(map<string,T>& aMapT, const string& name, T value);
      template<class T>  static bool HasParameter(const map<string,T>& aMapT, const string& name);
      template<class T>  static T  GetParameter(const map<string,T>& aMapT, const string& name);
  
    
};




//  ___________________________________________________________________________________________



template <class T> 
void ListManager::SetParameter(map<string,T>& aMapT, const string& name, T value)
{


 typedef typename map<string,T>::iterator itMap;
 pair<itMap,bool> ret;
 
 ret = aMapT.insert(pair<string,T>(name,value));


  if (ret.second ==false)
   {

     // parameter already exist, just replace
     ret.first->second = value;
     
   }

}




// ............................................



template <class T> 
bool ListManager::HasParameter(const map<string,T>& aMapT, const string& name) 
{

 typename map<string,T>::const_iterator itMap;
 itMap = aMapT.find(name);
 
 if (itMap == aMapT.end())
    return false;
 else
    return true;
  
}
 




// .....................................................................................





template <class T> 
T ListManager::GetParameter(const map<string,T> & aMapT, const string& name)
{

 typename map<string,T>::const_iterator itMap;
 itMap = aMapT.find(name);
 
 if (itMap != aMapT.end())
   { 
   return itMap->second;
  
   } else {

     cout<< "LISTMANAGER: Parameter '"<< name << "' not found!" << endl;
     exit(1);
   }

}
 







#endif /* LISTMANAGER_H */
