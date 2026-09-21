#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <opencv2/ximgproc.hpp>

using namespace cv;
using namespace std;


static const vector<Point> offsets = {
    {0, 0}, {0, 0}, {0, -1}, {1, -1}, {1, 0},
    {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}
};

bool A(Mat& img, int x, int y, const vector<Point>& offsets)
{
    int sum = 0;
    int white_pxls = 0;
    
    Point p9(x + offsets[9].x, y + offsets[9].y);
    Point p2(x + offsets[2].x, y + offsets[2].y);
    
    if ((img.at<uchar>(p9) == 0) && (img.at<uchar>(p2) != 0)) sum++;
    if (img.at<uchar>(p9) != 0) white_pxls++;
    
    for (int indx = 2; indx < 9; indx++)
    {
        Point p_indx(x + offsets[indx].x, y + offsets[indx].y);
        Point p_next(x + offsets[indx + 1].x, y + offsets[indx + 1].y);
        
        if (img.at<uchar>(p_indx) != 0) white_pxls++;
        if ((img.at<uchar>(p_indx) == 0) && (img.at<uchar>(p_next) != 0)) sum++;
        if (sum > 1) return 0;
    }
    
    return ((white_pxls >= 2) && (white_pxls <= 6) && sum == 1);
}


std::pair<Mat, bool> skelet(Mat& binary_img)
{
    Mat bin_img = binary_img.clone();
    Mat result = bin_img.clone();
    bool changed = false;
    int width_img = bin_img.cols;
    int height_img = bin_img.rows;

    for (int x {1}; x < width_img - 1; x++)
    {
        for (int y {1}; y < height_img - 1; y++)
        {
            Point center(x, y);
            
            if (bin_img.at<uchar>(center) == 0) continue;
            if (A(bin_img, x, y, offsets) == 0) continue;
            if ((bin_img.at<uchar>(x + offsets[2].x, y + offsets[2].y) != 0) && 
                (bin_img.at<uchar>(x + offsets[4].x, y + offsets[4].y) != 0) && 
                (bin_img.at<uchar>(x + offsets[6].x, y + offsets[6].y) != 0)) continue;
            if ((bin_img.at<uchar>(x + offsets[4].x, y + offsets[4].y) != 0) && 
                (bin_img.at<uchar>(x + offsets[6].x, y + offsets[6].y) != 0) && 
                (bin_img.at<uchar>(x + offsets[8].x, y + offsets[8].y) != 0)) continue;

            result.at<uchar>(center) = 0;
            changed = true;
        }
    }

    bin_img = result.clone();

    for (int x {1}; x < width_img - 1; x++)
    {
        for (int y {1}; y < height_img - 1; y++)
        {
            Point center(x, y);
        
            if (bin_img.at<uchar>(center) == 0) continue;
            if (A(bin_img, x, y, offsets) == 0) continue;
            if ((bin_img.at<uchar>(x + offsets[2].x, y + offsets[2].y) != 0) && 
                (bin_img.at<uchar>(x + offsets[4].x, y + offsets[4].y) != 0) && 
                (bin_img.at<uchar>(x + offsets[8].x, y + offsets[8].y) != 0)) continue;
            if ((bin_img.at<uchar>(x + offsets[2].x, y + offsets[2].y) != 0) && 
                (bin_img.at<uchar>(x + offsets[6].x, y + offsets[6].y) != 0) && 
                (bin_img.at<uchar>(x + offsets[8].x, y + offsets[8].y) != 0)) continue;

            result.at<uchar>(center) = 0;
            changed = true;
        }
    }

    return {result, changed};
}



void search_dist(const vector<Vec4i>& lines, double fx, double fy, double cx, double cy, double Y, int image_width, int image_height)
{
    Mat grid_image = Mat::zeros(image_height, image_width, CV_8UC3);
    
    for (int x = 0; x < image_width; x += 10)
    {
        line(grid_image, Point(x, 0), Point(x, image_height - 1), Scalar(255, 255, 255), 1);
    }
    
    for (int y = 0; y < image_height; y += 10)
    {
        line(grid_image, Point(0, y), Point(image_width - 1, y), Scalar(255, 255, 255), 1);
    }
    
    for (size_t i = 0; i < lines.size(); i++)
    {
        Vec4i line = lines[i];
        int x1 = line[0];
        int y1 = line[1];
        int x2 = line[2];
        int y2 = line[3];
        
        double Z1 = (fy * Y) / (y1 - cy);
        double X1 = ((x1 - cx) * Z1) / fx;
        
        double Z2 = (fy * Y) / (y2 - cy);
        double X2 = ((x2 - cx) * Z2) / fx;
        
        //cout << "Линия " << i << ":\n";
        //cout << "Точка 1: X = " << X1 << ", Z = " << Z1 << "\n";
        //cout << "Точка 2: X = " << X2 << ", Z = " << Z2 << "\n\n";
        
        circle(grid_image, Point(static_cast<int>((X1 / 10) + cx), static_cast<int>(Z1 / 10)), 3, Scalar(0, 255, 0), -1);
        circle(grid_image, Point(static_cast<int>((X2 / 10) + cx), static_cast<int>(Z2 / 10)), 3, Scalar(0, 255, 0), -1);
        cv::line(grid_image, Point((X1 / 10) + cx, Z1 / 10), Point((X2 / 10) + cx, Z2 / 10), Scalar(0, 255, 0), 3); 
    }
    
    imshow("Карта", grid_image);
}


int main(int args, char** argv)
{
    VideoCapture cap("calib_1.avi"); 

    Mat frame, hsv_frame, mask, mask1;
    
    cap >> frame;
    
    int img_width = frame.cols;
    int img_height = frame.rows;
    double cx = img_width / 2;
    double cy = img_height / 2;
    
    vector<Vec4i> lines;
    vector<Point> points;

    while (true)
    {
        cap >> frame; 
        
        if (frame.empty()) break;
        
        cvtColor(frame, hsv_frame, COLOR_BGR2HSV);
	
        inRange(hsv_frame, Scalar(0, 0, 130), Scalar(185, 90, 255), mask);
        
	while (true)
        {
            auto [mask1, changed] = skelet(mask);
            mask = mask1;
            if (!changed) break;
        }
	
        HoughLinesP(mask, lines, 1, CV_PI / 180, 10, 3, 3);
        
        imshow("Бинаризация", mask);

        for (size_t i = 0; i < lines.size(); i++)
        {
            Vec4i l = lines[i];
            line(frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1, LINE_AA);
        }
	search_dist(lines, 249.45, 249.45, cx, cy, 250, img_width, img_height);
	
        imshow("Результат", frame);
	
        char key = waitKey(30);
        if (key == 'q') break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
