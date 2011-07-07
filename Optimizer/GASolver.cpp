#include "GASolver.h"
#include "Helpers.h"
#include "RandomizedGreedyInitializer.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <set>
#include "omp.h"

using namespace std;

GASolver::GASolver()
{
	SelectionSize = 40;
	CrossoverRate = 0.5;
	RestartGenerations = 30;
	LocalSearchM = 50;
	status = Clean;
}

GASolver::~GASolver()
{
}

bool GASolver::Formulate(const DataStore &store)
{
	if (status != Clean)
		return false;
	ContigCount = store.ContigCount;
	U.assign(ContigCount, true);
	T.resize(ContigCount);
	X.resize(ContigCount); // compability
	formulateMatrix(store);
	Options.SuppressOutput = true;
	status = Formulated;
	return true;
}

bool GASolver::Solve()
{
	if (status < Formulated)
		return false;

	bestObjective = -Helpers::Inf;
	populationSize = 0;
	timerId = Helpers::ElapsedTimers.AddTimer();
	iteration = 0;
	restartCount = 0;
	lastSuccess = 0;
	double lastTime = 0;
	
	omp_set_num_threads(Options.Threads);
	localSearch(generatePopulation());
	if (Options.VerboseOutput)
		printf("[i] Generated population: %.2lf ms\n", getTime(lastTime));
	selectInitialSolution();
	while (!shouldTerminate())
	{
		localSearch(crossover());
		select();
		if (Options.VerboseOutput)
			printf("[i] Iteration %i: %.2lf ms\n", iteration + 1, getTime(lastTime));
		if (iteration - lastSuccess >= RestartGenerations)
		{
			restartCount++;
			lastSuccess = iteration;
			localSearch(restart(1));
			localSearch(generatePopulation(populationSize));
			if (Options.VerboseOutput)
			{
				if (Options.GARestarts > 0)
					printf("   [i] Restarted: attempt %i out of %i\n", restartCount, Options.GARestarts);
				else
					printf("   [i] Restarted: attempt %i\n", restartCount);
			}
		}
		iteration++;
	}
	Helpers::ElapsedTimers.RemoveTimer(timerId);
	if (status != Fail)
		status = Success;
	return status == Success;
}

SolverStatus GASolver::GetStatus() const
{
	return status;
}

double GASolver::GetObjective() const
{
	if (status == Success)
		return bestObjective;
	return -Helpers::Inf;
}

bool GASolver::shouldTerminate()
{
	if (Options.TimeLimit > 0 && Helpers::ElapsedTimers.Elapsed(timerId) > (double)Options.TimeLimit * 1000)
		return true;
	if (Options.GARestarts > 0 && restartCount >= Options.GARestarts)
		return true;
	return false;
}

int GASolver::generatePopulation(int from)
{
	if (populationSize < SelectionSize)
		population.resize(SelectionSize), populationSize = SelectionSize;
	#pragma omp parallel
	{
		srand(time(NULL) ^ omp_get_thread_num());
		#pragma omp for
		for (int i = from; i < SelectionSize; i++)
		{
			RandomizedGreedyInitializer init(ContigCount, matrix);
			population[i] = init.MakeSolution(matrix);
		}
	}
	return from;
}

int GASolver::localSearch(int from)
{
	#pragma omp parallel
	{
		srand(time(NULL) ^ omp_get_thread_num());
		#pragma omp for
		for (int i = from; i < populationSize; i++)
			RandomizedKopt(population[i]);
	}
	return from;
}

int GASolver::crossover()
{
	int count = (int)(CrossoverRate * SelectionSize);
	int newSize = populationSize + count;
	population.resize(newSize);
	#pragma omp parallel
	{
		srand(time(NULL) ^ omp_get_thread_num());
		#pragma omp for
		for (int i = 0; i < count; i++)
		{
			int a = rand() % populationSize, b = rand() % populationSize;
			population[populationSize + i] = InnovativeCrossover(population[a], population[b]);
		}
	}
	populationSize = newSize;
	return newSize - count;
}

void GASolver::select()
{
	int i = 1, j = 1;
	sort(population.begin(), population.end(), greater<GAIndividual>());
	for (; i < populationSize && j < SelectionSize; i++)
	{
		if (population[i] != population[j - 1])
			population[j++] = population[i];
	}
	populationSize = j;
	population.resize(populationSize);
	updateSolution(population[0]);
}

int GASolver::restart(int from)
{
	#pragma omp parallel
	{
		srand(time(NULL) ^ omp_get_thread_num());
		#pragma omp for
		for (int i = from; i < populationSize; i++)
			Mutate(population[i]);
	}
	return from;
}

void GASolver::formulateMatrix(const DataStore &store)
{
	matrix = GAMatrix(ContigCount + 1);
	for (DataStore::LinkMap::const_iterator it = store.Begin(); it != store.End(); it++)
	{
		int i = it->first.first, j = it->first.second;
		double w = it->second.Weight;
		if (it->second.EqualOrientation)
		{
			matrix[i][i] -= w;
			matrix[j][j] -= w;
			matrix[i][j] += 2 * w;
			matrix[ContigCount][ContigCount] += w; // constant summand
		}
		else
		{
			matrix[i][i] += w;
			matrix[j][j] += w;
			matrix[i][j] -= 2 * w;
		}
	}
	for (int i = 0; i < ContigCount; i++)
		for (int j = i + 1; j < ContigCount; j++)
			matrix[i][j] = matrix[j][i] = (matrix[i][j] + matrix[j][i]) / 2.0;
	matrix.CalculatePositions();
}

void GASolver::selectInitialSolution()
{
	double best = population[0].GetObjective();
	int bestInd = 0;
	for (int i = 1; i < populationSize; i++)
		if (best < population[i].GetObjective())
		{
			best = population[i].GetObjective();
			bestInd = i;
		}
	updateSolution(population[bestInd]);
}

void GASolver::updateSolution(const GAIndividual &ind)
{
	double objective = ind.GetObjective();
	if (objective > bestObjective + Helpers::Eps)
	{
		if (Options.VerboseOutput)
			cout << "   [+] Best found: " << objective << endl;
		bestObjective = objective;
		for (int i = 0; i < ContigCount; i++)
			T[i] = ind.X[i];
		lastSuccess = iteration;
	}
}

double GASolver::getTime(double &lastIteration) const
{
	double last = lastIteration;
	lastIteration = Helpers::ElapsedTimers.Elapsed(timerId);
	return lastIteration - last;
}

GAIndividual GASolver::InnovativeCrossover(const GAIndividual &p1, const GAIndividual &p2)
{
	GAIndividual offspring(p1);
	int eqCnt = 0, neqCnt = 0;
	vector<int> eq, neq;
	for (int i = 0; i < ContigCount; i++)
		if (p1.X[i] == p2.X[i])
			eq.push_back(i), eqCnt++;
		else
			neq.push_back(i), neqCnt++;
	for (int i = neqCnt; i > 0; i--)
	{
		random_shuffle(neq.begin(), neq.end());
		int p = -1;
		for (int j = 0; j < neqCnt; j++)
			if (offspring.Gain[neq[j]] > Helpers::Eps)
			{
				p = j;
				break;
			}
		if (p >= 0)
		{
			offspring.Flip(neq[p], matrix);
			swap(neq[p], neq[neqCnt - 1]), neqCnt--, neq.resize(neqCnt);
		}
		if (eqCnt > 0)
		{
			int p = -1;
			for (int j = 0; j < eqCnt; j++)
				if (p < 0 || offspring.Gain[eq[p]] < offspring.Gain[eq[j]])
					p = j;
			offspring.Flip(eq[p], matrix);
			swap(eq[p], eq[eqCnt - 1]), eqCnt--, eq.resize(eqCnt);
		}
	}
	return offspring;
}

void GASolver::RandomizedKopt(GAIndividual &ind)
{
	vector<int> perm(ContigCount);
	for (int i = 0; i < ContigCount; i++)
		perm[i] = i;
	vector<bool> used(ContigCount);
	while (true)
	{
		GAIndividual xprev(ind), xbest(ind);
		used.assign(ContigCount, false);
		double gBest = 0, g = 0;
		int unused = ContigCount, lastBest = 0;
		do
		{
			lastBest++;
			random_shuffle(perm.begin(), perm.end());
			for (int i = 0; i < ContigCount; i++)
				if (ind.Gain[perm[i]] > Helpers::Eps)
				{
					g += ind.Gain[perm[i]];
					ind.Flip(perm[i], matrix);
					used[perm[i]] = true;
					unused--;
					if (g > gBest)
					{
						gBest = g;
						xbest = ind;
						lastBest = 0;
					}
				}
			if (unused > 0)
			{
				int p = -1;
				for (int i = 0; i < ContigCount; i++)
					if (!used[i] && (p < 0 || ind.Gain[p] < ind.Gain[i]))
						p = i;
				g += ind.Gain[p];
				ind.Flip(p, matrix);
				used[p] = true;
				unused--;
				if (g > gBest)
				{
					gBest = g;
					xbest = ind;
					lastBest = 0;
				}
			}
		} while (unused > 0 && lastBest >= LocalSearchM);
		if (gBest > Helpers::Eps)
			ind = xbest;
		else
		{
			ind = xprev;
			break;
		}
	}
}

void GASolver::Mutate(GAIndividual &ind)
{
	int vars = ContigCount / 3;
	vector<int> perm(ContigCount);
	for (int i = 0; i < ContigCount; i++)
		perm[i] = i;
	random_shuffle(perm.begin(), perm.end());
	for (int i = 0; i < vars; i++)
		ind.Flip(perm[i], matrix);
}

/*bool GASolver::checkObjective(GAIndividual &ind)
{
	vector<bool> t(ContigCount);
	for (int i = 0; i < ContigCount; i++)
		t[i] = ind.X[i];
	GAIndividual check(t, matrix);
	if (fabs(check.GetObjective() - ind.GetObjective()) > Helpers::Eps)
		return false;
	return true;
}

bool GASolver::checkGains(GAIndividual &ind)
{
	vector<bool> t(ContigCount);
	for (int j = 0; j < ContigCount; j++)
		t[j] = ind.X[j];
	GAIndividual check(t, matrix);
	double objective = check.GetObjective();
	for (int i = 0; i < ContigCount; i++)
	{
		for (int j = 0; j < ContigCount; j++)
			t[j] = ind.X[j];
		t[i] = !t[i];
		GAIndividual inc(t, matrix);
		double gain = inc.GetObjective() - objective;
		if (fabs(gain - ind.Gain[i]) > Helpers::Eps)
		{
			cout << "Error at " << i << endl;
			cout << ind.Gain[i] << " vs. " << gain << endl;
			return false;
		}
	}
	return true;
}*/
