#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <algorithm>
#include <vector>

using namespace std;
using namespace cv;


bool A(Mat& img, vector<Point> pxl)
{
	int sum = 0;
	int white_pxls = 0;
	
	if ((img.at<uchar>(pxl[9]) == 0) && (img.at<uchar>(pxl[2]) != 0)) sum ++;
	if (img.at<uchar>(pxl[9]) != 0) white_pxls ++;
	
	for (int indx {2}; indx < 9; indx++)
	{
		if (img.at<uchar>(pxl[indx]) != 0) white_pxls ++;
		if ((img.at<uchar>(pxl[indx]) == 0) && (img.at<uchar>(pxl[indx + 1]) != 0)) sum ++;		
		if (sum > 1) return 0;
	}
	
	if (((white_pxls >= 2) && (white_pxls <= 6)) && sum == 1) return 1;
	else return 0;
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
            vector<Point> pixel = {
                {0, 0},
                {x, y},
                {x, y - 1},
                {x + 1, y - 1},
                {x + 1, y},
                {x + 1, y + 1},
                {x, y + 1},
                {x - 1, y + 1},
                {x - 1, y},
                {x - 1, y - 1}
            };

            if (bin_img.at<uchar>(pixel[1]) == 0) continue;
            if (A(bin_img, pixel) == 0) continue;
            if ((bin_img.at<uchar>(pixel[2]) != 0) && (bin_img.at<uchar>(pixel[4]) != 0) && (bin_img.at<uchar>(pixel[6]) != 0)) continue;
            if ((bin_img.at<uchar>(pixel[4]) != 0) && (bin_img.at<uchar>(pixel[6]) != 0) && (bin_img.at<uchar>(pixel[8]) != 0)) continue;

            result.at<uchar>(pixel[1]) = 0;
            changed = true;
        }
    }

    bin_img = result.clone();

    for (int x {1}; x < width_img - 1; x++)
    {
        for (int y {1}; y < height_img - 1; y++)
        {
            vector<Point> pixel = {
                {0, 0},
                {x, y},
                {x, y - 1},
                {x + 1, y - 1},
                {x + 1, y},
                {x + 1, y + 1},
                {x, y + 1},
                {x - 1, y + 1},
                {x - 1, y},
                {x - 1, y - 1}
            };

            if (bin_img.at<uchar>(pixel[1]) == 0) continue;
            if (A(bin_img, pixel) == 0) continue;
            if ((bin_img.at<uchar>(pixel[2]) != 0) && (bin_img.at<uchar>(pixel[4]) != 0) && (bin_img.at<uchar>(pixel[8]) != 0)) continue;
            if ((bin_img.at<uchar>(pixel[2]) != 0) && (bin_img.at<uchar>(pixel[6]) != 0) && (bin_img.at<uchar>(pixel[8]) != 0)) continue;

            result.at<uchar>(pixel[1]) = 0;
            changed = true;
        }
    }

    return {result, changed};
}


void clean_board(Mat& img_mask)
{
	int width_img = img_mask.cols;
	int height_img = img_mask.rows;

	for (int x {0}; x < width_img; x++)
	{
		img_mask.at<uchar>(0, x) = 0;
		img_mask.at<uchar>(height_img - 1, x) = 0;
	}
	
	for (int y {0}; y < height_img ; y++)
	{
		img_mask.at<uchar>(y, 0) = 0;
		img_mask.at<uchar>(y, width_img - 1) = 0;
	}
}


void drawConnectingLines(Mat& image, const vector<Vec4i>& lines, double maxDistance)
{
    struct EndPoint {
        Point pt;
        int lineIdx;
        bool isStart;
    };

    vector<EndPoint> endPoints;
    for (size_t i = 0; i < lines.size(); ++i) {
        const Vec4i& line = lines[i];
        endPoints.push_back({cv::Point(line[0], line[1]), static_cast<int>(i), true});
        endPoints.push_back({cv::Point(line[2], line[3]), static_cast<int>(i), false});
    }

    if (endPoints.size() < 2) return;

    // Верхняя и нижняя точки
    int idxTop = 0, idxBottom = 0;
    int minY = endPoints[0].pt.y;
    int maxY = endPoints[0].pt.y;

    for (size_t i = 1; i < endPoints.size(); ++i) {
        int y = endPoints[i].pt.y;
        if (y < minY) {
            minY = y;
            idxTop = i;
        }
        if (y > maxY) {
            maxY = y;
            idxBottom = i;
        }
    }

    // Точки для соединения
    std::vector<int> availablePoints;
    for (size_t i = 0; i < endPoints.size(); ++i) {
        if (i != idxTop && i != idxBottom) {
            availablePoints.push_back(i);
        }
    }

    while (availablePoints.size() >= 2) {
        double minDist = maxDistance + 1;
        int bestI = -1, bestJ = -1;
        
        // Самая близкая точка
        for (size_t ii = 0; ii < availablePoints.size(); ++ii) {
            for (size_t jj = ii + 1; jj < availablePoints.size(); ++jj) {
                int i = availablePoints[ii];
                int j = availablePoints[jj];
                
                if (endPoints[i].lineIdx == endPoints[j].lineIdx) continue;
                
                double dist = cv::norm(endPoints[i].pt - endPoints[j].pt);
                if (dist < minDist) {
                    minDist = dist;
                    bestI = i;
                    bestJ = j;
                }
            }
        }
        
        if (bestI != -1 && bestJ != -1 && minDist <= maxDistance) {
            line(image, endPoints[bestI].pt, endPoints[bestJ].pt, Scalar(0, 0, 255), 2);
            
            // Удаление соединённых точек из доступных
            availablePoints.erase(remove(availablePoints.begin(), availablePoints.end(), bestI), availablePoints.end());
            availablePoints.erase(remove(availablePoints.begin(), availablePoints.end(), bestJ), availablePoints.end());
        } else {
            break;
        }
    }
    
    for (int idx : availablePoints) {
        cv::circle(image, endPoints[idx].pt, 2, cv::Scalar(0, 0, 255), -1);
    }
}

int main(int args, char** argv)
{
    VideoCapture cap("0.avi"); 

    Mat frame, gray, mask, mask1, diff;
    //Mat cls_mask;
    
    vector<Vec4i> lines;
    vector<Point> points;
    
    int level = 70;

    while (true)
    {
        cap >> frame; 
        
        if (frame.empty()) break;
        
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        
        //if (cls_mask.empty()) {
            //int height = gray.rows;
            //int width = gray.cols;
            
            //cls_mask = Mat::zeros(gray.size(), CV_8UC1);
            //rectangle(cls_mask, Point(0, height - 200), Point(width - 1, height - 1), Scalar(255), FILLED);
        //}
	

	//Scalar mean_val = mean(gray, cls_mask);
	
	//cout << "Средняя интенсивность в нижних 200 пикселях: " << mean_val[0] << endl;
	
        inRange(gray, Scalar(level), Scalar(255), mask);
        
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	erode(mask, mask, kernel, Point(-1,-1));
        //imshow("Исходник1", frame);
        //imshow("Бинаризация", mask);
        

        while (true)
        {
            auto [mask1, changed] = skelet(mask);
            mask = mask1;
            if (!changed) break;
        }

        clean_board(mask);
        //imshow("Скелетизация", mask);

        HoughLinesP(mask, lines, 1, CV_PI / 180, 50, 5, 10);


        for (size_t i = 0; i < lines.size(); i++)
        {
            Vec4i l = lines[i];
            line(frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
        }

        drawConnectingLines(frame, lines, 300.0);

        imshow("Результат", frame);

        char key = waitKey(1);
        if (key == 'q') break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
