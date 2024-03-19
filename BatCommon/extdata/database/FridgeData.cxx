#include "FridgeData.h"
#include "BatRootTypes.h"

#include "TSQLResult.h"
#include "TSQLRow.h"

#include <iostream>
#include <sstream>

using CdmsDB::FridgeData;
//using CdmsDB::FridgeData::time_type;

void FridgeData::Reset()
{
  f_dbrows.clear();
}

std::string FridgeData::GetQueryString(int series_id)
{
  const char* varnames[N_FRIDGE_VARS] = { "lr700_temp4", "TE_FEB_RACK" };

  std::stringstream query;
  query<<"SELECT UNIX_TIMESTAMP(fridgeTime) ";
  for(size_t i=0; i<N_FRIDGE_VARS; ++i)
    query<<", "<<varnames[i]<<" ";
  query<<"FROM fridge_data "
       <<"WHERE fridgeTime BETWEEN "
       <<" (SELECT FROM_UNIXTIME(start_time/1000) FROM series WHERE id="
       <<series_id<<") AND "
       <<" (SELECT FROM_UNIXTIME(end_time/1000) FROM series WHERE id="
       <<series_id<<") "
       <<"ORDER BY id ASC";
  return query.str();
}

int FridgeData::LoadDbResult(TSQLResult* result)
{
  Reset();
  //make sure the result is well-formed
  const int nfields = N_FRIDGE_VARS + 1;
  if(!result || result->GetRowCount() < 1 ){
    std::cerr<<"FridgeData: ERROR: empty result passed to constructor!\n";
    return -1;
  }
  else if (result->GetFieldCount() != nfields){
    std::cerr<<"FridgeData: ERROR: Expected "<<nfields
	     <<" columns in query result, got "
	     <<result->GetFieldCount()<<"\n";
    return -2;
  }
  
  while(TSQLRow* row = result->Next()){
    //convert the first entry to a timestamp, all others to double
    time_type t = atoi(row->GetField(0));
    std::vector<double>& dbrow = f_dbrows[t];
    for(size_t i=0; i<N_FRIDGE_VARS; ++i){
      dbrow.push_back( atof(row->GetField(i+1)) );
    }
  }
  
  return 0;
}

const std::vector<double>* FridgeData::GetRow(time_type& time) const
{
  if(f_dbrows.empty()){
    std::cerr<<"FridgeData: ERROR: No data loaded to read!\n";
    return 0;
  }
  std::map<time_type,std::vector<double> >::const_iterator it;
  it = f_dbrows.lower_bound(time);
  //lower_bound gives the row with t >= time
  //see if the one before is actually a better fit
  if(it->first != time && it != f_dbrows.begin()){
    time_type diff1 = time-it->first;
    it--;
    time_type diff2 = it->first-time;
    if(diff1 < diff2)
      it++;
  }
  time = it->first;
  return &(it->second);
}

double FridgeData::GetField(FridgeVars var, time_type& time) const
{
  const std::vector<double>* row = GetRow(time);
  if(!row)
    return BatRootTypes::kEmptyVariable;
  return row->at(var);
}
