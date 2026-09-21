#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void search_center(Mat &img, int limit_low, int limit_high, string name_img)
{
	Mat hsv_img;
	cvtColor(img, hsv_img, COLOR_BGR2HSV);

	Mat mask, mask1, mask2;

	inRange(hsv_img, Scalar(0, 0, 100), Scalar(25, 255, 255), mask1);
        inRange(hsv_img, Scalar(160, 0, 100), Scalar(180, 255, 255), mask2);
	bitwise_or(mask1, mask2, mask);

	erode(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	dilate(mask, mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
	Mat all_contur;

        dilate(mask, all_contur, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 3);
        
	for (int x = 0; x < all_contur.cols; x++)
        {
                for (int y = 0; y < all_contur.rows; y++)
                {
                       if ((int)all_contur.at<uchar>(y, x) == (int)mask.at<uchar>(y, x)) all_contur.at<uchar>(y, x) = 0;
		}
        }
        
	vector<vector<Point>> contours;
	findContours(all_contur, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	int max_area = 0;
	int max_contour = 0;
	
	if (contours.size() == 0) {
		cout << "Контуров нет" << endl;
		
		destroyAllWindows();
	}

	if (contours.size() > 1) {
		for (int i = 0; i < contours.size(); i++)
		{
			if (contourArea(contours[i]) > max_area) {
				max_area = contourArea(contours[i]);
				max_contour = i;
			}
		}
	}

	Moments mnts = moments(contours[max_contour]);

	double x_cnt = mnts.m10 / mnts.m00;
	double y_cnt = mnts.m01 / mnts.m00;
	
	line(img, Point(x_cnt, y_cnt + 5), Point(x_cnt, y_cnt - 5), Scalar(0, 0, 0), 1);
	line(img, Point(x_cnt + 5, y_cnt), Point(x_cnt - 5, y_cnt), Scalar(0, 0, 0), 1);

        drawContours(img, contours, max_contour, Scalar(0, 0, 0), 1);

	imshow(name_img, img);
}

		
int main(int args, char** argv)	
{
	string name = "img1.jpg";
	Mat image = imread(name);
	search_center(image, 215, 255, name);

        name = "img2.jpg";
        Mat image1 = imread(name);
        search_center(image1, 215, 255, name);

        name = "img3.jpg";
        Mat image2 = imread(name);
        search_center(image2, 215, 255, name);
        
	name = "img4.png";
        Mat image4 = imread(name);
        search_center(image4, 215, 255, name);
        
	name = "img5.jpg";
        Mat image5 = imread(name);
        search_center(image5, 215, 255, name);

	waitKey(0);
	destroyAllWindows();
}
