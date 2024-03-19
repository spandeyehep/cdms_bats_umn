#ifndef DATABASEMANAGER_h
#define DATABASEMANAGER_h

#include "BySeriesVars.h"
#include "DetectorRtfMap.h"
#include "FridgeData.h"
#include "PhononOffsets.h"
#include "IsegHV.h"

#include <string>
#include <map>

namespace CdmsDB{
  class DatabaseManager;
}

class CdmsDB::DatabaseManager{
 public:
  DatabaseManager(const std::string& dbhost = "cdmsmini.cdms-soudan.org:3306",
		  const std::string& dbuser = "readonly",
		  const std::string& dbpass = "",
		  const std::string& dbname = "monitor_development");
  ~DatabaseManager();
  
  // Get and Set database access variables
  const std::string& GetDbHost() const { return f_dbhost; }
  void SetDbHost(const std::string& dbhost){ f_dbhost = dbhost; }
  
  const std::string& GetDbUser() const { return f_dbuser; }
  void SetDbUser(const std::string& dbuser){ f_dbuser = dbuser; }
  
  const std::string& GetDbPass() const { return f_dbpass; }
  void SetDbPass(const std::string& dbpass){ f_dbpass = dbpass; }
  
  const std::string& GetDbName() const { return f_dbname; }
  void SetDbName(const std::string& dbname){ f_dbname = dbname; }
  
  
  //Check what series is currently loaded
  const std::string& GetLoadedSeries() const { return f_loaded_series; }
  int LoadSeries(const std::string& series);
  
  //Get the currently loaded series variables
  const BySeriesVars& GetBySeriesVars() const { return f_byseries_vars; }
  
  const DetectorRtfMap& GetRtfMap(){ return f_rtfmap; }
  int GetRtfNum(uint32_t detCode){ return f_rtfmap.GetRtfNum(detCode); }
  
  const FridgeData& GetFridgeData() const { return f_fridgedata; }
  const PhononOffsets& GetPhononOffsets() const { return f_phononoffsets; }

  
  //RQ lists
  const std::map<std::string, double>& 
    ConstructEventRQs();
  const std::map<std::string, double>& 
    ConstructDetectorRQs(int detType, int detNum);
  const std::map<std::string, double>& 
    ConstructPulseRQs(const std::string& sensorType);
  
  
  const std::map<std::string, double>& GetEventRQList() const 
  { return fEvRQList; }
  std::map<std::string, double> GetDetectorRQList(int detNum) const
    { return fDetRQMap.count(detNum) ? 
	fDetRQMap.at(detNum) : std::map<std::string, double>();  }
  const std::map<std::string, double>& GetPulseRQList() const
  { return fPulseRQList; }
  
  void StoreEventRQs(time_t time);
  void StoreDetectorRQs(time_t time, int detType, int detNum);
  void StorePulseRQs(time_t time, uint32_t detCode, 
		     const std::string& sensorType);
  
 private:
  std::string f_loaded_series;
  BySeriesVars f_byseries_vars;
  DetectorRtfMap f_rtfmap;
  FridgeData f_fridgedata;
  PhononOffsets f_phononoffsets;
  IsegHV f_iseghv;
  
  std::map<std::string, double> fEvRQList;
  std::map<std::string, double> fPulseRQList;
  std::map<int, std::map<std::string, double> > fDetRQMap;

  std::string f_dbhost;          //< hostname of database server
  std::string f_dbuser;          //< username to log in with 
  std::string f_dbpass;          //< password to log in with
  std::string f_dbname;          //< name of the database to use
};


#endif

