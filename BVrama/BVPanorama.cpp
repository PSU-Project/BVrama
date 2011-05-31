#include "stdafx.h"
#include "defs.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include "BVPanorama.h"

BVPanorama::BVPanorama(void)
{
	imageLeft		= NULL;
	keyLeft			= NULL;
	imageRight		= NULL;
	keyRight		= NULL;
	matchedPoints	= NULL;
	homography		= NULL;
	completedResult = NULL;
	
}


BVPanorama::~BVPanorama(void)
{

}

void BVPanorama::setLeft(IplImage *l, Keypoint kL)
{
	imageLeft	= l;
	keyLeft		= kL;
}

void BVPanorama::setRight(IplImage *r, Keypoint kR)
{
	imageRight	= r;
	keyRight	= kR;
	
}

IplImage* BVPanorama::getResult()
{
	return completedResult;
}

void BVPanorama::Reset()
{
	imageLeft		= NULL;
	keyLeft			= NULL;
	imageRight		= NULL;
	keyRight		= NULL;
	matchedPoints	= NULL;
	homography		= NULL;
	completedResult = NULL;
}
void BVPanorama::FindMatchedPoints()
{
	Keypoint k, match;
    int count = 0;

	/// my code here ///
	M_Keypoints mkp;
	M_Keypoints mkpl;
	mkpl=NULL;
	/// my code here ///

    /* Match the keys in list keys1 to their best matches in keys2.
    */
    for (k = keyLeft; k != NULL; k = k->next) 
	{
      match = CheckForMatch(k, keyRight);  
      if (match != NULL) 
	  {
		/////my code here/////
		  mkp = (M_Keypoints) malloc(sizeof(struct M_KeypointsSt));
		  mkp->k1=k;
		  mkp->k2=match;
		  mkp->next=mkpl;
		  mkpl=mkp;
		//////////////////////
		count=count+1;
      }
    }
	matchedPoints = mkpl;
}

Keypoint BVPanorama::CheckForMatch(Keypoint key, Keypoint klist)
{
	 int dsq, distsq1 = 100000000, distsq2 = 100000000;
    Keypoint k, minkey = NULL;

    /* Find the two closest matches, and put their squared distances in
       distsq1 and distsq2.
    */
    for (k = klist; k != NULL; k = k->next) {
      dsq = DistSquared(key, k);

      if (dsq < distsq1) {
	distsq2 = distsq1;
	distsq1 = dsq;
	minkey = k;
      } else if (dsq < distsq2) {
	distsq2 = dsq;
      }
    }

    /* Check whether closest distance is less than 0.6 of second. */
    if (10 * 10 * distsq1 < 6 * 6 * distsq2)
      return minkey;
    else return NULL;
}

int BVPanorama::DistSquared(Keypoint k1, Keypoint k2)
{
    int i, dif, distsq = 0;
    unsigned char *pk1, *pk2;

    pk1 = k1->descrip;
    pk2 = k2->descrip;

    for (i = 0; i < 128; i++) {
      dif = (int) *pk1++ - (int) *pk2++;
      distsq += dif * dif;
    }
    return distsq;
}

void BVPanorama::FindHomographyMatrix()
{
	M_Keypoints itr;
	itr = matchedPoints;
	int cnt = 0;
	int rows, cols, i;
	float *pts1Array, *pts2Array;
	bool even = true;

	while (itr != NULL)
	{
		cnt++;
		itr = itr->next;
	}

	//Size the array
	rows = (cnt);
	pts1Array = new float[(rows*2)];
	pts2Array = new float[(rows*2)];

	itr = matchedPoints;
	//populate the array with points (must be square)
	for (i = 0; i<rows*2; i=i+2)
	{
		pts1Array[i]		= itr->k1->col;
		pts1Array[i+1]		= itr->k1->row;

		pts2Array[i]		= itr->k2->col;
		pts2Array[i+1]		= itr->k2->row;

		itr = itr->next;	
	}
	
	CvMat matdst			= cvMat(rows, 2, CV_32FC1, pts1Array);
	CvMat matsrc		    = cvMat(rows, 2, CV_32FC1, pts2Array); 
	CvMat* answerMat        = cvCreateMat(3,3,CV_32FC1);
	
	cvFindHomography(&matsrc, &matdst , answerMat, CV_RANSAC);
	
	homography = answerMat;

	delete pts1Array;
	delete pts2Array;
}

void BVPanorama::Stitch(float *p1, float *p2)
{
	float p;
	if((imageLeft->width - p1[0])>(imageLeft->width - p2[0]))
		p=imageLeft->width - p2[0];
	else
		p=imageLeft->width - p1[0];
	IplImage * canvas1 = cvCreateImage(cvSize(imageRight->width,imageRight->height),IPL_DEPTH_8U,3);
	IplImage * canvas2 = cvCreateImage(cvSize(imageRight->width,imageRight->height),IPL_DEPTH_8U,3);
	IplImage * canvas3 = cvCreateImage(cvSize(imageRight->width,imageRight->height),IPL_DEPTH_8U,3);
	//Copy the first image onto the blank test image
	cvSetImageROI(canvas1, cvRect(0, 0, imageLeft->width, imageLeft->height));
	cvSetImageROI(imageLeft, cvRect(0, 0,imageLeft->width,imageLeft->height));
	cvCopy(imageLeft, canvas1);
	cvResetImageROI(imageLeft);
	cvResetImageROI(canvas1);
	
	//Copy the second image onto the blank test image
	cvSetImageROI(canvas2, cvRect(0, 0, imageRight->width, imageRight->height));
	cvSetImageROI(imageRight, cvRect(0, 0, imageRight->width, imageRight->height));
	cvCopy(imageRight, canvas2);
	cvResetImageROI(imageRight);
	cvResetImageROI(canvas2);
	
	//blending
	cvSetImageROI(canvas1, cvRect(imageLeft->width-p,0,p,imageLeft->height));
	cvSetImageROI(canvas2, cvRect(imageLeft->width-p,0,p,imageLeft->height));
	cvSetImageROI(canvas3, cvRect(imageLeft->width-p,0,p,imageLeft->height));
	cvAddWeighted(canvas2,.5,canvas1,.5,0,canvas3);
	cvResetImageROI(canvas1);
	cvResetImageROI(canvas2);
	cvResetImageROI(canvas3);

	cvSetImageROI(canvas1, cvRect(0,0,imageLeft->width-p,imageLeft->height));
	cvSetImageROI(canvas3, cvRect(0,0,imageLeft->width-p,imageLeft->height));
	cvCopy(canvas1, canvas3);
	cvResetImageROI(canvas1);
	cvResetImageROI(canvas3);

	cvSetImageROI(canvas1, cvRect(0,0,imageLeft->width-p+1,imageLeft->height));
	cvSetImageROI(canvas3, cvRect(0,0,imageLeft->width-p+1,imageLeft->height));
	cvCopy(canvas1, canvas3);
	cvResetImageROI(canvas1);
	cvResetImageROI(canvas3);

	cvSetImageROI(canvas2, cvRect(imageLeft->width-1,0,imageRight->width-imageLeft->width,imageRight->height));
	cvSetImageROI(canvas3, cvRect(imageLeft->width-1,0,imageRight->width-imageLeft->width,imageRight->height));
	cvCopy(canvas2, canvas3);
	cvResetImageROI(canvas2);
	cvResetImageROI(canvas3);

//	cvSetImageROI(canvas3, cvRect(0,10,right->width,left->height-30));
	completedResult = canvas3;


}

void BVPanorama::makePanorama()
{
	//Check for null values and other errors
	FindMatchedPoints();
	FindHomographyMatrix();

	// Determine where to crop
	// Also determine relative position with overlap
	float p1[]={0,0,1};
	float p2[3];
	float p3[]={0,imageRight->height,1};
	float p4[3];
	float p5[]={imageRight->width,imageRight->height,1};
	float p6[3];
	CvMat src1 = cvMat(3,1,CV_32FC1,&p1);
	CvMat dst1 = cvMat(3,1,CV_32FC1,&p2);
	CvMat src2 = cvMat(3,1,CV_32FC1,&p3);
	CvMat dst2 = cvMat(3,1,CV_32FC1,&p4);
	CvMat src3 = cvMat(3,1,CV_32FC1,&p5);
	CvMat dst3 = cvMat(3,1,CV_32FC1,&p6);
	cvMatMulAdd(homography,&src1,0,&dst1);
	cvMatMulAdd(homography,&src2,0,&dst2);
	cvMatMulAdd(homography,&src3,0,&dst3);

	IplImage * result = cvCreateImage(cvSize(p6[0]*2,imageRight->height),IPL_DEPTH_8U,3);
	cvWarpPerspective(imageRight, result, homography);
	imageRight = result;
	Stitch(p2, p4);

}