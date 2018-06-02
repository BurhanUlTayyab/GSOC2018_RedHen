#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>


using namespace cv;
using namespace std;

int threshold_value = 70;
int threshold_type = 0;
int const max_BINARY_value = 255;
RNG rng(12345);
int t = 0;


int main(){
    string filename = "file.avi";
    VideoCapture capture(filename);
    Mat frame;


    int morph_size = 2;
    Mat element = getStructuringElement( MORPH_RECT, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );



    if(!capture.isOpened()){
     throw "Error when reading steam_avi";
    }

    namedWindow( "w", 1);
    for( ; ; ){
        capture >> frame;
        Mat tframe = frame;
        rectangle(tframe, Point(100, 525),Point(715, 550),Scalar(255),2,8,0);

        //Hardcoding Parameters
        int offset_x = 100;
        int offset_y = 525;

        //Cropping
        cv::Rect roi;
        roi.x = offset_x;
        roi.y = offset_y;
        roi.width = 615;
        roi.height = 25;
        cv::Mat crop = frame(roi);
        cv::imshow("crop", crop);

        //Detecting Blue Areas for Ticker Stopping Point
        Mat hsv, thresholdx,res;
        cvtColor(crop, hsv, CV_BGR2HSV);
        inRange(hsv, Scalar(110,50,50), Scalar(130,255,255), thresholdx);
        bitwise_and(crop,crop,res,thresholdx);


        //Gray Areas for Stopping Points
        Mat gray,dst, tHold;
        cvtColor(res, gray, CV_BGR2GRAY);
        threshold(gray, gray, 70, 255, CV_THRESH_BINARY);
        morphologyEx( gray, dst, MORPH_CLOSE, element, Point(-1,-1) );   


        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours( dst, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

        Mat drawing = Mat::zeros( dst.size(), CV_8UC3 );
        t = t + 1;
        for( int i = 0; i< contours.size(); i++ ){
         Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
         if (contourArea(contours[i]), arcLength( contours[i], true )< 300 && contourArea(contours[i]), arcLength( contours[i], true )> 150) {
         drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
         ostringstream convert;
         convert << "image" << t << ".jpg";
         imwrite(convert.str().c_str(), crop );
         }
}
        imshow( "Contours", drawing );






        if(frame.empty())
            break;
        imshow("w", gray);
        waitKey(20); // waits to display frame
    }
    waitKey(0); // key press to close window
    // releases and window destroy are automatic in C++ interface
}
