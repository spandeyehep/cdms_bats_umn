#include "IsegHV.h"
#include "BatRootTypes.h" 

#include "TSQLResult.h"
#include "TSQLRow.h"
#include <sstream>
#include <iostream>

using CdmsDB::IsegHV;

IsegHV::IsegHV(){ Reset(); }
IsegHV::~IsegHV(){}

void IsegHV::Reset()
{
  f_hvmap.clear();
}

std::string IsegHV::GetQueryString(int series_id)
{
  std::stringstream query;
  query<<"SELECT lvTime-2082844800 as t, "
       <<"  (SELECT detector_a FROM iseg_hv_mapping WHERE "
       <<"   FROM_UNIXTIME(t) BETWEEN start_time and IFNULL(end_time,NOW())) "
       <<"       AS detector_a, "
       <<"       voltage_a, current_a, status_a, "
       <<"  (SELECT detector_b FROM iseg_hv_mapping WHERE "
       <<"   FROM_UNIXTIME(t) BETWEEN start_time and IFNULL(end_time,NOW())) "
       <<"       AS detector_b, "
       <<"       voltage_b, current_b, status_b "
       <<" FROM iseg_hv "
       <<" WHERE lvTime-2082844800 BETWEEN "
       <<"    (SELECT start_time/1000 from series where id="<<series_id<<") "
       <<"    AND (SELECT end_time/1000 from series where id="<<series_id<<") "
       <<" ORDER BY t ASC ";
  return query.str();
}

int IsegHV::LoadDbResult(TSQLResult* result)
{
  Reset();
  //make sure the result is well-formed
  const int nfields = 9;
  if(!result || result->GetRowCount() < 1 ){
    std::cerr<<"IsegHV: ERROR: empty result passed to constructor!\n";
    return -1;
  }
  else if (result->GetFieldCount() != nfields){
    std::cerr<<"IsegHV: ERROR: Expected "<<nfields
	     <<" columns in query result, got "
	     <<result->GetFieldCount()<<"\n";
    return -2;
    
  }
  
  //if we get here, we can fill
  while(TSQLRow* row = result->Next()){
    //each row has timestamp, then 4 cols of detNum, volts, namps, status a+b
    time_type t = atoi(row->GetField(0));
    for(int ch=0; ch<2; ++ch){
      const char* detStr = row->GetField(1+4*ch);
      if(!detStr || !strcmp(detStr,""))
	continue;
      int detNum = atoi(detStr);
      HVinfo hv;
      hv.volts = atof(row->GetField(1+4*ch+1));
      hv.namps = atof(row->GetField(1+4*ch+2));
      hv.status = atoi(row->GetField(1+4*ch+3));
      f_hvmap[detNum][t] = hv;
    }
  }
  
  return 0;
}

IsegHV::HVinfo IsegHV::GetHVinfo(int detNum, time_type& time) const
{
  std::map<int, std::map<time_type, HVinfo> >::const_iterator itDet;
  itDet = f_hvmap.find(detNum);
  if(itDet == f_hvmap.end()){
    std::cerr<<"IsegHV: No offset data for detector "<<detNum<<"\n";
    return HVinfo();
  }
  const std::map<time_type, HVinfo>& submap = itDet->second;
  if(submap.empty()){
     std::cerr<<"IsegHV: No offset data for detector "<<detNum<<"\n";
     return HVinfo();
  }
  std::map<time_type, HVinfo>::const_iterator itOff = submap.lower_bound(time);
  //lower_bound returns an iterator to the first element >= time
  //do some sanity checks
  if(itOff == submap.end()){
    //we asked for a timestamp after the last one present, so go back one
    itOff--;
  }
  else if(itOff->first > time && itOff != submap.begin()){
    //we didn't get an exact match, so see which is closest, this or other
    time_type diff1 = itOff->first - time;
    itOff--;
    //by definition, the one before MUST be < time
    time_type diff2 = time - itOff->first;
    if(diff1 < diff2) itOff++;
  }
  const time_type MAX_TIME_DIFF = 500; //completely arbitrary!
  if( std::abs(itOff->first - time) > MAX_TIME_DIFF){
    std::cerr<<"IsegHV:WARNING: Unable to find offset for channel "
	     <<detNum<<" within "<<MAX_TIME_DIFF<<" of "<<time<<"!\n";
  }
  
  //update the time variable to the actual one used
  time = itOff->first;
  return itOff->second;
  
}

