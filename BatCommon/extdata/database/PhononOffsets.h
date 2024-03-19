#ifndef PHONONOFFSETS_h
#define PHONONOFFSETS_h

#include <map>
#include <string>
#include <time.h>
#include <stdint.h>

class TSQLResult;

namespace CdmsDB{
  class PhononOffsets;
}

class CdmsDB::PhononOffsets{
 public:
  PhononOffsets();
  ~PhononOffsets();
  PhononOffsets(TSQLResult* result){ LoadDbResult(result); }
  
  static std::string GetQueryString(int series_id);
  
  void Reset();
  int LoadDbResult(TSQLResult* result);
  
  typedef time_t time_type;
  
  //Get the offset value for detCode closest to the given time
  //the time variable is updated with the actual time found
  double GetOffset(uint32_t detCode, time_type& time) const;
  
 private:
  std::map<uint32_t, std::map<time_type, double> > f_offsetmap;
};

#endif
