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
	M_Keypoints M_PtsList, itr;

    /* Find the two closest matches, and put their squared distances in
       distsq1 and distsq2.
    */
    for (k = klist; k != NULL; k = k->next) {
      dsq = DistSquared(key, k);

    M_PtsList = FindMatches(im1, k1, im2, k2);

	// count the number of matches for matrix allocation
	int M_count = 0;
	itr = M_PtsList;
	while (itr != NULL)
	{
		M_count++;
		itr = itr->next;
	}
	
	printf("# of matches: %d\n", M_count);

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

void FindMatches(Image im1, Keypoint keys1, Image im2, Keypoint keys2,int * count)
{
    Keypoint k, match;
    Image result;
	/// my code here ///
	M_Keypoints mkp;
	M_Keypoints mkpl;
	mkpl=NULL;
	/// my code here ///

    /* Match the keys in list keys1 to their best matches in keys2.
    */
    for (k= keys1; k != NULL; k = k->next) {
      match = CheckForMatch(k, keys2);  

      /* Draw a line on the image from keys1 to match.  Note that we
	 must add row count of first image to row position in second so
	 that line ends at correct location in second image.
      */
      if (match != NULL) {
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
}

int main (int argc, char **argv)
{
	printf("hello\n");
    int arg = 0;
	int count = 0;
    Image im1 = NULL, im2 = NULL;
    Keypoint k1 = NULL, k2 = NULL;

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

    FindMatches(im1, k1, im2, k2, &count);
	fprintf(stderr,"%i\n", count);
    return 0;
}

