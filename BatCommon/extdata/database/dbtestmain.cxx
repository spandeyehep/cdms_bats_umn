#include "DatabaseManager.h"
#include "BySeriesVars.h"
#include <iostream>

using namespace std;

int main(int argc, const char** argv)
{
  if(argc < 2){
    cerr<<"Usage: "<<argv[0]<<" series [series2, ...]"<<endl;
    return -1;
  }
  
  CdmsDB::DatabaseManager dbman;
  for(int i=1; i<argc; ++i){
    
    dbman.LoadSeries(argv[i]);
    const CdmsDB::BySeriesVars& series = dbman.GetBySeriesVars();
    
    cout<<CdmsDB::FridgeData::GetQueryString(series.GetSeriesID())<<endl;

    cout<<"Info for series "<<argv[i]<<": \n"
	<<series<<endl;

    const CdmsDB::FridgeData& fd = dbman.GetFridgeData();
    cout<<"10 sample base temp points:\n";
    for(time_t t = series.GetStartTime(); t < series.GetEndTime();
	t+= (series.GetEndTime() - series.GetStartTime())/10){
      time_t realtime = t;
      double temp = fd.GetField(CdmsDB::FridgeData::kBaseTemp, realtime);
      cout<<"\t"<<realtime<<"\t"<<temp<<endl;
    }
    
    const CdmsDB::PhononOffsets& po = dbman.GetPhononOffsets();
    cout<<"10 sample phonon offset points for 11013008:\n";
    for(time_t t = series.GetStartTime(); t < series.GetEndTime();
	t+= (series.GetEndTime() - series.GetStartTime())/10){
      time_t realtime = t;
      double off = po.GetOffset(11013008, realtime);
      cout<<"\t"<<realtime<<"\t"<<off<<endl;
    }
    

    
  }
  return 0;
}
