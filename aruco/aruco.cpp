#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

struct MarkerInfo {
    int id;
    vector<Point2f> corners; //углы маркера
    Point2f center;
    Vec3d rvec; //поворот
    Vec3d tvec; //перемещение
    float size;
};


Point2f search_center(const vector<Point2f>& corners) {
    Point2f center(0, 0);
    
    for(const auto& corner : corners) {
        center += corner;
    }
    
    center /= 4.0;
    
    return center;
}


double search_distance(const MarkerInfo& marker1, const MarkerInfo& marker2) {
    Vec3d diff = marker2.tvec - marker1.tvec;
    return sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
}


Point2f projectPoint(const Vec3d& point3d, const Mat& camMatrix, const Mat& distCoeffs, const Vec3d& rvec, const Vec3d& tvec) 
{
    vector<Point3f> points3d = {Point3f(point3d[0], point3d[1], point3d[2])};
    vector<Point2f> points2d;
    projectPoints(points3d, rvec, tvec, camMatrix, distCoeffs, points2d);
    return points2d[0];
}

int main(int argc, char** argv) {

    float bigMarkerSize = 0.080f;
    float smallMarkerSize = 0.040f;
    
    int bigMarkerId = 63;
    int smallMarkerId = 64;
    
    Mat camMatrix, distCoeffs;
    FileStorage fs("calibration_params.yml", FileStorage::READ);
    
    fs["camera_matrix"] >> camMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    
    fs.release();
    
    Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(10)); //словарь
    
    Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();
    detectorParams->cornerRefinementMethod = aruco::CORNER_REFINE_SUBPIX;
    
    VideoCapture cap(0);
    
    Mat frame;
    
    while(true) {
        cap >> frame;
        if(frame.empty()) break;
        
        Mat frameCopy = frame.clone();
        
        vector<int> ids;
        vector<vector<Point2f>> corners, rejected;
        vector<Vec3d> rvecs, tvecs;
        
        aruco::detectMarkers(frame, dictionary, corners, ids, detectorParams, rejected);
        
        if(ids.size() >= 2) {
            vector<MarkerInfo> markers;
            
            for(size_t i = 0; i < ids.size(); i++) {
                float markerSize;
                
                if(ids[i] == bigMarkerId) {
                    markerSize = bigMarkerSize;
                } 
                else {
                    markerSize = smallMarkerSize;
                }
                
                MarkerInfo info;
                info.id = ids[i];
                info.corners = corners[i];
                info.center = search_center(corners[i]);
                info.size = markerSize;
                markers.push_back(info);
            }
            
            for(size_t i = 0; i < markers.size(); i++) {
                vector<vector<Point2f>> singleCorner = {markers[i].corners};
                vector<Vec3d> singleRvec, singleTvec;
                
                aruco::estimatePoseSingleMarkers(singleCorner, markers[i].size, camMatrix, distCoeffs, singleRvec, singleTvec);
                
                if(!singleRvec.empty()) {
                    markers[i].rvec = singleRvec[0];
                    markers[i].tvec = singleTvec[0];
                }
            }
            

            MarkerInfo* bigMarker = nullptr;
            MarkerInfo* smallMarker = nullptr;
            
            for(auto& marker : markers) {
            
                if(marker.id == bigMarkerId) {
                    bigMarker = &marker;
                } else if(marker.id == smallMarkerId) {
                    smallMarker = &marker;
                }
            }
            
            if(bigMarker != nullptr && smallMarker != nullptr) {
            
                aruco::drawDetectedMarkers(frameCopy, corners, ids); //контуры маркеров

                float axisLength = bigMarker->size / 1.5f;

                Vec3d xPoint(axisLength, 0, 0);
                Vec3d yPoint(0, axisLength, 0);

                Point2f xEnd = projectPoint(xPoint, camMatrix, distCoeffs, bigMarker->rvec, bigMarker->tvec);
                Point2f yEnd = projectPoint(yPoint, camMatrix, distCoeffs, bigMarker->rvec, bigMarker->tvec);

                arrowedLine(frameCopy, bigMarker->center, xEnd, Scalar(0, 0, 255), 3, 8, 0, 0.2);

                arrowedLine(frameCopy, bigMarker->center, yEnd, Scalar(0, 255, 0), 3, 8, 0, 0.2);

                putText(frameCopy, "X", xEnd, FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
                putText(frameCopy, "Y", yEnd, FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);

                line(frameCopy, bigMarker->center, smallMarker->center, Scalar(0, 255, 255), 3);

                Vec3d vecToSmall = smallMarker->tvec - bigMarker->tvec; //вектор между маркерами
                
                Mat rmat;
                Rodrigues(bigMarker->rvec, rmat); //вектор поворота в матрицу
                
                Mat vecToSmallLocal = rmat.t() * Mat(vecToSmall); //локальные координаты
                
                double localX = vecToSmallLocal.at<double>(0);
                double localY = vecToSmallLocal.at<double>(1);
                double localZ = vecToSmallLocal.at<double>(2);
                
                double localX_mm = localX * 1000;
                double localY_mm = localY * 1000;
                
                double realDistance = search_distance(*bigMarker, *smallMarker);
                double distance_mm = realDistance * 1000;
                
                string infoText = format("X: %.1f", localX_mm);
                string infoText2 = format("Y: %.1f", localY_mm);
                string infoText3 = format("D: %.1f", distance_mm);
                
                putText(frameCopy, infoText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
                putText(frameCopy, infoText2, Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 0), 2);
                putText(frameCopy, infoText3, Point(10, 90), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 2);
                
                //отрисовка проекций
                
                Vec3d xProjLocal(localX, 0, 0);
                Vec3d yProjLocal(0, localY, 0);
                
                Mat xProjCam = rmat * Mat(xProjLocal);
                Mat yProjCam = rmat * Mat(yProjLocal);
                
                Vec3d xProjVec(xProjCam.at<double>(0), xProjCam.at<double>(1), xProjCam.at<double>(2));
                Vec3d yProjVec(yProjCam.at<double>(0), yProjCam.at<double>(1), yProjCam.at<double>(2));
                
                Vec3d xProjPoint = bigMarker->tvec + xProjVec;
                Vec3d yProjPoint = bigMarker->tvec + yProjVec;
                
                vector<Point3f> points3d;
                points3d.push_back(Point3f(xProjPoint[0], xProjPoint[1], xProjPoint[2]));
                points3d.push_back(Point3f(yProjPoint[0], yProjPoint[1], yProjPoint[2]));
                
                vector<Point2f> points2d;
                projectPoints(points3d, Vec3d(0,0,0), Vec3d(0,0,0), 
                            camMatrix, distCoeffs, points2d);
                
                if(points2d.size() >= 2) {
                    line(frameCopy, points2d[0], smallMarker->center, Scalar(0, 0, 255), 2);
                    line(frameCopy, points2d[1], smallMarker->center, Scalar(0, 255, 0), 2);
                }
            }
        }
        
        imshow("Аруко", frameCopy);
        
        char key = (char)waitKey(1);
        if(key == 27) break;
    }
    
    cap.release();
    destroyAllWindows();
    
    return 0;
}
