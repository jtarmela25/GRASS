/*
 * dataSimulator : a support tool used in debugging to create simulated scaffolding
 * problems given a reference sequence and paired reads.
 * Copyright (C) 2011  Alexey Gritsenko
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 * 
 * 
 * 
 * Email: a.gritsenko@tudelft.nl
 * Mail: Delft University of Technology
 *       Faculty of Electrical Engineering, Mathematics, and Computer Science
 *       Department of Mediamatics
 *       P.O. Box 5031
 *       2600 GA, Delft, The Netherlands
 */

#ifndef _CONTIGINFORMATION_H
#define _CONTIGINFORMATION_H
#include <string>

using namespace std;

class ContigInformation
{
public:
	ContigInformation(int position = 0, bool reverseOrientation = false, int sourceContig = 0) : Position(position), ReverseOrientation(reverseOrientation), SourceContig(sourceContig) {};

public:
	string FormatName() const;

public:
	int Position;
	bool ReverseOrientation;
	int SourceContig;
};
#endif
