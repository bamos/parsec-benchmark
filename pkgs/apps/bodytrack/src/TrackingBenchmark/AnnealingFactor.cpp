//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                           
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : AnnealingFactor.h
//  author : Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//  description : Estimates the annealing factor needed
//				  for the annealed particle filter to target
//				  a desired particle survival rate
//  modified : 
//--------------------------------------------------------------


#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "AnnealingFactor.h"
#include <math.h>

// Computes the difference between the particle survival rate alpha and the desired value alpha_desired
float delta_alpha(float beta,std::vector<float> &ets,float alpha_desired)
{ 
	int N = (int) ets.size();
	float B=0;
	float F=0;
	float ei;
	double v;

	for(int i=0;i<N;i++)
	{	v = beta*ets[i];
		if(v>LOG_MAX_FLOAT) return (-alpha_desired); // exit the function is the value is too large. Return the limit value
		ei = (float)exp(v);
		B += ei;
		F += (ei*ei);
	}
	return (((B*B/F)/N) - alpha_desired);
};


// Estimates the optimal beta coefficient to achieve a particle survival rate of alpha_desired
float BetaAnnealingFactor(std::vector<float> &ets,float alpha_desired, float beta_min, float beta_max )
{
	int n_iterations = 0;

	// conpute the values at the range extremas:
	float delta_alpha_min = delta_alpha(beta_min,ets,alpha_desired);
	float delta_alpha_max = delta_alpha(beta_max,ets,alpha_desired);

	// Make sure that there is a zero crossing within the range. Otherwise, return 1.0 (i.e. equivalent to no scaling)
	if (((delta_alpha_min>0)&&(delta_alpha_max>0))||((delta_alpha_min<0)&&(delta_alpha_max<0))) return 1.0f;

	float beta = (beta_min + beta_max)/2;
	float delta_alpha_beta = delta_alpha(beta,ets,alpha_desired);

	while(((delta_alpha_beta<0.0 ? -delta_alpha_beta : delta_alpha_beta)>ALPHA_PRECISION) && (n_iterations < N_ITER_MAX))
	{	if(((delta_alpha_min>0)&&(delta_alpha_beta>0)) || ((delta_alpha_min<=0)&&(delta_alpha_beta<0)))
		{   beta_min = beta;
			delta_alpha_min = delta_alpha_beta;
		}
		else
		{	beta_max = beta;
			delta_alpha_max = delta_alpha_beta;
		}
		beta = (beta_min + beta_max)/2;
		delta_alpha_beta = delta_alpha(beta,ets,alpha_desired);
		n_iterations++;
	}
	return beta;
}


/********************  FUNCTIONS FOR DEBUG PURPOSES ***********************/

// Sets the value of the vector ets for debug purposes:
void set_ets(std::vector<float> &ets)
{
	//float vals[25] = {
	//	4.3128728695f,0.0000000000f,18.9053596273f,13.1425134762f,17.4484091251f,17.2461182394f,16.6566663579f,10.0850678559f,
	//	15.7001027141f,14.6932104123f,9.4194667162f,8.7384354437f,10.1139429009f,14.0191494204f,9.4412632449f,17.4711466418f,
	//	8.6149993135f,17.4476798776f,18.8192540642f,8.4540423726f,14.1630306404f,17.2265023809f,3.5324361101f,19.1927652329f,
	//	12.5504577255f};

	float vals[25] = {
		0.0431287287f,0.0000000000f,0.1890535963f,0.1314251348f,0.1744840913f,0.1724611824f,0.1665666636f,0.1008506786f,
		0.1570010271f,0.1469321041f,0.0941946672f,0.0873843544f,0.1011394290f,0.1401914942f,0.0944126324f,0.1747114664f,
		0.0861499931f,0.1744767988f,0.1881925406f,0.0845404237f,0.1416303064f,0.1722650238f,0.0353243611f,0.1919276523f,
		0.1255045773f};

	ets.resize(25);
	for(int i=0;i<25;i++)
		ets[i] = vals[i];
}
