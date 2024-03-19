#include "DetectorRtfMap.h"
#include "BatRootTypes.h"

#include "TSQLResult.h"
#include "TSQLRow.h"

#include <sstream>
#include <iostream>

using CdmsDB::DetectorRtfMap;

void DetectorRtfMap::Reset()
{ f_rtfmap.clear(); }

std::string DetectorRtfMap::GetQueryString(int series_id)
{
  std::stringstream query;
  query<<"SELECT rtf_num, "
       <<"       detector_codeA, detector_codeB, detector_codeC, "
       <<"       detector_codeD, detector_codeQO, detector_codeQI " 
       <<"FROM detector_rtf_mapping " 
       <<"WHERE series_id = "<<series_id;
  return query.str();
}

int DetectorRtfMap::LoadDbResult(TSQLResult* result)
{
  Reset();
  if(!result || result->GetRowCount() < 1){
    std::cerr<<"DetectorRtfMap: ERROR: Empty mysql result passed!\n";
    return -1;
  }
  const int nfields = 7;
  if(result->GetFieldCount() != nfields){
    std::cerr<<"DetectorRtfMap: ERROR: Expected "<<nfields
	     <<" columns in result, got "<<result->GetFieldCount()
	     <<"!\n";
    return -2;
  }
  
  //loop through each row in the result and fill the rtf map
  while(TSQLRow* row = result->Next()){
    for(int i=1; i<nfields; ++i)
      f_rtfmap[ atoi(row->GetField(i)) ] = atoi(row->GetField(0));
  }
  
  return 0;
}

int DetectorRtfMap::GetRtfNum(uint32_t detCode) const
{
  std::map<uint32_t,int>::const_iterator res = f_rtfmap.find(detCode);
  if(res == f_rtfmap.end())
    return BatRootTypes::kEmptyVariable;
  return res->second;
}
