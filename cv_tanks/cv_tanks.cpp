#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int height = {800};
int width = {800};
int x = {(width / 2) - 26};
int y = {(height / 2) - 26};
bool missile_flag = {0};
int x_missile = {};
int y_missile = {};
char key = {}, last_key = {};
double angle = {180.0};
char roat {'e'};
Mat tank_rotation;
int cnt = {0};


void rotation(Mat &image, double ang)
{
        Point2f center(26.0, 26.0);
        Mat rotation_matrix = getRotationMatrix2D(center, ang, 1.0);
        warpAffine(image, tank_rotation, rotation_matrix, Size(51, 51));
}


void missile(char l_key, Mat &missile_lay)
{
	roat = l_key;
	switch (l_key)
	{
		case 'w':
		case 'e':
			rectangle(
				missile_lay, 
				Point(x + 20, y - 30),
				Point(x + 20 + 11, y),
				Scalar(255, 255, 255),
				FILLED
				);
			break;
		case 'a':
                        rectangle(
                	        missile_lay, 
                                Point(x - 30, y + 20),
				Point(x, y + 20 + 11),
                                Scalar(255, 255, 255),
				FILLED
                        	);
			break;
		case 's':
                        rectangle(
                                missile_lay, 
                                Point(x + 20, y + 51),
                                Point(x + 20 + 11, y + 51 + 30),
                                Scalar(255, 255, 255),
                                FILLED
                        	);
			break;
		case 'd':
                        rectangle(
                                missile_lay, 
                                Point(x + 51, y + 20),
                                Point(x + 51 + 30, y + 20 + 11),
                                Scalar(255, 255, 255),
                                FILLED
                        	);                	
			break;
		defalt:
			break;
	}
}

int main(int args, char** argv)
{

	Mat background(
			height,
		       	width,
			CV_8UC3, 
			Scalar(0, 180, 0)
			);

	Mat tank_layer(
	      		height,
                        width,
                        CV_8UC3, 
                        Scalar(0, 0, 0)
                        );

        Mat missile_layer(
                        height,
                        width,
                        CV_8UC3,
                        Scalar(0, 0, 0)
                        );


        Mat tank = imread("tank.png");
        resize(tank, tank, Size(51, 51));

	rotation(tank, angle);
	
	while(key != 'q')
	{

		key = waitKey(0);
                background.copyTo(tank_layer);

		switch (key)
        	{
                	case 'w':
				rotation(tank, 180.0);			
                        	y -= 1;
	                        break;
        	        case 'a':
                                rotation(tank, 270.0);
				x -= 1;
        	                break;
        	        case 's':
				rotation(tank, 0.0);
				y += 1;
        	               	break;
        	        case 'd':
                                rotation(tank, 90.0);
				x += 1;
        	                break;
        	        case 'e':
                                rotation(tank, 180.0);
				x = (width / 2) - 26;
				y = (height / 2) - 26;
              	  	        break;
			case ' ':
				if (missile_flag == 0) {	
					missile_flag = 1;
					x_missile = x;
					y_missile = y;
					missile(last_key, background);
				}
				break;
             		default:
             	        	break;  
        	}

		if ((x == 0) || (x == width - 51) || (y == 0) || (y == height - 51)) {
			rotation(tank, 180.0);
			x = (width / 2) - 26;
 			y = (height / 2) - 26;
		}

		Mat position = tank_layer(Rect(x, y, 51, 51));
	        
                tank_rotation.copyTo(position);
		
		if (missile_flag == 1) {
			switch (roat)
			{
				case 'w':
				{
					Mat back_missile = background(Rect(x_missile + 20, 0, 12, 800));
					dilate(back_missile, back_missile, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(1, 0), 2);
					erode(back_missile, back_missile, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(1, 0), 2);
					cnt++;
					
					if (y_missile < (cnt * 4)) 
					{
						missile_flag = 0;
						cnt = 0;
					}
					break;
				}
				case 'a':
                                {
                                        Mat back_missile = background(Rect(0, y_missile + 20, 800, 12));
                                        dilate(back_missile, back_missile, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(0, 1), 2);
                                        erode(back_missile, back_missile, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(0, 1), 2);
                                        cnt++;

                                        if (x_missile < (cnt * 4))
                                        {
                                                missile_flag = 0;
                                                cnt = 0;
                                        }
                                        break;
                                }
				case 's':
                                {
                                        Mat back_missile = background(Rect(x_missile + 20, 0, 12, 800));
                                        dilate(back_missile, back_missile, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(1, 2), 2);
                                        erode(back_missile, back_missile, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(1, 2), 2);
                                        cnt++;

                                        if ((800 - y_missile) < (cnt * 4))
                                        {
                                                missile_flag = 0;
                                                cnt = 0;
                                        }
                                        break;
                                }
				case 'd':
				{
					Mat back_missile = background(Rect(0, y_missile + 20, 800, 12));
                                        dilate(back_missile, back_missile, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(2, 1), 2);
                                        erode(back_missile, back_missile, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(2, 1), 2);
                                        cnt++;

                                        if ((800 - x_missile) < (cnt * 3))
                                        {
                                                missile_flag = 0;
                                                cnt = 0;
                                        }
                                        break;
                                }

				default:
					break;
			}
			
		}

		
		last_key = key;

		imshow("tank", tank_layer);

	}

	destroyWindow("tank");
}
