#ifndef DETECTORRTFMAP_h
#define DETECTORRTFMAP_h

#include <string>
#include <map>
#include <stdint.h>

class TSQLResult;

namespace CdmsDB{
  class DetectorRtfMap;
}

/**@class DetectoRtfMap
   @brief Maps detector codes onto RTF numbers
*/

class CdmsDB::DetectorRtfMap{
 public:
  DetectorRtfMap(){ Reset(); }
  ~DetectorRtfMap(){}

  DetectorRtfMap(TSQLResult* result){ LoadDbResult(result); }
  void Reset();
  int LoadDbResult(TSQLResult* result); 
  
  static std::string GetQueryString(int series_id);
  
  int GetRtfNum(uint32_t detector_code) const;

 private:
  std::map<uint32_t,int> f_rtfmap;
  
};

#endif

