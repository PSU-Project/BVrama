#pragma once
#include "defs.h"

class BVPanorama
{
	IplImage	*imageLeft;
	Keypoint	keyLeft;
	IplImage	*imageRight;
	Keypoint	keyRight;
	M_Keypoints matchedPoints;
	CvMat		*homography;
	IplImage	*completedResult;
	
	//Helper Functions
	Keypoint	CheckForMatch (Keypoint key, Keypoint klist);
	void		FindMatchedPoints();
	int			DistSquared (Keypoint k1, Keypoint k2);
	void		FindHomographyMatrix();
	void		Stitch(float *p1, float *p2);

public:
				BVPanorama	(void);
				~BVPanorama	(void);
	void		setLeft		(IplImage *l, Keypoint kL);
	void		setRight	(IplImage *r, Keypoint kR);
	void		makePanorama();
	IplImage*	getResult();
	void		Reset();
};

