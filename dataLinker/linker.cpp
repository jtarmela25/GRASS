#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include "Configuration.h"
#include "DataStore.h"
#include "PairedReadConverter.h"
#include "DataStoreWriter.h"

using namespace std;

Configuration config;
DataStore store;

bool processPairs(const Configuration &config, DataStore &store, const vector<PairedInput> &paired)
{
	PairedReadConverter converter(store);
	int n = paired.size();
	for (int i = 0; i < n; i++)
	{
		PairedInput p = paired[i];
		cerr << "   [i] Processing " << (p.IsIllumina ? "Illumina" : "454") << " paired reads (" << p.LeftFileName << ", " << p.RightFileName << ") of weight " << p.Weight << " with insert size " << p.Mean << " +/- " << p.Std << endl; 
		switch (converter.Process(config, paired[i]))
		{
		case PairedReadConverter::Success:
			cerr << "      [+] Successfully processed paired reads." << endl;
			break;
		case PairedReadConverter::FailedLeftAlignment:
			cerr << "      [-] Unable to align left read mates (" << (p.IsIllumina ? "Illumina" : "454") << ")." << endl;
			return false;
		case PairedReadConverter::FailedRightAlignment:
			cerr << "      [-] Unable to align second read mates (" << (p.IsIllumina ? "Illumina" : "454") << ")." << endl;
			return false;
		case PairedReadConverter::FailedLeftConversion:
			cerr << "      [-] Unable to convert left alignment SAM to BAM (SAM Tools)." << endl;
			return false;
		case PairedReadConverter::FailedRightConversion:
			cerr << "      [-] Unable to convert right alignment SAM to BAM (SAM Tools)." << endl;
			return false;
		case PairedReadConverter::FailedLinkCreation:
			cerr << "      [-] Unable to create contig links from alignment." << endl;
			return false;
		}
	}
	return true;
}

bool writeStore(const DataStore &store, const string &fileName)
{
	DataStoreWriter writer;
	bool result = writer.Open(fileName) && writer.Write(store);
	writer.Close();
	return result;
}

int main(int argc, char *argv[])
{
	srand((unsigned int)time(NULL));
	if (config.ProcessCommandLine(argc, argv))
	{
		if (!store.ReadContigs(config.InputFileName))
		{
			cerr << "[-] Unable to read contigs from file (" << config.InputFileName << ")." << endl;
			return -2;
		}
		cerr << "[+] Read input contigs from file (" << config.InputFileName << ")." << endl;
		cerr << "[i] Processing paired reads." << endl;
		if (!processPairs(config, store, config.PairedReadInputs))
			return -3;
		if (!writeStore(store, config.OutputFileName))
		{
			cerr << "[-] Unable to output generated links into file (" << config.OutputFileName << ")." << endl;
			return -4;
		}
		cerr << "[+] Output generated link into file (" << config.OutputFileName << ")." << endl;
		return 0;
	}
	cerr << config.LastError;
	return -1;
}