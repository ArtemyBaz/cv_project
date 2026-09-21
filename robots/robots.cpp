#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace cv;
using namespace std;

Mat search_contour(Mat &mask, Mat &img, Scalar color, int x, int y)
{
	Mat resault = img.clone();
        erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 2);
        dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 2);
        Mat all_contour;

        dilate(mask, all_contour, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);

        for (int x = 0; x < all_contour.cols; x++)
        {
                for (int y = 0; y < all_contour.rows; y++)
                {
                       if ((int)all_contour.at<uchar>(y, x) == (int)mask.at<uchar>(y, x)) all_contour.at<uchar>(y, x) = 0;
                }
        }
        
	vector<vector<Point>> contours;
        findContours(all_contour, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

        if (contours.size() == 0) {
                cout << "Контуров нет" << endl;
                destroyAllWindows();
        }	
	
	vector<vector<float>> radius(contours.size());
	vector<cv::Point2f> centers;
	vector<float> radii;
	
	for (int i = 0; i < contours.size() ; i++)
	{
		Point2f center;
    		float radius;
		minEnclosingCircle(contours[i], center, radius);		
		centers.push_back(center);
        	radii.push_back(radius);
	}
	
	int min_x = 1000, min_y = 1000;

        for (int j = 0; j < contours.size(); j++)
        {
                if ((contourArea(contours[j]) * 1.8 > (radii[j] * radii[j] * 3.14)) && (contourArea(contours[j]) > 100)) {
			drawContours(resault, contours, j, color, 2);	
			if (sqrt((x - min_x) * (x - min_x) + (y - min_y) * (y - min_y)) > sqrt((x - (int)centers[j].x) * (x - (int)centers[j].x) + (y - (int)centers[j].y) * (y - (int)centers[j].y)))
			{
				min_x = centers[j].x;
				min_y = centers[j].y;
			}
			circle(resault, centers[j], 2, color, -1);
		}
        }
	
	line(resault, Point(x, y), Point(min_x, min_y), color, 2);

	return resault;
}


void search_center(Mat &img, string name_img)
{
	Mat s_img, res_img;
	cvtColor(img, s_img, COLOR_BGR2HSV);
	Mat lamp_mask, r1_mask, r2_mask, red_mask, blue_mask, green_mask;
        
	inRange(s_img, Scalar(0, 0, 250), Scalar(180, 8, 255), lamp_mask);
	inRange(s_img, Scalar(90, 80, 140), Scalar(140, 255, 255), blue_mask);
        inRange(s_img, Scalar(40, 55, 141), Scalar(80, 255, 255), green_mask);
        inRange(s_img, Scalar(0, 80, 140), Scalar(30, 240, 255), r1_mask);
        inRange(s_img, Scalar(160, 80, 140), Scalar(180, 240, 255), r2_mask);
	bitwise_or(r1_mask, r2_mask, red_mask);
	
        erode(lamp_mask, lamp_mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 2);
        dilate(lamp_mask, lamp_mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 2);

	Mat all_contour;

        dilate(lamp_mask, all_contour, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);

        for (int x = 0; x < all_contour.cols; x++)
        {
                for (int y = 0; y < all_contour.rows; y++)
                {
                       if ((int)all_contour.at<uchar>(y, x) == (int)lamp_mask.at<uchar>(y, x)) all_contour.at<uchar>(y, x) = 0;
                }
        }

        vector<vector<Point>> lamp;
        findContours(all_contour, lamp, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	
	Moments mnts = moments(lamp[0]);

	double x_cnt = mnts.m10 / mnts.m00;
	double y_cnt = mnts.m01 / mnts.m00;
	
	drawContours(img, lamp, 0, Scalar(0, 255, 255), 2);

	res_img = search_contour(blue_mask, img, Scalar(255, 0, 0), x_cnt, y_cnt);
	res_img = search_contour(green_mask, res_img, Scalar(0, 255, 0), x_cnt, y_cnt);
	res_img = search_contour(red_mask, res_img, Scalar(0, 0, 255), x_cnt, y_cnt);
	
	circle(res_img, Point(x_cnt, y_cnt), 4, Scalar(0, 255, 255), -1);

        imshow(name_img, res_img);	
}

		
int main(int args, char** argv)	
{
	string name = "roi_robotov.jpg";
	Mat image = imread(name);
	search_center(image, name);

        name = "roi_robotov_1.jpg";
        Mat image1 = imread(name);
        search_center(image1, name);
	
	VideoCapture cap("video.mp4");

	Mat frame;
	
	while(1) {
		cap >> frame;
		if (frame.empty()) break;
		search_center(frame, "roboti");
		if (waitKey(25) == 27) break;
	}

	destroyAllWindows();
}
