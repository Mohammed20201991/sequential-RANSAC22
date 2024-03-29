/* Plane Estimators
 * 
 * Implemented by Levente Hajder
 * hajder@inf.elte.hu
 * 01-07-2021
 */

#include "PlaneEstimation.h"

/* Optimal Plane Estimation
 * Goal: Fit plane to the given spatial points
 * Plane is given in implicit form: Ax + By + Cz + D=0 
 * 
 * Input: 
 * vector<Point3f> pts: input points for plane fitting
 * 
 * Output:
 * float* an array with four elements
 * return[0]: A
 * return[1]: B
 * return[2]: C
 * return[3]: D
 */

float* EstimatePlaneOptimal(vector<Point3f> pts)
{
    int num = pts.size();

    // find center of gravity
    Point3d t(0.0, 0.0, 0.0);

    for (int idx = 0; idx < num; idx++)
    {
        t.x += pts.at(idx).x;
        t.y += pts.at(idx).y;
        t.z += pts.at(idx).z;
    }

    t.x = t.x / num;
    t.y = t.y / num;
    t.z = t.z / num;

    // X*l = 0 (homogenous equation)
    Mat X(num, 3, CV_32F);

    // get matrix X
    for (int idx = 0; idx < num; idx++)
    {
        Point3d pt = pts.at(idx);
        X.at<float>(idx, 0) = pt.x - t.x;
        X.at<float>(idx, 1) = pt.y - t.y;
        X.at<float>(idx, 2) = pt.z - t.z;
    }

    // normal vector l -> eigenvector of X^T X corresponding to the least eigenvalues

    Mat mtx = X.t() * X;
    Mat evals, evecs;

    eigen(mtx, evals, evecs);

    float A = evecs.at<float>(2, 0);
    float B = evecs.at<float>(2, 1);
    float C = evecs.at<float>(2, 2);

    float* ret = new float[4];

    // A, B, C are the normal vector to the plane
    // D = Ax + By + Cz    
    ret[0] = A;
    ret[1] = B;
    ret[2] = C;
    ret[3] = -(A*t.x + B*t.y + C*t.z);    

    return ret;
}

    
/* Plane Estimation
 * Goal: Fit plane to the given spatial points. The fitting is robust, RANSAC method is applied
 * Plane is given in implicit form: Ax + By + Cz + D=0 
 * 
 * Input: 
 * vector<Point3f> pts: input points for plane fitting
 * threshold: threshold for point-plane distabce
 * iterateNum: number of iteration within RANSAC
 * 
 * 
 * Output:
 * float* an array with four elements
 * return[0]: A
 * return[1]: B
 * return[2]: C
 * return[3]: D
 * 
 */
    

float* EstimatePlaneRANSAC(vector<Point3f> pts, float threshold, int iterateNum)
{
    int num=pts.size();
        
    int bestSampleInlierNum=0;
    float bestPlane[4];
        
    for(int iter=0; iter<iterateNum; iter++)
    {
        float rand1=(float)(rand())/RAND_MAX;
        float rand2=(float)(rand())/RAND_MAX;
        float rand3=(float)(rand())/RAND_MAX;                
                
        // Generate three different(!) random numbers:
        int index1=(int)(rand1*num);
        int index2=(int)(rand2*num);
        while (index2==index1) 
        {
            rand2=(float)(rand())/RAND_MAX; index2=(int)(rand2*num);
        }

        int index3=(int)(rand3*num);
        while ((index3==index1)||(index3==index2)) 
        {
            rand3=(float)(rand())/RAND_MAX; index3=(int)(rand3*num);
        }

        Point3f pt1=pts.at(index1);
        Point3f pt2=pts.at(index2);
        Point3f pt3=pts.at(index3);
            
        // In each RANSAC cycle, a minimal sample with 3 points are formed
            
        vector<Point3f> minimalSample;
            
        minimalSample.push_back(pt1);
        minimalSample.push_back(pt2);
        minimalSample.push_back(pt3);
            
        float* samplePlane = EstimatePlaneOptimal(minimalSample);
            
        // printf("Plane params: %f %f %f %f \n",samplePlane[0],samplePlane[1],samplePlane[2],samplePlane[3]);
            
        // Compute consensus set
            
        RANSACDiffs sampleResult=PlanePointRANSACDifferences(pts, samplePlane, threshold);
            
        // printf("NumInliers: %d \n",sampleResult.inliersNum);
            
        // Check the new test is larger than the best one.
            
        if (sampleResult.inliersNum>bestSampleInlierNum)
        {
            bestSampleInlierNum=sampleResult.inliersNum;
            bestPlane[0]=samplePlane[0];
            bestPlane[1]=samplePlane[1];
            bestPlane[2]=samplePlane[2];
            bestPlane[3]=samplePlane[3];
        }// end if
            
        delete[] samplePlane;
            
    }// end for iter
    
    // Finally, the plane is refitted from the best consensus set
    RANSACDiffs bestResult=PlanePointRANSACDifferences(pts, bestPlane, threshold);   
    
    vector<Point3f> inlierPts;   
    
    for (int idx=0;idx<num;idx++)
    {
        if (bestResult.isInliers.at(idx))
        {
            inlierPts.push_back(pts.at(idx));
        }
    }
    
    float* finalPlane = EstimatePlaneOptimal(inlierPts);

    return finalPlane;
}
    
/* Plane-point differences
 * Goal: This method calculates the plane point differences, and determines if a point is an outlier.
 * Plane is given in implicit form: Ax + By + Cz + D=0 
 * Plane parameters are normalized: A^2 + B^2 + C^2 =1
 * 
 * Input: 
 * vector<Point3f> pts: input points
 * float plane: plane parameters; plane[0]:A, plane[1]:B, plane[2]:C, plane[3]:D
 * float threshold: the threshold for inlier/outlier separation
 * 
 * Output:
 * RANSACDiffs 
 * see header file for details
 * 
 */   
    
RANSACDiffs PlanePointRANSACDifferences(vector<Point3f> pts, float* plane, float threshold)
{
    int num=pts.size();
        
    float A=plane[0];
    float B=plane[1];
    float C=plane[2];
    float D=plane[3];        

    RANSACDiffs ret;
        
    vector<bool> isInliers;
    vector<float> distances;
        
    int inlierCounter=0;
    for (int idx=0; idx<num; idx++)
    {
        Point3f pt=pts.at(idx);
        float diff=fabs(A*pt.x+B*pt.y+C*pt.z+D);
        distances.push_back(diff);
        if (diff<threshold)
        {
            isInliers.push_back(true);
            inlierCounter++;
        }
        else
        {
            isInliers.push_back(false);
        }
            
    }// end for idx;
        
    ret.distances=distances;
    ret.isInliers=isInliers;
    ret.inliersNum=inlierCounter;
        
    return ret;
}

// function that combined the other functions to find the differences
RANSACDiffs findDifferences(vector<Point3f> points, float threshold, int iter)
{
    // Estimate plane parameters without robustification
    float* plane = EstimatePlaneOptimal(points);
    printf("Plane fitting results for the whole data:\nA:%f B:%f C:%f D:%f\n", plane[0], plane[1], plane[2], plane[3]);

    delete[] plane;

    // RANSAC-based robust estimation

    float* planeParams = EstimatePlaneRANSAC(points, threshold, iter);

    printf("Plane params RANSAC:\n A:%f B:%f C:%f D:%f \n", planeParams[0], planeParams[1], planeParams[2], planeParams[3]);

    // Compute differences of the fitted plane in order to separate inliers from outliers

    RANSACDiffs differences = PlanePointRANSACDifferences(points, planeParams, threshold);

    delete[] planeParams;

    // Inliers and outliers are coloured by green and red, respectively

    return differences;
}