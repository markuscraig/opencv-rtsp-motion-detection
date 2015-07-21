//
//  main.cpp
//  OpenCV IP Video Cam
//
//  Created by Mark Craig on 7/19/15.
//  Copyright (c) 2015 Mark Craig. All rights reserved.
//

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main(int argc, const char * argv[]) {
    std::cout << "Starting...\n";
    
    /*
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;
    
    Mat edges;
    namedWindow("edges",1);
    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, CV_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 0, 30, 3);
        imshow("edges", edges);
        if(waitKey(30) >= 0) break;
    }
     
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
    */
    
    
    cv::namedWindow("Annotated Frame");
    cv::namedWindow("Contour Delta");
    //cv::namedWindow("Edges");
    
    cv::VideoCapture capture("rtsp://admin:Appo7ite@192.168.0.201:554/Streaming/Channels/1");

    int frameIndex = 0;
    Mat lastFrame;
    cv::VideoWriter writer;
    
    while ( capture.isOpened() )     // check !!
    {
        cv::Mat frame;
        if ( ! capture.read(frame) ) // another check !!
            break;
        
#if 1
        Mat grayFrame, dilatedFrame, edges, deltaFrame, deltaCopyFrame;
        
        // scale down image
        cv::resize(frame, frame, Size(0,0), 0.33, 0.33);

        // convert to grayscale
        cvtColor(frame, grayFrame, CV_BGR2GRAY);
        
        // add blur
        GaussianBlur(grayFrame, grayFrame, Size(21,21), 0);

        if (frameIndex == 0) {
            frameIndex++;
            
            // position the windows
            moveWindow("Annotated Frame", 0, 0);
            moveWindow("Contour Delta", 0, grayFrame.size().height);
            
            // initialize the video writer
            std::string filename = "/Users/Mark/Desktop/capture.avi";
            //int fcc = CV_FOURCC('D', 'I', 'V', '3');
            int fcc = capture.get(CV_CAP_PROP_FOURCC);
            //int fcc = CV_FOURCC('H', '2', '6', '4');
            //int fps = 20;
            int fps = capture.get(CV_CAP_PROP_FPS);
            //Size frameSize(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT));
            Size frameSize(grayFrame.size().width, grayFrame.size().height);
            writer = VideoWriter(filename, fcc, fps, frameSize);
            
            // manually transcode with 'ffmpeg -i ~/Desktop/capture.avi -vcodec libx264 ~/Desktop/capture.mp4'
            
            std::cout << "FRAME SIZE = " << grayFrame.size().width << " x " << grayFrame.size().height << "\n";
            
            // check if video writer is open
            if (!writer.isOpened()) {
                std::cout << "ERROR OPENING VIDEO FILE FOR WRITE" << std::endl;
                return -1;
            }

            // initialize the last reference frame
            lastFrame = grayFrame;
            continue;
        } else if ((frameIndex % 50) == 0) {
            frameIndex = 0;
            // update the last reference frame
            //lastFrame = grayFrame;
        }
        frameIndex++;
        
        
        // create difference frame
        cv::absdiff(lastFrame, grayFrame, deltaFrame);
        cv::threshold(deltaFrame, deltaFrame, 50, 255, cv::THRESH_BINARY);
        
        // dilate to fill-in holes and find contours
        int iterations = 2;
        cv::dilate(deltaFrame, deltaFrame, Mat(), Point(-1,-1), iterations);
        
        /*
        // find the contours
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        deltaFrame.copyTo(deltaCopyFrame);
        cv::findContours(deltaCopyFrame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
        for (int i=0; i < contours.size(); i++) {
            // ignore contour if smaller than min area
            if (contourArea(contours[i]) < 100) {
                continue;
            }
            
            Rect boundRect = boundingRect(Mat(contours[i]));
            
            Scalar color = Scalar(255, 0, 0);
            rectangle(frame, boundRect.tl(), boundRect.br(), color, 2, 8, 0);
        }
         */
        
        /// Approximate contours to polygons + get bounding rects and circles
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        //deltaFrame.copyTo(deltaCopyFrame);
        cv::findContours(deltaFrame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
        vector<vector<Point> > contours_poly( contours.size() );
        vector<Rect> boundRect( contours.size() );
        vector<Point2f>center( contours.size() );
        vector<float>radius( contours.size() );
        
        for( int i = 0; i < contours.size(); i++ )
        { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
            boundRect[i] = boundingRect( Mat(contours_poly[i]) );
            minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
        }
        
        
        /// Draw polygonal contour + bonding rects + circles
        //Mat drawing = Mat::zeros( deltaFrame.size(), CV_8UC3 );
        for( int i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar(255, 0, 0);
            drawContours( frame, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
            rectangle( frame, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
            circle( frame, center[i], (int)radius[i], color, 2, 8, 0 );
        }
        
        
        
        // find edges with canny
        //Canny(blurFrame, edges, 0, 30, 3);
        
        imshow("Annotated Frame", frame);
        //imshow("Edges", edges);
        imshow("Contour Delta", deltaFrame);
#else
        imshow("Frame", frame);
#endif
        
        // write the frame to the video file
        //std::cout << "WRITING FRAME...";
        writer.write(frame);

        // wait for escape (needed to exit and properly write the video file)
        switch(waitKey(1)) {
            case 27:
                capture.release();
                writer.release();
                return 0;
        }
    }
    return 0;
    
}