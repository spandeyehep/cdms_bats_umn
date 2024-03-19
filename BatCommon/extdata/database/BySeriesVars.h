#ifndef BYSERIESVARS_h
#define BYSERIESVARS_h

#include "DetectorRtfMap.h"

#include <string>
#include <iostream>
#include <map>
#include <time.h>

namespace CdmsDB{
  class BySeriesVars;
}

class TSQLResult;
class TSQLRow;

class CdmsDB::BySeriesVars{
 public:
  //default constructor
  BySeriesVars();
  //constructor from a TSQL result
  BySeriesVars(TSQLResult* result){ LoadDbResult(result); }
  //destructor (no-op)
  ~BySeriesVars();

  //reset everything to default
  void Reset();
  
  //print results
  std::ostream& Print(std::ostream& out = std::cout, bool printrtf=true) const;
  
  //load everything from a query result
  int LoadDbResult(TSQLResult* result);
  
  //get the query string for the series
  static std::string GetQueryString(const std::string& series);
  
  //subclass RtfSetttings has thresholds
  class RtfSettings{
  public:
    RtfSettings(){};
    ~RtfSettings(){};
    RtfSettings(TSQLRow* row, int startfield = 0);
    
    std::ostream& Print(std::ostream& out = std::cout) const;
    
    static std::string GetQueryVars();
    int GetRtfNum()       const { return f_rtf_num; }
    double GetQhi()       const { return f_qhi; }
    double GetQlo()       const { return f_qlo; }
    double GetPhi()       const { return f_phi; }
    double GetPlo()       const { return f_plo; }
    double GetWhisper()   const { return f_whisper; }
    double GetDcOffset()  const { return f_offset; }
    int GetPhononTrigMode() const { return f_phononTrigMode; }
    int GetChargeTrigMode() const { return f_chargeTrigMode; }
    
  private:
    int f_rtf_num;
    double f_qhi;
    double f_qlo;
    double f_phi;
    double f_plo;
    double f_whisper;
    double f_offset;
    int f_phononTrigMode;
    int f_chargeTrigMode;
  };
  

  const std::string& GetSeries() const { return f_series;      }
  int    GetSeriesID()           const { return f_series_id;   }
  int    GetSeriesMode()         const { return f_series_mode; }
  time_t GetStartTime()          const { return f_start_time;  }
  time_t GetEndTime()            const { return f_end_time;    }
  int    GetNevents()            const { return f_nevents;     }
  double GetLivetime()           const { return f_livetime;    }
  
  //get rtf num from detcode
  int GetRtfNum(uint32_t detCode) const { return f_rtfmap.GetRtfNum(detCode); }
  
  //get rtf info by rtfnum
  const RtfSettings* GetRtfSettings(int rtfnum) const
  { 
    if(f_rtf_settings.find(rtfnum) == f_rtf_settings.end()) return 0;
    return &(f_rtf_settings.at(rtfnum));
  }
  
  //get rtf info by detCode
  const RtfSettings* GetRtfSettingsByDetCode(uint32_t detCode) const
    { return GetRtfSettings(GetRtfNum(detCode)); }
  
  
  const DetectorRtfMap& GetRtfMap(){ return f_rtfmap; }
  void SetRtfMap( const DetectorRtfMap& map){ f_rtfmap = map; }

 private:
  std::string f_series;
  int f_series_id;
  int f_series_mode;
  time_t f_start_time;
  time_t f_end_time;
  int f_nevents;
  double f_livetime;
  //rtf settings
  std::map<int,RtfSettings> f_rtf_settings;
  
  //rtf mapping
  DetectorRtfMap f_rtfmap;
};

//ostream overloads
inline std::ostream& operator<<(std::ostream& out, 
				const CdmsDB::BySeriesVars& sv)
{ return sv.Print(out); }

inline std::ostream& operator<<(std::ostream& out,
				const CdmsDB::BySeriesVars::RtfSettings& r)
{ return r.Print(out); }

#endif /*BYSERIESVARS_h*/
