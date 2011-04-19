// BVrama.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

using namespace cv;

int _tmain(int argc, _TCHAR* argv[])
{
		//Cloned and Updated - Ben
		// Open the file.
        IplImage *img = cvLoadImage("photo.jpg");

        if (!img) {
                printf("Error: Couldn't open the image file.\n");
                return 1;
        }

		Mat src = Mat(img,true);
		Mat dst;
		bilateralFilter( src, dst, 15, 100.0, 100.0,BORDER_DEFAULT);

		imwrite("photo1.jpg",dst);

        // Display the image.
        cvNamedWindow("Image:", CV_WINDOW_AUTOSIZE);
        cvShowImage("Image:", img);
		cvNamedWindow("mat", 1);
		imshow("mat", dst);


        // Wait for the user to press a key in the GUI window.
        cvWaitKey(0);

        // Free the resources.
        cvDestroyWindow("Image:");
        cvReleaseImage(&img);
		//test
        
        return 0;
}