#ifndef ISEGHV_h
#define ISEGHV_h

class TSQLResult;
namespace CdmsDB{
  class IsegHV;
}

#include "BatRootTypes.h"
#include <map>
#include <string>

class CdmsDB::IsegHV{
  public:
  IsegHV();
  ~IsegHV();
  IsegHV(TSQLResult* result){ LoadDbResult(result); }
  
  static std::string GetQueryString(int series_id);
  
  void Reset();
  int LoadDbResult(TSQLResult* result);
  
  typedef time_t time_type;
  struct HVinfo{
    double volts;
    double namps;
    int status;
    HVinfo(double v = BatRootTypes::kEmptyVariable, 
	   double a = BatRootTypes::kEmptyVariable, 
	   int s    = BatRootTypes::kEmptyVariable) :
    volts(v), namps(a), status(s) {}
  };
  
  //Get the offset value for detNum closest to the given time
  //the time variable is updated with the actual time found
  HVinfo GetHVinfo(int detNum, time_type& time) const;
  
  
 private:
  std::map<int, std::map<time_type, HVinfo> > f_hvmap;
};

#endif

