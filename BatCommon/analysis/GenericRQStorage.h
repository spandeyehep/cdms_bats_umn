#ifndef GENERICRQSTORAGE_h
#define GENERICRQSTORAGE_h

#include "TCDMSAnalysis.h"
#include <string>
#include <map>

class GenericRQStorage : public TCDMSAnalysis{
 public:
  GenericRQStorage(const std::string& classname,
		   const std::map<std::string, double> rqlist) {
    fClassName=classname; fStoreRQs=true; fRQList = rqlist;
  }
    
  ~GenericRQStorage(){}
  
};

#endif
