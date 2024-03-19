#ifndef FRIDGEDATA_h
#define FRIDGEDATA_h

#include <string>
#include <map>
#include <vector>
#include <time.h>

class TSQLResult;

namespace CdmsDB{
  class FridgeData;
}

class CdmsDB::FridgeData{
 public:
  enum FridgeVars { kBaseTemp = 0, kFEBTemp, N_FRIDGE_VARS };

 public:
  FridgeData(){ Reset(); }
  ~FridgeData() {}
  FridgeData(TSQLResult* result){ LoadDbResult(result); }
  
  void Reset();
  int LoadDbResult(TSQLResult* result);
  
  static std::string GetQueryString(int series_id);
  
  typedef time_t time_type;
  
  /// main accessor
  double GetField(FridgeVars var, time_type& time) const;
  /// get the entire row at a given time
  const std::vector<double>* GetRow(time_type& t) const;
  
 private:
  std::map<time_type, std::vector<double> > f_dbrows;
    
};

#endif
