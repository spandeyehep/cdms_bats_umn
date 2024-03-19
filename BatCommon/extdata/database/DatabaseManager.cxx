#include "DatabaseManager.h"
#include "BySeriesVars.h"
#include "BatRootTypes.h"

#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"

#include <sstream>
#include <memory>

using CdmsDB::DatabaseManager;
using CdmsDB::BySeriesVars;


DatabaseManager::DatabaseManager(const std::string& dbhost,
				 const std::string& dbuser,
				 const std::string& dbpass,
				 const std::string& dbname) :
  f_loaded_series(""), f_byseries_vars(), 
  f_dbhost(dbhost), f_dbuser(dbuser), f_dbpass(dbpass), f_dbname(dbname)
{
}

DatabaseManager::~DatabaseManager()
{}

int DatabaseManager::LoadSeries(const std::string& series)
{
  std::cout<<"DatabaseManager: Loading data for series "<<series<<std::endl;
  time_t start = time(0);
  
  if(f_loaded_series == series){
    std::cout<<"DatabaseManager: Series "<<series<<" already loaded."
	     <<std::endl;
    return 0;
  }
  
  //attempt to open the database connection
  std::stringstream dbfullhost;
  dbfullhost<<"mysql://"<<f_dbhost<<"/"<<f_dbname;
  //put the server in an auto_ptr to simplify closing it
  std::auto_ptr<TSQLServer> server (
	 TSQLServer::Connect(dbfullhost.str().c_str(),
			     f_dbuser.c_str(), f_dbpass.c_str() )
  );
  
  if(!server.get()){
    std::cerr<<"DatabaseManager: ERROR: Unable to open database connection\n";
    return -1;
  }
  
  //query the database for the byseries information
  TSQLResult* result = 
    server->Query(BySeriesVars::GetQueryString(series).c_str());
  if(!result){
    std::cerr<<"DatabaseManager: ERROR querying database: "
	     <<server->GetErrorMsg()<<"\n";
    return -2;
  }
  
  //if we get here, the result should be valid, so we can go ahead
  int retval = 0;
  if( (retval = f_byseries_vars.LoadDbResult(result)) ){
    //there was an error
    return retval;
  }
  
  //load the detector rtf mapping as well
  int id = f_byseries_vars.GetSeriesID();
  result = server->Query(DetectorRtfMap::GetQueryString(id).c_str());
  if(!result){
    std::cerr<<"DatabaseManager: ERROR querying database: "
	     <<server->GetErrorMsg()<<"\n";
    return -2;
  }
  
  if( (retval = f_rtfmap.LoadDbResult(result)) )
    return retval;
  f_byseries_vars.SetRtfMap(f_rtfmap);

  //there may not be any by-time stuff, so set the loaded series here  
  f_loaded_series = series;

  //load the various bytime stuff
  //fridge data
  result = server->Query(FridgeData::GetQueryString(id).c_str());
  f_fridgedata.LoadDbResult(result);
  //phonon offsets
  result = server->Query(PhononOffsets::GetQueryString(id).c_str());
  f_phononoffsets.LoadDbResult(result);
  //CDMSlite HV
  result = server->Query(IsegHV::GetQueryString(id).c_str());
  f_iseghv.LoadDbResult(result);
  //if we got here, everything was successful
  std::cout<<"DatabaseManager: Finished loading data in "<<time(0)-start
	   <<" seconds."<<std::endl;
  return 0;
}

const std::map<std::string, double>& DatabaseManager::ConstructEventRQs()
{
  fEvRQList.clear();
  fEvRQList["BaseTemp"] = BatRootTypes::kEmptyVariable;
  fEvRQList["FEBTemp"] = BatRootTypes::kEmptyVariable;
  return fEvRQList;
}

void DatabaseManager::StoreEventRQs(time_t time)
{
  //currently, only basetemp and fridgetemp are event level
  //time is updated by GetField, so copy before sending
  time_t tcopy = time;
  fEvRQList["BaseTemp"] = f_fridgedata.GetField(FridgeData::kBaseTemp, tcopy);
  fEvRQList["FEBTemp"] = f_fridgedata.GetField(FridgeData::kFEBTemp, time);
  
}

const std::map<std::string, double>& 
DatabaseManager::ConstructPulseRQs(const std::string& sensorType) 
{
  fPulseRQList.clear();
  double initVal = BatRootTypes::kEmptyVariable;
  //right now only phonon types
  if(sensorType == "phonon"){
    fPulseRQList["offset"] = initVal;
    fPulseRQList["threshlow"] = initVal;
    fPulseRQList["trigmode"] = initVal;
  }
  return fPulseRQList;
}

void DatabaseManager::StorePulseRQs(time_t time, uint32_t detCode,
				   const std::string& sensorType)
{
  ConstructPulseRQs(sensorType);
  if(sensorType=="phonon")
  {
    const BySeriesVars::RtfSettings* thresholds = 
      f_byseries_vars.GetRtfSettingsByDetCode(detCode);
    //only defined for phonon channels right now
    if(thresholds){
      fPulseRQList["threshlow"] = thresholds->GetPlo();
      fPulseRQList["trigmode"] = thresholds->GetPhononTrigMode();
    }
    fPulseRQList["offset"] = f_phononoffsets.GetOffset(detCode, time);
  }
}


const std::map<std::string, double>&
DatabaseManager::ConstructDetectorRQs(int detType, int detNum){
  double initVal = BatRootTypes::kEmptyVariable;
  std::map<std::string, double>& rqlist = fDetRQMap[detNum];
  rqlist.clear();
  if(detType == BatRootTypes::kCDMSliteSoudanI || 
     detType == BatRootTypes::kCDMSliteSoudanII ){
    rqlist["HVvolts"] = initVal;
    rqlist["HVnamps"] = initVal;
    rqlist["HVstatus"] = initVal;
  }
  return rqlist;
}


void DatabaseManager::StoreDetectorRQs(time_t time, int detType, int detNum)
{
  if(detType == BatRootTypes::kCDMSliteSoudanI || 
     detType == BatRootTypes::kCDMSliteSoudanII  ){
    IsegHV::HVinfo hv = f_iseghv.GetHVinfo(detNum, time);
    fDetRQMap[detNum]["HVvolts"] = hv.volts;
    fDetRQMap[detNum]["HVnamps"] = hv.namps;
    fDetRQMap[detNum]["HVstatus"] = hv.status;
  }
}
