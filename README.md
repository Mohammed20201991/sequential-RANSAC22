# Squential-RANSAC22- Algorthim Implementation . 
<h3> Robust plane detection on LiDAR point clouds by SeqRANSAC </h3>
<p>Implement the Sequential RANSAC (SeqRANSAC) algorithm with optimal plane detection. The method for optimal plane detection </p> <br>
<h5> Sequential RANSAC is the iterative modification of RANSAC: it finds a plane by RANSAC, then the plane points are removed from the dataset, and RANSAC method is run again on the rest of the points. The plane detection and removal can be iterated many times </h5> <br>
Task is to :  <br>
- Find the three most dominant planes by SeqRANSAC . <br>
- Tuning the RANSAC parameter to find good planes . <br>
- Run the method on all the data (except plane.xyz) given in this ZIP file(data is private). <br>
- Results in PLY format (coloured point clouds).<br>
- The  three most dominant plane  be found her.<br>
