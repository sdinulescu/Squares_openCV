//
//  SquareGenerator.cpp
//  OpticalFlowExample
//
//  Created by Stejara Dinulescu on 8/30/18.
//  Square, Square Generator, SquareFeatures, and SquareFrameDiff class
//  Allows generation of squares on-screen for optical flow based on counting features and frame-differencing

#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h" //includes capture object
#include "cinder/Log.h" //logs exceptions if something is wrong with capture object


#include "CinderOpenCV.h"

#include <iostream>
#include <vector>
#include <random>

using namespace ci;
using namespace ci::app;
using namespace std;

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SQUARE_WIDTH 64
#define SQUARE_HEIGHT 48

/*********************************************/

/**
 * Square class
 * Creates a square object that stores square attributes (number of features, x, y, width, height)
**/

class Square {
protected:
    int xPos{}, yPos{}, squareWidth{}, squareHeight{}, numFeatures{};
    
public:
    Square() {}
    Square(int x, int y, int width, int height);
    void setXPos(int x);
    void setYPos(int y);
    void setWidth(int width);
    void setHeight(int height);
    void setFeatureCount(int num);
    int getXPos();
    int getYPos();
    int getWidth();
    int getHeight();
    int getFeatureCount();
};

Square::Square(int x, int y, int width, int height) {
    xPos = x;
    yPos = y;
    squareWidth = width;
    squareHeight = height;
}

void Square::setXPos(int x) { xPos = x; }
void Square::setYPos(int y) { yPos = y; }
void Square::setWidth(int width) { squareWidth = width; }
void Square::setHeight(int height) { squareHeight = height; }
void Square::setFeatureCount(int num) { numFeatures = num; }
int Square::getXPos() { return xPos; }
int Square::getYPos() { return yPos; }
int Square::getWidth() { return squareWidth; }
int Square::getHeight() { return squareHeight; }
int Square::getFeatureCount() { return numFeatures; }

/*********************************************/

/**
 * Abstract class that divides the screen into a grid of squares, storing them into a vector
**/

class SquareGenerator {
protected:
    vector<Square> squares;
public:
    SquareGenerator() {}
    void divideScreen(int);
    void squareProperties(); //test function for squares
    void displaySquares(char);
};

void SquareGenerator::divideScreen(int numSquares)
{
    int squareWidth = getWindowWidth()/numSquares;
    int squareHeight = getWindowHeight()/numSquares;
    for (int i = 0; i < getWindowWidth(); i += squareWidth)
    {
        for ( int j = 0; j < getWindowHeight(); j+= squareHeight)
        {
            Square square(i, j, squareWidth, squareHeight);
            squares.push_back(square);
        }
    }
    //squareProperties();
}

void SquareGenerator::squareProperties()
{
    cout << "Size of square vector: " << squares.size() << endl;
    cout << "**********************" << endl;
    for (int i = 0; i < squares.size(); i++)
    {
        cout << "Square Number " << i << endl;
        cout << "xPos: " << squares[i].getXPos() << endl;
        cout << "yPos: " << squares[i].getYPos() << endl;
        cout << "width of a square: " << squares[i].getWidth() << endl;
        cout << "height of a square: " << squares[i].getHeight() << endl;
        cout << "**********************" << endl;
    }
}

void SquareGenerator::displaySquares(char key) //displays the squares on-screen with transparency varying based on number of features
{
    int norm = 0; //normalizing variable for color/transparecy
    if (key == 'f') { norm = 10; }
    else if (key == 'd' ) { norm = 1005555; }
    
    //squareFeatureProperties();
    for (int i = 0; i < squares.size(); i++)
    {
        gl::color(0, 1, 0,  (float)squares[i].getFeatureCount() / norm ); //set color based on features
        if (squares[i].getFeatureCount() > 0) //draw the square
        {
            gl::drawSolidRect( Rectf( squares[i].getXPos(), squares[i].getYPos(), squares[i].getXPos() + SQUARE_WIDTH, squares[i].getYPos() + SQUARE_HEIGHT  ) );
        }
    }
}

/*********************************************/

/**
 * Child class inheriting from abstract class SquareGenerator
 * Displays squares based on counting the number of features in each square
**/


class SquareFeatures : public SquareGenerator
{
public:
    void countFeatures(vector<cv::Point2f>);
    void squareFeatureProperties(); //cout testing function
};

void SquareFeatures::countFeatures(vector<cv::Point2f> points)
{
    for (int i = 0; i < squares.size(); i++) //checking each square area
    {
        squares[i].setFeatureCount(0);
        int count = 0;
        
        for (int j = 0; j < points.size(); j++) //get the number of features in the square area
        {
            if ( (points[j].x >= squares[i].getXPos() && points[j].x < squares[i].getXPos() + SQUARE_WIDTH )  &&
                 (points[j].y >= squares[i].getYPos() && points[j].y < squares[i].getYPos() + SQUARE_HEIGHT ) )
            {
                count++;
            }
        }
        squares[i].setFeatureCount(count); //set the feature count as a property of the square
    }
}

void SquareFeatures::squareFeatureProperties() // prints the feature count in each square
{
    for (int i = 0; i < squares.size(); i++)
    {
        cout << "Square " << i << ": " << squares[i].getFeatureCount() << endl;
    }
}

/*********************************************/

/**
 * Child class inheriting from abstract class SquareGenerator
 * Displays squares based on frame differencing
**/

class SquareFrameDiff : public SquareGenerator
{
public:
    cv::Mat frameDifferencing(cv::Mat, cv::Mat);
    void countPixels(cv::Mat);
};

cv::Mat SquareFrameDiff::frameDifferencing(cv::Mat currFrame, cv::Mat prevFrame) //calculates frame differencing
{
    cv::Mat input, outputImg;
    cv::GaussianBlur(currFrame, input, cv::Size(5, 5), 0);
    cv::absdiff(input, prevFrame, outputImg);
    cv::threshold(outputImg, outputImg, 150, 255, cv::THRESH_BINARY);
    return outputImg;
}

void SquareFrameDiff::countPixels(cv::Mat outputImg) //counts the number of pixels in each square area
{
    int pixelAddition = 0;
    for (int i = 0; i < squares.size(); i++) //cycle through square vector
    {
        pixelAddition = 0;
        squares[i].setFeatureCount(0);
        for(int a = squares[i].getXPos(); a < squares[i].getXPos() + squares[i].getWidth(); a++)
        {
            for(int b = squares[i].getYPos(); b < squares[i].getYPos() + squares[i].getHeight(); b++)
            {
                pixelAddition += outputImg.at<u_int8_t>(b, a);
            }
        }
        squares[i].setFeatureCount(pixelAddition);
    }
}

/*********************************************/
