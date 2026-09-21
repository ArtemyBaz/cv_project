#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace cv;

int x_1 = {};

void robot(Mat& img, int x_cen, int y_cen)
{
	circle(
		img, 
		Point(x_cen, y_cen), 
		8, 
		Scalar(255, 0, 0), 
		FILLED
	);

	for (int i = -1; i < 2; i+=2)
	{
		for (int j = -1; j < 2; j+=2)
		{
			circle(
				img, 
				Point(x_cen + i * 8, y_cen + j * 8), 
				5, 
				Scalar(255, 0, 0), 
				FILLED
			);
		}
	}
}

double y(int x, int offset)
{
	double fr = {0.05};
	double ampl = {150};

	return  ampl * sin(fr * x) + (offset / 2);
}

		
int main(int args, char** argv)
{
	Mat image = imread("image.jpg");
	
	if (image.empty())
	{
		std::cout << "Проблемы!" << std::endl;
		std::cin.get();
		return -1;
	}
	
	int width = image.cols;
	int height = image.rows;

	Mat image2(width, height, CV_8UC1);

	for (x_1 = 0; x_1 < width; x_1++)
	{
		image.copyTo(image2);
		
		for (int x = 0; x < x_1; x++)
		{	
			line(
				image2,
				Point(x, y(x, height)),
			       	Point(x + 1, y((x + 1), height)),
			       	Scalar(0, 0, 255), 
				2
			);
		}

		robot(image2, x_1, y(x_1, height));
		
		namedWindow("image_first");
		imshow("image_first", image2);
		
		if (x_1 == (width / 2)) imwrite("image2.jpg", image2);

		waitKey(10);
	}

	waitKey(0);
}
