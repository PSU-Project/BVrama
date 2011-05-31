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
#include "BVPanorama.h"

using namespace cv;



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
    Keypoint key1 = NULL, key2 = NULL, key3 = NULL;
	M_Keypoints M_list;

    /* Parse command line arguments and read given files.  The command
       line must specify two input images and two files of keypoints
       using command line arguments as follows:
          match -im1 i1.pgm -k1 k1.key -im2 i2.pgm -k2 k2.key > result.v
    */
	/****** reading key files *****************/
	char k1[20]={"setApic1.key"};
	char k2[20]={"setApic2.key"};
	char k3[20]={"setApic3.key"};

	key1 = ReadKeyFile(k1);
	key2 = ReadKeyFile(k2);
	key3 = ReadKeyFile(k3);
	
    if (key1 == NULL || key2 == NULL)
      FatalError("Command line does not specify keys.");
	/*******************************************/
    M_list = FindMatches( key1, key2, &count);
	
	// Open the file.
    
	IplImage *img1 = cvLoadImage("setApic1.jpg");
    if (!img1) {
            printf("Error: Couldn't open the image file.\n");
            return 1;
	}
	IplImage *img2 = cvLoadImage("setApic2.jpg");
    if (!img2) {
            printf("Error: Couldn't open the image file.\n");
            return 1;
	}
	IplImage *img3 = cvLoadImage("setApic3.jpg");
    if (!img3) {
            printf("Error: Couldn't open the image file.\n");
            return 1;
//	}
	/********finding homography********************/
	
	BVPanorama *BVObject = new BVPanorama();
	BVObject->setLeft(img2, key2);
	BVObject->setRight(img3, key3);
	CvMat src3 = cvMat(3,1,CV_32FC1,&p5);
	CvMat dst3 = cvMat(3,1,CV_32FC1,&p6);
	cvMatMulAdd(homographyMatrix,&src1,0,&dst1);
	cvMatMulAdd(homographyMatrix,&src2,0,&dst2);
	cvMatMulAdd(homographyMatrix,&src3,0,&dst3);
	
	//BVObject->setLeft(img1, key1);
	//BVObject->setRight(img2, key2);

	BVObject->makePanorama();
	IplImage *out1 = BVObject->getResult();
	BVObject->Reset();

	//Display the image.
	cvNamedWindow("First Iteration", CV_WINDOW_AUTOSIZE );
	cvShowImage("First Iteration", out1 );
	cvWaitKey();
	
	BVObject->setLeft(img1, key1);
	BVObject->setRight(out1, key2);
	BVObject->makePanorama();
	IplImage *out2 = BVObject->getResult();
	//cvNamedWindow("Out", CV_WINDOW_AUTOSIZE );
	//cvShowImage("Out", out );
	//cvWaitKey();
	
	//Display the image.
	cvNamedWindow("Second Iteration", CV_WINDOW_AUTOSIZE );
	cvShowImage("Second Iteration", out2 );
	cvWaitKey();
	

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

	cvCopy(right, canvas2);
	cvResetImageROI(right);
	cvResetImageROI(canvas2);

    return 0;
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