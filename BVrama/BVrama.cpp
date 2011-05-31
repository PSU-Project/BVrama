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
	}
	
	
	BVPanorama *BVObject = new BVPanorama();
	BVObject->setLeft(img2, key2);
	BVObject->setRight(img3, key3);
	
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
	
	//Display the image.
	cvNamedWindow("Second Iteration", CV_WINDOW_AUTOSIZE );
	cvShowImage("Second Iteration", out2 );
	cvWaitKey();
	



    return 0;
}