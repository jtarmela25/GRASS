#include "ScaffoldComparer.h"
#include "Helpers.h"
#include <limits>

#include <iostream>

int ScaffoldComparer::Compare(const Scaffold &a, const Scaffold &b)
{
	Scaffold c(b);
	c.Reverse();
	//int p = compareOriented(a, b);
	//cout << "reverse" << endl;
	//int q = compareOriented(a, c);
	//return min(p, q);
	return min(compareOriented(a, b), compareOriented(a, c));
}

int ScaffoldComparer::Compare(const vector<Scaffold> &a, const vector<Scaffold> &b)
{
	int aSize = a.size(), bSize = b.size();
	int mismatch = 0;
	for (int i = 0; i < aSize; i++)
	{
		int minScore = INT_MAX;
		for (int j = 0; j < bSize; j++)
		{
			//int t = Compare(a[i], b[j]);
			//minScore = min(minScore, t);
			minScore = min(minScore, Compare(a[i], b[j]));
			//cout << "Distance between " << i << " and " << j << ": " << t << endl;
		}
		mismatch += minScore;
	}
	return mismatch;
}

int ScaffoldComparer::compareOriented(const Scaffold &a, const Scaffold &b)
{
	map<int, int> pos;
	int aSize = a.ContigCount(), bSize = b.ContigCount(), mismatch = 0;
	for (int i = 0; i < bSize; i++)
		pos[b[i].Id] = i;
	for (int i = 1; i < aSize; i++)
	{
		ScaffoldContig p = a[i - 1], q = a[i];
		if (pos.find(p.Id) == pos.end() || pos.find(q.Id) == pos.end())
		{
			mismatch++;
			//cout << "at " << i << ": " << p.Id << " " << q.Id << endl;
			continue;
		}
		if (pos[p.Id] > pos[q.Id])
		{
			mismatch++;
			//cout << "+at " << i << ": " << p.Id << " " << q.Id << endl;
			continue;
		}
		if ((b[pos[p.Id]].T ^ b[pos[q.Id]].T) != (p.T ^ q.T))
		{
			mismatch++;
			//cout << "-at " << i << ": " << p.Id << " " << q.Id << endl;
			continue;
		}
	}
	return mismatch;
}

int ScaffoldComparer::OrientationDistance(const Scaffold &a, const Scaffold &b)
{
	map<int, bool> orientation;
	int aSize = a.ContigCount(), bSize = b.ContigCount(), mismatchForward = 0, mismatchReverse = 0;
	for (int i = 0; i < bSize; i++)
		orientation[b[i].Id] = b[i].T;
	for (int i = 1; i < aSize; i++)
	{
		int id = a[i].Id;
		if (orientation.find(id) == orientation.end())
			mismatchForward++, mismatchReverse++;
		else if (a[i].T == orientation[id])
			mismatchReverse++;
		else
			mismatchForward++;
	}
	return min(mismatchForward, mismatchReverse);
}

int ScaffoldComparer::OrientationDistance(const vector<Scaffold> &a, const vector<Scaffold> &b)
{
	int aSize = a.size(), bSize = b.size();
	int mismatch = 0;
	for (int i = 0; i < aSize; i++)
	{
		int minScore = INT_MAX;
		for (int j = 0; j < bSize; j++)
			minScore = min(minScore, OrientationDistance(a[i], b[j]));
		mismatch += minScore;
	}
	return mismatch;
}