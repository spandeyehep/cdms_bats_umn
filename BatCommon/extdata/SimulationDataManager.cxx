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

#include "SimulationDataManager.h"
#include <sstream>


////////////////////////////////////////////////////////

SimulationDataManager::SimulationDataManager()
{
	// hardcoding channel names from Soudan iZIP and
        // cdmslite... this is a stopgap measure that should
        // be made more general eventually [AJA]
	channelNamesiZIP.push_back("PAS1");
	channelNamesiZIP.push_back("PBS1");
	channelNamesiZIP.push_back("PCS1");
	channelNamesiZIP.push_back("PDS1");
	channelNamesiZIP.push_back("PAS2");
	channelNamesiZIP.push_back("PBS2");
	channelNamesiZIP.push_back("PCS2");
	channelNamesiZIP.push_back("PDS2");
	channelNamesiZIP.push_back("PS1");
	channelNamesiZIP.push_back("PS2");
	channelNamesiZIP.push_back("PT");
	channelNamesiZIP.push_back("QIS1");
	channelNamesiZIP.push_back("QOS1");
	channelNamesiZIP.push_back("QIS2");
	channelNamesiZIP.push_back("QOS2");

	channelNamesLite.push_back("PA");
	channelNamesLite.push_back("PB");
	channelNamesLite.push_back("PC");
	channelNamesLite.push_back("PD");
	channelNamesLite.push_back("PT");
	channelNamesLite.push_back("QI");
	channelNamesLite.push_back("QO");
}


SimulationDataManager::~SimulationDataManager() {}


void SimulationDataManager::setDataFilename(string fname)
{
	filename = fname;
}


void SimulationDataManager::openDataFile()
{
	// check that data file is declared
	if(filename.empty())
		cout << "ERROR in SimulationDataManager::openDataFile! Filename not set yet!" << endl;

	// open the file
	simDataFile.open(filename.c_str());

	// check that the file exists
	if(simDataFile.is_open() == false || simDataFile.peek() == EOF)
	{
	    cout << "ERROR in SimulationDataManager::openDataFile! File does not exist!: " << filename << endl;
	    exit(EXIT_FAILURE);
	}

	// skip lines starting with #
	while(simDataFile.peek() == '#')
		simDataFile.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );

	cout << "**** Opening pulse simulation input file: " << filename << endl;
}


void SimulationDataManager::readEvent()
{
    cout << "**** Reading event energies from simulation input file ****" << endl;

    // skip lines starting with #
    while(simDataFile.peek() == '#')
	simDataFile.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );
    
    // Loop over lines in this event.
    // Events are separated in file by a line containing '*'.
    while(simDataFile.peek() != '*')
    {
        // READ ONE LINE
	// assuming that input file has columns in particular order given in channelNames!!
	// read the detector number in 1st column
	//int detNum;
	//simDataFile >> detNum;
	
	//double energy;
	//int jChan = 0;
	//while(jChan < channelNames.size())
	string line;
	vector<double> lineValues;
	double colValue;
	getline(simDataFile, line);
	istringstream ss(line);
	while(ss >> colValue)
	{
	    //istringstream ss(line)
	    //simDataFile >> energy;

	    lineValues.push_back(colValue);

	    //std::pair<int, string> chanID(detNum, channelNames[jChan]);
	    //eventEnergies[chanID] = energy;

	    //jChan++;
	}

	//int tempLib;
	//simDataFile >> tempLib;
	int detNum = lineValues[0];
	libNum[detNum] = lineValues[lineValues.size()-1];
	vector<string> channelNames;
	if(lineValues.size() == 17)  // Soudan iZIP
	    channelNames = channelNamesiZIP;
	else if(lineValues.size() == 9)  // Soudan cdmslite
	    channelNames = channelNamesLite;

	eventEnergies[detNum] = std::map<string, double>();
	for(int jChan = 0; jChan < channelNames.size(); jChan++)
	{
	    std::pair<string, double> chanID(channelNames[jChan], lineValues[jChan+1]);
	    eventEnergies[detNum].insert(chanID);
	}
	
	// if we are at the end of the file, issue an error--last line should
	// be an asterisk
	if(simDataFile.peek() == EOF)
	{
	    cout << "ERROR!: Reached end of input file: " << filename << endl;
	    exit(EXIT_FAILURE);
	}

	// throw away the rest of the line
	//simDataFile.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );	
    }
    // throw away the rest of the line containing '*'
    simDataFile.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );
}


void SimulationDataManager::closeDataFile()
{
    simDataFile.close();
}


bool SimulationDataManager::isOpen()
{
    return simDataFile.is_open();
}


map<string, double> SimulationDataManager::getEventEnergies(int detNum)
{
    /*map<string, double> energyMap;
    
    if(eventEnergies.size() > 0)
    {
	for(int jchan = 0; jchan < channelNames.size(); jchan++)
	{	    
	    std::pair<int, string> chanID(detNum, channelNames[jchan]);
	    if(eventEnergies.count(chanID) > 0)
		energyMap[channelNames[jchan]] = eventEnergies[chanID];
	}	
	}*/

    return eventEnergies[detNum];
}


int SimulationDataManager::getLibNum(int detNum)
{
    int thisLibNum = libNum[detNum];
    return thisLibNum;
}
