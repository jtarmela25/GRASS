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
#include "MummerCoord.h"

using namespace std;

class MummerCoordReader
{
public:
    MummerCoordReader();
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
};

#endif	/* _MUMMERCOORDREADER_H */
