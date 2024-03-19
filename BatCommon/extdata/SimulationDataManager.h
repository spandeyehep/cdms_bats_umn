////////////////////////////////////////////////////////////////////////////////////
// Class Name: SimulationDataManager
// Authors: A. Anderson
// Description:  This class reads the text file containing the energy info for the
// pulse simulation.
//
// File Import By: A. Anderson
// Creation Date: 17 August 2013
//
////////////////////////////////////////////////////////////////////////////////////

#ifndef SIMULATIONDATAMANAGER_H
#define SIMULATIONDATAMANAGER_H

#include "ListManager.h"
#include <fstream>
#include <map>
#include <limits>

using namespace std;

class SimulationDataManager {
public:
	SimulationDataManager();
	~SimulationDataManager();

	void setDataFilename(string fname);		// set the filename
	void openDataFile();					// mainly to check the file formatting
	void readEvent();		// read one event form the datafile
	map<string, double> getEventEnergies(int detNum);	// get the event energies
	int getLibNum(int detNum);
	void closeDataFile();					// be sure to close the file
	bool isOpen();

private:
	string filename;
	ifstream simDataFile;

	//map<std::pair<int, string>, double> eventEnergies;
	map<int, std::map<string, double> > eventEnergies;
	map<int, int> libNum;
	vector<string> channelNamesiZIP;
	vector<string> channelNamesLite;
};


#endif /* SIMULATIONDATAMANAGER_H */
