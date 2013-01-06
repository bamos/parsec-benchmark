//------------------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                           
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
//  file :	ParticleFilterPthread.h
//  author :	Christian Bienia - cbienia@cs.princeton.edu
//				Scott Ettinger - scott.m.ettinger@intel.com
//
//  description : Posix threads parallelized version of the particle filter
//					object derived from ParticleFilter.h
//		
//  modified : 
//--------------------------------------------------------------------------


#ifndef PARTICLEFILTERPTHREAD_H
#define PARTICLEFILTERPTHREAD_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "ParticleFilter.h"
#include "WorkPoolPthread.h"
#include "threads/WorkerGroup.h"
#include "threads/TicketDispenser.h"
#include "threads/Barrier.h"

#undef min

//Generic particle filter class templated on model object
template<class T> 
class ParticleFilterPthread: public ParticleFilter<T>, public threads::Threadable {

	using ParticleFilter<T>:: mModel;
	using ParticleFilter<T>:: mWeights;
	using ParticleFilter<T>:: mParticles;
	using ParticleFilter<T>:: mNewParticles;
	using ParticleFilter<T>:: mBestParticle;
	using ParticleFilter<T>:: mNParticles;
	using ParticleFilter<T>:: mMinParticles;
	using ParticleFilter<T>:: mBins;
	using ParticleFilter<T>:: mRnd;
	typedef typename ParticleFilter<T>::fpType fpType;
	typedef typename ParticleFilter<T>::Vectorf Vectorf;


public:
	//constructor
	ParticleFilterPthread(WorkPoolPthread &);
	//destructor
	~ParticleFilterPthread();

	//entry function for worker threads
	void Exec(threads::thread_cmd_t, threads::thread_rank_t);

protected:
	inline bool CalcWeight(std::vector<Vectorf> &particles, int i, int rank);   //calculate weight of one particle
	virtual void CalcWeights(std::vector<Vectorf> &particles);                  //calculate particle weights based on model likelihood

	//threaded version of the base class
	void GenerateNewParticles(int k);
	
	std::vector<int> mIndex;																//list of particles to regenerate


private:
	WorkPoolPthread &workers;
	threads::Barrier *workInit;
	threads::TicketDispenser<int> particleTickets;
	
	//granularity of work unit for dynamic load balancing
	const int WORKUNIT_SIZE_PARTICLEWEIGHTS;
	const int WORKUNIT_SIZE_NEWPARTICLES;

	//work to do
	std::vector<Vectorf> *particles;
	std::vector<unsigned char> *valid;
	int annealing_parameter;
};

//constructor
template<class T>
ParticleFilterPthread<T>::ParticleFilterPthread(WorkPoolPthread &_workers) : workers(_workers), WORKUNIT_SIZE_PARTICLEWEIGHTS(4), WORKUNIT_SIZE_NEWPARTICLES(32)
{
	workInit = new threads::Barrier(workers.Size());
}

//constructor
template<class T>
ParticleFilterPthread<T>::~ParticleFilterPthread()
{
	delete workInit;
}

//thread entry function
template<class T>
void ParticleFilterPthread<T>::Exec(threads::thread_cmd_t cmd, threads::thread_rank_t rank)
{
	int ticket,i;
	
	if (cmd == workers.THREADS_CMD_PARTICLEWEIGHTS) {
		//reset dispenser, set new increment to work unit size
		if(rank == 0) {
			particleTickets.resetDispenser(WORKUNIT_SIZE_PARTICLEWEIGHTS);
		}
		workInit->Wait();
	
		ticket = particleTickets.getTicket();
		while(ticket < (int)(particles->size())) {
			//process all elements in work unit
			for(i = ticket; i < (int)(particles->size()) && i < ticket + WORKUNIT_SIZE_PARTICLEWEIGHTS; i++) {
				(*valid)[i] = CalcWeight(*particles, i, rank);
			}
			ticket = particleTickets.getTicket();
		}
	} else if(cmd == workers.THREADS_CMD_NEWPARTICLES) {
		//reset dispenser, set new increment to work unit size
		if(rank == 0) {
			particleTickets.resetDispenser(WORKUNIT_SIZE_NEWPARTICLES);
		}
		workInit->Wait();
	
		ticket = particleTickets.getTicket();
		//distribute new particles randomly according to model stdDevs
		while(ticket < mNParticles) {
			//process all elements in work unit
			for(i = ticket; i < mNParticles && i < ticket + WORKUNIT_SIZE_NEWPARTICLES; i++) {
				//add new particle for each entry in each bin distributed randomly about duplicated particle
				mNewParticles[i] = mParticles[mIndex[i]];
				AddGaussianNoise(mNewParticles[i], mModel->StdDevs()[annealing_parameter], mRnd[i]);
			}
			ticket = particleTickets.getTicket();
		}
	} else {
		//unknown command
		throw -1;
	}
}


//helper function for CalcWeights.  Calculates the weight (mWeights) for one particle
template<class T>
bool ParticleFilterPthread<T>::CalcWeight(std::vector<Vectorf> &particles, int i, int rank)
{	bool valid;
	mWeights[i] = mModel->LogLikelihood(particles[i], valid, rank);                   //compute log-likelihood weights for each particle
	return valid;
}

//calculate particle weights (mWeights) and find highest likelihood particle. 
//computes an optimal annealing factor and scales the likelihoods. 
//Also removes any particles reported as invalid by the model.
template<class T>
void ParticleFilterPthread<T>::CalcWeights(std::vector<Vectorf> &particles)
{
	std::vector<unsigned char> valid(particles.size());
	mBestParticle = 0;
	fpType total = 0, best = 0, minWeight = 1e30f, annealingFactor = 1;
	mWeights.resize(particles.size());
	uint i = 0;

	ParticleFilterPthread<T>::particles = &particles;
	ParticleFilterPthread<T>::valid = &valid;
	
	//signal to workers that work is available
	workers.SendCmd(workers.THREADS_CMD_PARTICLEWEIGHTS);

	i = 0;
	while(i < particles.size())
	{	if(!valid[i])                                                                //if not valid(model prior), remove the particle from the list
		{	particles[i] = particles[particles.size() - 1];
			mWeights[i] = mWeights[particles.size() - 1];
			valid[i] = valid[valid.size() - 1];
			particles.pop_back(); mWeights.pop_back(); valid.pop_back();
		}
		else
			minWeight = std::min(mWeights[i++], minWeight);                          //find minimum weight
	}
	if((int)particles.size() < mMinParticles) return;                                //bail out if not enough valid particles
	mWeights -= minWeight;                                                           //shift weights to zero for numerical stability
	if(mModel->StdDevs().size() > 1) 
		annealingFactor = BetaAnnealingFactor(mWeights, 0.5f);                       //calculate annealing factor if more than 1 step
	for(i = 0; i < mWeights.size(); i++)
	{	double wa = annealingFactor * mWeights[i];
		mWeights[i] = (float)exp(wa);                                                //exponentiate log-likelihoods scaled by annealing factor
		total += mWeights[i];                                                        //save sum of all weights
		if(i == 0 || mWeights[i] > best)                                             //find highest likelihood particle
		{	best = mWeights[i];
			mBestParticle = i;
		}
	}
	mWeights *= (fpType)1.0 / total;                                                 //normalize weights
}

//generate new particles distributed with std deviation given by the model annealing parameter
template<class T> 
void ParticleFilterPthread<T>::GenerateNewParticles(int k)
{	int p = 0;
	mNewParticles.resize(mNParticles);
	mIndex.resize(mNParticles);
	for(int i = 0; i < (int)mBins.size(); i++)										
		for(uint j = 0; j < mBins[i]; j++)                                           //index particles to be regenerated
			mIndex[p++] = i;
			
	ParticleFilterPthread<T>::annealing_parameter = k;
	//signal to workers that work is available
	workers.SendCmd(workers.THREADS_CMD_NEWPARTICLES);
}

#endif
