/* 
 * File:   MummerCoordReader.h
 * Author: alexeyg
 *
 * Created on December 6, 2011, 9:48 AM
 */

#ifndef _MUMMERCOORDREADER_H
#define	_MUMMERCOORDREADER_H

#include <vector>
#include <cstdio>
#include <string>
#include <map>
#include "MummerCoord.h"
#include "Sequence.h"

using namespace std;

class MummerCoordReader
{
public:
    MummerCoordReader(const vector<FastASequence> &references, const vector<FastASequence> &scaffolds);
    ~MummerCoordReader();
    
public:
    bool Open(const string &fileName, const string &mode = "rb");
    bool Close();
    bool IsOpen() const;
    bool Read(MummerCoord &coord);
    long long Read(vector<MummerCoord> &coords);
    long long NumCoords();
    
private:
    FILE *fin;
    char *line;
    long long numCoords;
    
private:
    map<string, int> referenceIds;
    map<string, int> scaffoldIds;
    
private:
    void createMap(map<string, int> &store, const vector<FastASequence> &seq);
};

#endif	/* _MUMMERCOORDREADER_H */
