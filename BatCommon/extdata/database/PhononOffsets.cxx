#include "PhononOffsets.h"
#include "BatRootTypes.h" 

#include "TSQLResult.h"
#include "TSQLRow.h"
#include <sstream>
#include <iostream>

using CdmsDB::PhononOffsets;


PhononOffsets::PhononOffsets(){ Reset(); }
PhononOffsets::~PhononOffsets(){}

void PhononOffsets::Reset()
{
  f_offsetmap.clear();
}

std::string PhononOffsets::GetQueryString(int series_id)
{
  std::stringstream query;
  query<<"SELECT time, detector_codeA, offsetA, detector_codeB, offsetB, "
       <<"             detector_codeC, offsetC, detector_codeD, offsetD "
       <<" FROM phonon_offsets JOIN detector_rtf_mapping "
       <<"   ON detector_rtf_mapping.rtf_num = phonon_offsets.rtf_num "
       <<" WHERE detector_rtf_mapping.series_id = "<<series_id
       <<"   AND phonon_offsets.time BETWEEN "
       <<"       (SELECT start_time/1000 from series where id="<<series_id<<") "
       <<"     AND (SELECT end_time/1000 from series where id="<<series_id<<") "
    //<<" ORDER BY phonon_offsets.id ASC "
    ;
  return query.str();
}

int PhononOffsets::LoadDbResult(TSQLResult* result)
{
  Reset();
  //make sure the result is well-formed
  const int nfields = 9;
  if(!result || result->GetRowCount() < 1 ){
    std::cerr<<"PhononOffsets: ERROR: empty result passed to constructor!\n";
    return -1;
  }
  else if (result->GetFieldCount() != nfields){
    std::cerr<<"PhononOffsets: ERROR: Expected "<<nfields
	     <<" columns in query result, got "
	     <<result->GetFieldCount()<<"\n";
    return -2;
    
  }
  
  //if we get here, we can fill
  while(TSQLRow* row = result->Next()){
    //each row has timestamp, then 8 cols of detCode+offset{A,B,C,D}
    time_type t = atoi(row->GetField(0));
    for(int ch=0; ch<4; ++ch){
      uint32_t detCode = strtoul( row->GetField(1 + 2*ch) ,0,0);
      double offset = atof( row->GetField(1 + 2*ch + 1) );
      f_offsetmap[detCode][t] = offset;
    }
  }
  
  return 0;
}

double PhononOffsets::GetOffset(uint32_t detCode, time_type& time) const
{
  std::map<uint32_t, std::map<time_type, double> >::const_iterator itDet;
  itDet = f_offsetmap.find(detCode);
  if(itDet == f_offsetmap.end()){
    std::cerr<<"PhononOffsets: No offset data for detector "<<detCode<<"\n";
    return BatRootTypes::kEmptyVariable;
  }
  
  const std::map<time_type, double>& submap = itDet->second;
  if(submap.empty()){
    std::cerr<<"PhononOffsets: No offset data for detector "<<detCode<<"\n";
    return BatRootTypes::kEmptyVariable;
  }
  
  std::map<time_type, double>::const_iterator itOff = submap.lower_bound(time);
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
    std::cerr<<"PhononOffsets:WARNING: Unable to find offset for channel "
	     <<detCode<<" within "<<MAX_TIME_DIFF<<" of "<<time<<"!\n";
  }
  
  //update the time variable to the actual one used
  time = itOff->first;
  return itOff->second;
  
}
