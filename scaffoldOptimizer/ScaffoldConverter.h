/*
 * scaffoldOptimizer : solves the MIQP optimization and produces linear scaffold
 * sequences.
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

#ifndef _SCAFFOLDCONVERTER_H
#define _SCAFFOLDCONVERTER_H

#include "ScaffoldExtractor.h"
#include "DataStore.h"
#include "Sequence.h"
#include "OverlapperConfiguration.h"
#include <vector>

using namespace std;

class ScaffoldConverter
{
public:
	static vector<FastASequence> ToFasta(const DataStore &store, const Scaffold &scaffold, const OverlapperConfiguration &config);
	static vector<FastASequence> ToFasta(const DataStore &store, const vector<Scaffold> &scaffold, const OverlapperConfiguration &config);
};

#endif
