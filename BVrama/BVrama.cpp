// BVrama.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "defs.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>

using namespace cv;

IplImage* Stitch(IplImage* img1, IplImage* result,float * p1, float * p2);

int DistSquared(Keypoint k1, Keypoint k2)
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

Keypoint CheckForMatch(Keypoint key, Keypoint klist)
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

M_Keypoints FindMatches(Keypoint keys1, Keypoint keys2,int * count)
{
    Keypoint k, match;
  
	/// my code here ///
	M_Keypoints mkp;
	M_Keypoints mkpl;
	mkpl=NULL;
	/// my code here ///

    /* Match the keys in list keys1 to their best matches in keys2.
    */
    for (k= keys1; k != NULL; k = k->next) 
	{
      match = CheckForMatch(k, keys2);  
      if (match != NULL) 
	  {
		/////my code here/////
		  mkp = (M_Keypoints) malloc(sizeof(struct M_KeypointsSt));
		  mkp->k1=k;
		  mkp->k2=match;
		  mkp->next=mkpl;
		  mkpl=mkp;
		//////////////////////
		*count=*count+1;
      }
    }
	return mkpl;
}

CvMat* FindHomographyMatrix (M_Keypoints M_list)
{
	M_Keypoints itr;
	itr = M_list;
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

	itr = M_list;
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
	
	return answerMat;
}


void MyStitch(IplImage* left, IplImage* right)
{
	IplImage * lg1 = cvCreateImage(cvSize(left->width/2,left->height/2),left->depth,left->nChannels);
	IplImage * rg1 = cvCreateImage(cvSize(right->width/2,right->height/2),right->depth,right->nChannels);
	cvPyrDown(left,lg1);
	cvPyrDown(right,rg1);
	
	IplImage * lg2 = cvCreateImage(cvSize(left->width,left->height),left->depth,left->nChannels);
	IplImage * rg2 = cvCreateImage(cvSize(right->width,right->height),right->depth,right->nChannels);
	cvPyrUp(lg1,lg2);
	cvPyrUp(rg1,rg2);
	IplImage * l = cvCreateImage(cvSize(left->width,left->height),left->depth,left->nChannels);
	IplImage * r = cvCreateImage(cvSize(right->width,right->height),right->depth,right->nChannels);
	cvSub(left,lg2,l);
	cvSub(right,rg2,r);

	/*******************************/
	IplImage * result = cvCreateImage(cvSize(600,600),IPL_DEPTH_8U,3);

	//Copy the first image onto the blank test image
	//add the remainder of the image to the end
	cvSetImageROI(result, cvRect(0, 0, right->width, right->height));
	cvSetImageROI(r, cvRect(0, 0,r->width,r->height));
	cvCopy(r, result);
	cvResetImageROI(r);
	cvResetImageROI(result);
	//set the region of interest
	cvSetImageROI(result, cvRect(0, 0, l->width, l->height));
	cvSetImageROI(l, cvRect(0, 0, l->width, l->height));
	cvAddWeighted(l,1,result,0,0,result);
	//always release the region of interest
	cvResetImageROI(l);
	cvResetImageROI(result);

	/************************************/
	cvNamedWindow("L1", CV_WINDOW_AUTOSIZE );
	cvShowImage("Out", result );
	cvWaitKey();

	
}
int main (int argc, char **argv)
{
	int count = 0;
    Keypoint key1 = NULL, key2 = NULL;
	M_Keypoints M_list;

    /* Parse command line arguments and read given files.  The command
       line must specify two input images and two files of keypoints
       using command line arguments as follows:
          match -im1 i1.pgm -k1 k1.key -im2 i2.pgm -k2 k2.key > result.v
    */
	/****** reading key files *****************/
	char k1[20]={"temp.key"};
	char k2[20]={"3.key"};

	key1 = ReadKeyFile(k1);
	key2 = ReadKeyFile(k2);
	
    if (key1 == NULL || key2 == NULL)
      FatalError("Command line does not specify keys.");
	/*******************************************/

    M_list = FindMatches( key1, key2, &count);
	
	// Open the file.
    IplImage *img1 = cvLoadImage("temp.jpg");
    if (!img1) {
            printf("Error: Couldn't open the image file.\n");
            return 1;
	}
	IplImage *img2 = cvLoadImage("3c.jpg");
    if (!img2) {
            printf("Error: Couldn't open the image file.\n");
            return 1;
	}

	M_Keypoints itr;
	itr = M_list;

	/* display matched points */
//	while(itr!=NULL)
//	{
//		cvCircle(img1,cvPoint(itr->k1->col,itr->k1->row),3,cvScalar(0,0,255),1);
//		cvCircle(img2,cvPoint(itr->k2->col,itr->k2->row),3,cvScalar(0,0,255),1);
//		itr=itr->next;
//	}
	/********finding homography********************/
	CvMat* homographyMatrix = FindHomographyMatrix(M_list);
	
	float p1[]={0,0,1};
	float p2[3];
	float p3[]={0,img2->height,1};
	float p4[3];
	float p5[]={img2->width,0,1};
	float p6[3];
	CvMat src1 = cvMat(3,1,CV_32FC1,&p1);
	CvMat dst1 = cvMat(3,1,CV_32FC1,&p2);
	CvMat src2 = cvMat(3,1,CV_32FC1,&p3);
	CvMat dst2 = cvMat(3,1,CV_32FC1,&p4);
	CvMat src3 = cvMat(3,1,CV_32FC1,&p5);
	CvMat dst3 = cvMat(3,1,CV_32FC1,&p6);
	cvMatMulAdd(homographyMatrix,&src1,0,&dst1);
	cvMatMulAdd(homographyMatrix,&src2,0,&dst2);
	cvMatMulAdd(homographyMatrix,&src3,0,&dst3);
	
	/**********************************************/
	IplImage * result = cvCreateImage(cvSize(p6[0]*2,img2->height),IPL_DEPTH_8U,3);
	cvWarpPerspective(img2, result, homographyMatrix);

	
	if(!cvSaveImage("CircledPic9.jpg",img1)) 
	{
		printf("Could not save: %s\n","CircledPic9.jpg");
	}
	if(!cvSaveImage("CircledPic10.jpg",img2)) 
	{
		printf("Could not save: %s\n","CircledPic10.jpg");
	}
	if(!cvSaveImage("CircledPic10Result.jpg",result)) 
	{
		printf("Could not save: %s\n","CircledPic10Result.jpg");
	}


	
	IplImage* out=Stitch(img1, result,p2,p4);

	//Display the image.
	//cvNamedWindow("Out", CV_WINDOW_AUTOSIZE );
	//cvShowImage("Out", out );
	//cvWaitKey();

    return 0;
}


IplImage* Stitch(IplImage* left, IplImage* right,float *p1, float* p2)
{
	float p;
	if((left->width - p1[0])>(left->width - p2[0]))
		p=left->width - p2[0];
	else
		p=left->width - p1[0];
	IplImage * canvas1 = cvCreateImage(cvSize(right->width,right->height),IPL_DEPTH_8U,3);
	IplImage * canvas2 = cvCreateImage(cvSize(right->width,right->height),IPL_DEPTH_8U,3);
	IplImage * canvas3 = cvCreateImage(cvSize(right->width,right->height),IPL_DEPTH_8U,3);
	//Copy the first image onto the blank test image
	cvSetImageROI(canvas1, cvRect(0, 0, left->width, left->height));
	cvSetImageROI(left, cvRect(0, 0,left->width,left->height));
	cvCopy(left, canvas1);
	cvResetImageROI(left);
	cvResetImageROI(canvas1);
	
	//Copy the second image onto the blank test image
	cvSetImageROI(canvas2, cvRect(0, 0, right->width, right->height));
	cvSetImageROI(right, cvRect(0, 0, right->width, right->height));
	cvCopy(right, canvas2);
	cvResetImageROI(right);
	cvResetImageROI(canvas2);
	
	//blending
	cvSetImageROI(canvas1, cvRect(left->width-p,0,p,left->height));
	cvSetImageROI(canvas2, cvRect(left->width-p,0,p,left->height));
	cvSetImageROI(canvas3, cvRect(left->width-p,0,p,left->height));
	cvAddWeighted(canvas2,.5,canvas1,.5,0,canvas3);
	cvResetImageROI(canvas1);
	cvResetImageROI(canvas2);
	cvResetImageROI(canvas3);

	cvSetImageROI(canvas1, cvRect(0,0,left->width-p,left->height));
	cvSetImageROI(canvas3, cvRect(0,0,left->width-p,left->height));
	cvCopy(canvas1, canvas3);
	cvResetImageROI(canvas1);
	cvResetImageROI(canvas3);

	cvSetImageROI(canvas1, cvRect(0,0,left->width-p+1,left->height));
	cvSetImageROI(canvas3, cvRect(0,0,left->width-p+1,left->height));
	cvCopy(canvas1, canvas3);
	cvResetImageROI(canvas1);
	cvResetImageROI(canvas3);

	cvSetImageROI(canvas2, cvRect(left->width-1,0,right->width-left->width,right->height));
	cvSetImageROI(canvas3, cvRect(left->width-1,0,right->width-left->width,right->height));
	cvCopy(canvas2, canvas3);
	cvResetImageROI(canvas2);
	cvResetImageROI(canvas3);

//	cvSetImageROI(canvas3, cvRect(0,10,right->width,left->height-30));

	cvNamedWindow("Out", CV_WINDOW_AUTOSIZE );
	cvShowImage("Out", canvas3 );
	cvSaveImage("out2.jpg",canvas3);
	cvWaitKey();
	return canvas2;
}