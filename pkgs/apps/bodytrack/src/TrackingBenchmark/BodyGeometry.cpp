//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : 
//  authors : Scott Ettinger - scott.m.ettinger@intel.com
//			  Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//			  
//  description : Body geometry description
//  modified : 
//--------------------------------------------------------------

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <fstream>
#include <iostream>
#include "BodyGeometry.h"

#define DMatrixf DMatrix<float>

using namespace std;

//Multiplication by a diagonal matrix
inline void Scale(DMatrixf &m, float s1, float s2, float s3)
{	m(0,0) *= s1;  m(1,0) *= s1; m(2,0) *= s1;
	m(0,1) *= s2;  m(1,1) *= s2; m(2,1) *= s2;
	m(0,2) *= s3;  m(1,2) *= s3; m(2,2) *= s3;
}

//Matrix pre-translation
inline void PreTranslate(DMatrixf &m, float t1, float t2, float t3)
{
	m(0,3) = m(0,0) * t1 + m(0,1) * t2 + m(0,2) * t3 + m(0,3);
	m(1,3) = m(1,0) * t1 + m(1,1) * t2 + m(1,2) * t3 + m(1,3);
	m(2,3) = m(2,0) * t1 + m(2,1) * t2 + m(2,2) * t3 + m(2,3);
}

//Create the geometric representation of the kinematic tree model given a body pose (angles and translation)
void BodyGeometry::ComputeGeometry(BodyPose &angles, BodyParameters &params)
{
	const DMOrder ANGLE_DECOMPOSITION_ORDER = ZXY;

	//create the torso conic cylinder and transform
	DMatrixf globalTpelvis(angles(0), angles(1), angles(2), ANGLE_DECOMPOSITION_ORDER);					//create torso transform given euler angles and translation
	globalTpelvis.SetTranslation(angles(3), angles(4), angles(5));
	mCylinders[0].Set(params.limbs[0][0], params.limbs[0][1], params.lengths[0]);						//set the torso conic cylinder values
	mCylinders[0].pose = globalTpelvis;
	Scale(mCylinders[0].pose, params.limbs[0][2], params.limbs[0][3], 1);								//scale transform by limb parameters

	//create left thigh using geometric transformations from the torso and model parameters
	DMatrixf globalTlpelvis = (globalTpelvis * DMatrixf(YRot180));
	PreTranslate(globalTlpelvis, 0, params.lengths[1], 0);
	DMatrixf lthighTlpelvis(angles(6), angles(7), angles(8), ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTlthigh = globalTlpelvis * Inverse(lthighTlpelvis);
	mCylinders[1].Set(params.limbs[1][0], params.limbs[1][1], params.lengths[2]);
	mCylinders[1].pose = globalTlthigh;
	Scale(mCylinders[1].pose, params.limbs[1][2], params.limbs[1][3], 1);

	//create left lower leg cascading from the left thigh transform and model parameters
	PreTranslate(globalTlthigh, 0, 0, params.lengths[2]);
	DMatrixf ltibiaTlthigh(0, angles(9), 0, ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTltibia = globalTlthigh * Inverse(ltibiaTlthigh);
	mCylinders[2].Set(params.limbs[2][0], params.limbs[2][1], params.lengths[3]);
	mCylinders[2].pose = globalTltibia;
	Scale(mCylinders[2].pose, params.limbs[2][2], params.limbs[2][3], 1); 

	//create right thigh
	DMatrixf globalTrpelvis = (globalTpelvis * DMatrixf(YRot180));
	PreTranslate(globalTrpelvis, 0, -params.lengths[5], 0);
	DMatrixf rthighTrpelvis(angles(10),angles(11),angles(12),ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTrthigh = globalTrpelvis * Inverse(rthighTrpelvis);
	mCylinders[3].Set(params.limbs[3][0], params.limbs[3][1], params.lengths[6]);
	mCylinders[3].pose = globalTrthigh;
	Scale(mCylinders[3].pose, params.limbs[3][2], params.limbs[3][3], 1);

	//create left lower leg
	PreTranslate(globalTrthigh, 0, 0, params.lengths[6]);
	DMatrixf rtibiaTrthigh(0, angles(13), 0, ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTrtibia = globalTrthigh * Inverse(rtibiaTrthigh);
	mCylinders[4].Set(params.limbs[4][0], params.limbs[4][1], params.lengths[7]);
	mCylinders[4].pose = globalTrtibia;
	Scale(mCylinders[4].pose, params.limbs[4][2], params.limbs[4][3], 1);

	DMatrixf thoraxTpelvis(0, angles(26), angles(27), ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTthorax = (globalTpelvis * DMatrixf(XRot180)) * (Inverse(thoraxTpelvis));
	PreTranslate(globalTthorax, 0, 0, -params.lengths[0]);
	
	//create left upper arm
	DMatrixf lclavicleTlthorax(angles(14), angles(15), 0.0, ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTlclavicle = (globalTthorax * DMatrixf(XRot90)) * Inverse(lclavicleTlthorax);
	PreTranslate(globalTlclavicle, 0, 0, -params.lengths[9]);
	DMatrixf lshoulderTlclavicle(angles(16), angles(17), angles(18), ANGLE_DECOMPOSITION_ORDER);
	DMatrixf r = DMatrixf(XRot90) * DMatrixf(ZRot180) * DMatrixf(YRot90);
	DMatrixf globalTlshoulder = globalTlclavicle * r * Inverse(lshoulderTlclavicle);
	mCylinders[5].Set(params.limbs[5][0], params.limbs[5][1], -params.lengths[10]);
	mCylinders[5].pose = globalTlshoulder;
	Scale(mCylinders[5].pose, params.limbs[5][2], params.limbs[5][3], 1);
	
	//create left lower arm
	PreTranslate(globalTlshoulder, 0, 0, -params.lengths[10]);
	DMatrixf lelbowTlshoulder(0.0, angles(19), 0.0, ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTlelbow = globalTlshoulder * Inverse(lelbowTlshoulder);
	mCylinders[6].Set(params.limbs[6][0], params.limbs[6][1], -params.lengths[11]);
	mCylinders[6].pose = globalTlelbow;
	Scale(mCylinders[6].pose, params.limbs[6][2], params.limbs[6][3], 1);
	
	//create right upper arm
	DMatrixf rclavicleTrthorax(angles(20), angles(21), 0.0, ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTrclavicle = globalTthorax * DMatrixf(XRot270) * Inverse(rclavicleTrthorax);
	PreTranslate(globalTrclavicle, 0, 0, -params.lengths[13]);
	DMatrixf rshoulderTrclavicle(angles(22),angles(23),angles(24),ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTrshoulder = (globalTrclavicle * r) * Inverse(rshoulderTrclavicle);
	mCylinders[7].Set(params.limbs[7][0], params.limbs[7][1], -params.lengths[14]);
	mCylinders[7].pose = globalTrshoulder;
	Scale(mCylinders[7].pose, params.limbs[7][2], params.limbs[7][3], 1);
	
	//create right lower arm
	PreTranslate(globalTrshoulder, 0, 0, -params.lengths[14]);
	DMatrixf relbowTrshoulder(0.0, angles(25), 0.0, ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalTrelbow = globalTrshoulder * Inverse(relbowTrshoulder);
	mCylinders[8].Set(params.limbs[8][0], params.limbs[8][1], -params.lengths[15]);
	mCylinders[8].pose = globalTrelbow;
	Scale(mCylinders[8].pose, params.limbs[8][2], params.limbs[8][3], 1);

	//create head
	DMatrixf headTthorax(angles(28), angles(29), angles(30), ANGLE_DECOMPOSITION_ORDER);
	DMatrixf globalThead = (globalTthorax * DMatrixf(XRot180)) * Inverse(headTthorax);
	mCylinders[9].Set(params.limbs[9][0], params.limbs[9][1], params.lengths[17]);
	mCylinders[9].pose = globalThead;
	Scale(mCylinders[9].pose, params.limbs[9][2], params.limbs[9][3], 1);

}

// Load the body shape parameters from a file
bool BodyParameters::InitBodyParameters(const string fname)
{	
	ifstream f(fname.c_str());
	if(!f.is_open())
	{	cout << "Unable to open Body Parameter file : " << fname << endl;
		return false;
	}
	for(int i = 0; i < N_PARTS; i++)
		for(int j = 0; j < 4; j++)
			f >> limbs[i][j];
	for(int i = 0; i < N_LENGTHS; i++)
		f >> lengths[i];
	return true;
}

// Checks if 2 cylinders intersect. Returns true if they do.
bool BodyGeometry::IntersectingCylinders(KTCylinder &cylA, KTCylinder &cylB)
{
	float numPA2 = ((cylA.length / std::max(cylA.bottom,cylA.top))/2.00f);
	int numPA = (int) ((numPA2<0.0 ? -numPA2 : numPA2) + 0.5f);

	float numPB2 = (cylB.length / std::max(cylB.bottom,cylB.top))/2.00f;
	int numPB = (int) ((numPB2<0.0 ? -numPB2 : numPB2) + 0.5f);

	numPB = std::min(numPB, 20);
	numPA = std::min(numPA, 20);

	Vector3f obA(0.00f,0.00f,0.00f);
	Vector3f otA(0.00f,0.00f,cylA.length);
	Vector3f oA[2];
	oA[0] = cylA.pose * obA;
	oA[1] = cylA.pose * otA;

	Vector3f obB(0.00f,0.00f,0.00f);
	Vector3f otB(0.00f,0.00f,cylB.length);
	Vector3f oB[2];
	oB[0] = cylB.pose * obB;
	oB[1] = cylB.pose * otB;

	//approximate first conic cylinder as a series of spheres
	float deltaPA = 1.00f / (float) numPA;
	Vector3f d12 = oA[1] - oA[0];
	mCentersA.resize(numPA+1);
	mRadiiA.resize(numPA+1);
	float alpha;
	float delta_r = cylA.top - cylA.bottom;
	double val = delta_r/cylA.length;
	float scaling = (float)sqrt(1.00 + (val*val));
	for(int i=0;i<=numPA;i++)
	{	alpha = ((float) i)*deltaPA;
		mCentersA[i].Set(oA[0].x + (alpha * d12.x) , oA[0].y + (alpha * d12.y) , oA[0].z + (alpha * d12.z) ); // = oA[0] + (alpha * d12);
		mRadiiA[i] = (cylA.bottom + alpha * delta_r)/scaling;
	}

	//approximate second conic cylinder as a series of spheres
	float deltaPB = 1.00f / (float) numPB;
	d12 = oB[1] - oB[0];
	mCentersB.resize(numPB+1);
	mRadiiB.resize(numPB+1);
	delta_r = cylB.top - cylB.bottom;
	val = delta_r/cylB.length;
	scaling = (float)sqrt(1.00f + (val*val));
	for(int i=0;i<=numPB;i++)
	{	alpha = ((float) i)*deltaPB;
		mCentersB[i].Set(oB[0].x + (alpha * d12.x) , oB[0].y + (alpha * d12.y) , oB[0].z + (alpha * d12.z) ); // = oB[0] + alpha * d12;
		mRadiiB[i] = (cylB.bottom + alpha * delta_r)/scaling;
	}

	//check if any spheres intersect from one cylinder to the other
	Vector3f difC;
	float rAB;
	float nrmsq;
	for(int i=0;i<=numPA;i++)
	{
		for(int j=0;j<=numPB;j++)
		{
			difC.Set(mCentersA[i].x - mCentersB[j].x , mCentersA[i].y - mCentersB[j].y , mCentersA[i].z - mCentersB[j].z ); //  = mCentersA[i] - mCentersB[j]
			nrmsq = (difC.x * difC.x) + (difC.y * difC.y) + (difC.z * difC.z);
			rAB = mRadiiA[i] + mRadiiB[j];
			if (rAB*rAB> nrmsq)
				return true;
		}
	}
	return false;
};

//Check that the body is valid by checking that no pair of body part intersect
bool BodyGeometry::Valid()
{
	if(IntersectingCylinders(mCylinders[6],mCylinders[0])) return false;		//Check left lower arm and torso
	if(IntersectingCylinders(mCylinders[8],mCylinders[0])) return false;		//Check right lower arm and torso
	if(IntersectingCylinders(mCylinders[2],mCylinders[4])) return false;		//Check left lower leg and right lower leg
	return true;
}
