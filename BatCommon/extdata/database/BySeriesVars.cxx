#include "BySeriesVars.h"
#include "BatRootTypes.h" //defines kEmptyVariable
#include "TSQLRow.h"
#include "TSQLResult.h"

#include <sstream>

using CdmsDB::BySeriesVars;
typedef CdmsDB::BySeriesVars::RtfSettings RtfSettings;
using BatRootTypes::kEmptyVariable;

BySeriesVars::BySeriesVars() { Reset(); }

BySeriesVars::~BySeriesVars() {}

void BySeriesVars::Reset()
{
  f_series      = ""; 
  f_series_id   = kEmptyVariable; 
  f_series_mode = kEmptyVariable;
  f_start_time  = kEmptyVariable; 
  f_end_time    = kEmptyVariable; 
  f_nevents     = kEmptyVariable; 
  f_livetime    = kEmptyVariable;
  
  f_rtfmap.Reset();
}


std::string RtfSettings::GetQueryVars()
{
  return std::string("rtf_settings_by_series.rtf_num, ") + 
    "rtf_settings_by_series.qhi, rtf_settings_by_series.qlo, " +
    "rtf_settings_by_series.phi, rtf_settings_by_series.plo, " + 
    "rtf_settings_by_series.whisper, rtf_settings_by_series.dc_offset, "+
    "rtf_settings_by_series.phononTriggerMode, "+
    "rtf_settings_by_series.chargeTriggerMode ";
}
    

std::string BySeriesVars::GetQueryString(const std::string& series)
{
  std::stringstream query;
  query<<"SELECT series.series, series.id, series.mode, "
       <<"       series.start_time/1000, series.end_time/1000, "
       <<"       series.nevents, "
       <<"       livetime_by_series.livetime, "
       <<        RtfSettings::GetQueryVars()

       <<" FROM  series, livetime_by_series, "
       <<"       rtf_settings_by_series "
       <<" WHERE series.series = \"" << series <<"\"  AND "
       <<"       series.id = livetime_by_series.series_id AND "
       <<"       series.id = rtf_settings_by_series.series_id"
       <<" GROUP BY rtf_settings_by_series.rtf_num "
    ;
  return query.str();
}

RtfSettings::RtfSettings(TSQLRow* row, int firstfield)
{
  //no checks, so the row size must be verified before construction!
  f_rtf_num = row->GetField(firstfield) == 0 ? 
    kEmptyVariable : atoi(row->GetField(firstfield));
  f_qhi     = row->GetField(firstfield+1) == 0 ? 
    kEmptyVariable : atof(row->GetField(firstfield+1));
  f_qlo     = row->GetField(firstfield+2) == 0 ? 
    kEmptyVariable : atof(row->GetField(firstfield+2));
  f_phi     = row->GetField(firstfield+3) == 0 ? 
    kEmptyVariable : atof(row->GetField(firstfield+3));
  f_plo     = row->GetField(firstfield+4) == 0 ? 
    kEmptyVariable : atof(row->GetField(firstfield+4));
  f_whisper = row->GetField(firstfield+5) == 0 ? 
    kEmptyVariable : atof(row->GetField(firstfield+5));
  f_offset  = row->GetField(firstfield+6) == 0 ? 
    kEmptyVariable : atof(row->GetField(firstfield+6));
  f_phononTrigMode = row->GetField(firstfield+7) == 0 ? 
    kEmptyVariable : atoi(row->GetField(firstfield+7));
  f_chargeTrigMode = row->GetField(firstfield+8) == 0 ? 
    kEmptyVariable : atoi(row->GetField(firstfield+8));
  
}

int BySeriesVars::LoadDbResult(TSQLResult* result) 
{
  //result to default
  Reset();
  //make sure the result is well-formed
  const int nfields = 16;
  const int first_rtf_field = 7;
  
  if(!result || result->GetRowCount() < 1 ){
    std::cerr<<"BySeriesVars: ERROR: empty result passed to constructor!\n";
    return -1;
  }
  else if (result->GetFieldCount() != nfields){
    std::cerr<<"BySeriesVars: ERROR: Expected "<<nfields
	     <<" columns in query result, got "
	     <<result->GetFieldCount()<<"\n";
    return -2;
    
  }
  else{
    //the result should be ok to load
    //all fields except rtf stuff should be identical for each row, so only 
    // fill on first one
    while( TSQLRow* row = result->Next() ){
      if(f_series_id == kEmptyVariable){
	f_series      = row->GetField(0);
	f_series_id   = atoi(row->GetField(1));
	f_series_mode = atoi(row->GetField(2));
	f_start_time  = atoi(row->GetField(3));
	f_end_time    = atoi(row->GetField(4));
	f_nevents     = atoi(row->GetField(5));
	f_livetime    = atof(row->GetField(6));
      }
      //this will create by copy, but they're small so it's probably ok
      RtfSettings rtf(row,first_rtf_field);
      f_rtf_settings[rtf.GetRtfNum()] = rtf;
    }
  }
  return 0;
}

std::ostream& BySeriesVars::Print(std::ostream& out, bool printrtf) const 
{
  out <<"series:       "<<f_series<<"\n"
      <<"series id:    "<<f_series_id<<"\n"
      <<"mode:         "<<f_series_mode<<"\n"
      <<"start_time:   "<<f_start_time<<"\n"
      <<"end_time:     "<<f_end_time<<"\n"
      <<"nevents:      "<<f_nevents<<"\n"
      <<"livetime:     "<<f_livetime<<"\n"
    ;
  if(printrtf){
    out<<"rtf_settings: \n"
       <<"\trtfnum\tqhi\tqlo\tphi\tplo\twhisper\tdcoffset\n";
    std::map<int,RtfSettings>::const_iterator it = f_rtf_settings.begin();
    while(it != f_rtf_settings.end()){
      out<<"\t"<< (it->second) <<"\n";
      ++it;
    }
  }
  return out<<std::flush;

}

std::ostream& RtfSettings::Print(std::ostream& out) const
{
  return out<<f_rtf_num  <<"\t"
	    << f_qhi     << "\t"
	    << f_qlo     << "\t"
	    << f_phi     << "\t"
	    << f_plo     << "\t"
	    << f_whisper << "\t"
	    << f_offset;
}
