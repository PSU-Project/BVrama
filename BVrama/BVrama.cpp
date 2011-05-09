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

M_Keypoints FindMatches(Image im1, Keypoint keys1, Image im2, Keypoint keys2,int * count)
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

int main (int argc, char **argv)
{
    int arg = 0;
	int count = 0;
    Image im1 = NULL, im2 = NULL;
    Keypoint k1 = NULL, k2 = NULL;
	M_Keypoints M_list;

    /* Parse command line arguments and read given files.  The command
       line must specify two input images and two files of keypoints
       using command line arguments as follows:
          match -im1 i1.pgm -k1 k1.key -im2 i2.pgm -k2 k2.key > result.v
    */
    while (++arg < argc) {
      if (! strcmp(argv[arg], "-im1")) 
	im1 = ReadPGMFile(argv[++arg]);
      else if (! strcmp(argv[arg], "-im2")) 
	im2 = ReadPGMFile(argv[++arg]);
      else if (! strcmp(argv[arg], "-k1"))
	k1 = ReadKeyFile(argv[++arg]);
      else if (! strcmp(argv[arg], "-k2"))
	k2 = ReadKeyFile(argv[++arg]);
      else
	FatalError("Invalid command line argument: %s", argv[arg]);
    }
    if (im1 == NULL || im2 == NULL || k1 == NULL || k2 == NULL)
      FatalError("Command line does not specify all images and keys.");

    M_list = FindMatches(im1, k1, im2, k2, &count);
	
	// Open the file.
    IplImage *img1 = cvLoadImage("pic9c.jpg");
    if (!img1) {
            printf("Error: Couldn't open the image file.\n");
            return 1;
	}
	IplImage *img2 = cvLoadImage("pic10c.jpg");
    if (!img2) {
            printf("Error: Couldn't open the image file.\n");
            return 1;
	}

	M_Keypoints itr;
	itr = M_list;

	/* display matched points */
	while(itr!=NULL)
	{
		cvCircle(img1,cvPoint(itr->k1->col,itr->k1->row),3,cvScalar(0,0,255),1);
		cvCircle(img2,cvPoint(itr->k2->col,itr->k2->row),3,cvScalar(0,0,255),1);
		itr=itr->next;
	}
	
	CvMat* homographyMatrix = FindHomographyMatrix(M_list);
	result = cvCloneImage(img);
	//CvMat* result			= cvCreateMat(384,512,CV_32FC3);

 //        Just for checking homography Matrix
	float a1=CV_MAT_ELEM(*homographyMatrix,float,0,0);
	float a2=CV_MAT_ELEM(*homographyMatrix,float,1,0);
	float a3=CV_MAT_ELEM(*homographyMatrix,float,2,0);
	float a4=CV_MAT_ELEM(*homographyMatrix,float,0,1);
	float a5=CV_MAT_ELEM(*homographyMatrix,float,1,1);
	float a6=CV_MAT_ELEM(*homographyMatrix,float,2,1);
	float a7=CV_MAT_ELEM(*homographyMatrix,float,0,2);
	float a8=CV_MAT_ELEM(*homographyMatrix,float,1,2);
	float a9=CV_MAT_ELEM(*homographyMatrix,float,2,2);

	IplImage * cimg1= cvCloneImage(img1);
	IplImage * cimg2= cvCloneImage(img2);

	IplImage * result = cvCreateImage(cvSize(600,550),IPL_DEPTH_8U,3);
	cvWarpPerspective(cimg2, result, homographyMatrix);  

	// Display the image.


    cvNamedWindow("Image1:", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Image2:", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Image3:", CV_WINDOW_AUTOSIZE);
    cvShowImage("Image1:", img1);
	cvShowImage("Image2:", img2);
	cvShowImage("Image3:", result);


    // Wait for the user to press a key in the GUI window.
    cvWaitKey(0);

    // Free the resources.
    cvDestroyWindow("Image:");
    //cvReleaseImage(&img);

    return 0;
}

