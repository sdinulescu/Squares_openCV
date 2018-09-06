//
//  OpticalFlowExampleApp.cpp
//  OpticalFlowExample
//
//  Created by Stejara Dinulescu on 8/30/18.
//  Optical Flow Class
//  A program to handle optical flow and frame-differencing

#include "SquareGenerator.hpp"

#define MAX_CORNERS 300 //sets up a constant
#define QUALITY_LEVEL 0.005 //whatever corner you find is good
#define MIN_DISTANCE 3 //how far away the corners have to be from each other
#define ELAPSED_FRAMES 300 //number of elapsed frames to check features

#define NUMBER_OF_SQUARES 10

class OpticalFlowExampleApp : public App {
    
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event) override;
    
    void update() override;
    void draw() override;
    
protected:
    CaptureRef mCapture; //pointer to a capture object
    gl::TextureRef mTexture; //pointer to a texture object
    ci::SurfaceRef mSurface;
    
    cv::Mat mPrevFrame, mCurrFrame, mBGFrame;
    vector<cv::Point2f> mPrevFeatures, mFeatures;
    vector<uint8_t> mFeatureStatuses;
    vector<float> errors; //unsigned integers

    void opticalFlow();
    cv::Mat bsFrameDifference();
    void displayBSDiff();
    void drawBGDots();
    
    SquareFeatures squareFeat;
    SquareFrameDiff squareDiff;
    
    char keyPressed;
    
};

void OpticalFlowExampleApp::setup()
{
    //assigns width and height of rectangle divisions
    squareFeat.divideScreen(NUMBER_OF_SQUARES);
    squareDiff.divideScreen(NUMBER_OF_SQUARES);
    
    try
    {
        mCapture = Capture::create(640, 480); //creates the capture using default camera (webcam)
        mCapture->start(); //start the capture (starts the webcam)
    } catch (ci::Exception &e)
    {
        CI_LOG_EXCEPTION("Failed to init capture", e); //if it fails, it will log on the console
    }
    
}

void OpticalFlowExampleApp::mouseDown( MouseEvent event ) { }

void OpticalFlowExampleApp::keyDown( KeyEvent event )
{
    if (event.getChar() == ' ') //if space bar is down, take a new frame
    {
        mBGFrame = mCurrFrame;
        cv::GaussianBlur(mBGFrame, mBGFrame, cv::Size(5, 5), 0);
    }
    
    //if key pressed is f for features
    if (event.getChar() == 'f') { keyPressed = 'f'; }
    // if key pressed is d for difference
    if (event.getChar() == 'd' )  { keyPressed = 'd'; }
}

cv::Mat OpticalFlowExampleApp::bsFrameDifference()
{
    //background frame differencing when you want to drawBGDots
    cv::Mat outputImg;
    outputImg = squareDiff.frameDifferencing(mCurrFrame, mBGFrame);
    
    //frame differencing for squares by passing in the current frame and the previous frame
    if (keyPressed == 'd') {
        cv::Mat img = squareDiff.frameDifferencing(mCurrFrame, mPrevFrame);
        squareDiff.countPixels( img );
    }
    return outputImg;
}

void OpticalFlowExampleApp::displayBSDiff()
{
    if ( !mCurrFrame.data || !mBGFrame.data ) return; //make sure we have set these frames
    //convert matrix to a surface, convert the surface to a texture, then draw the texture
    cv::Mat output = bsFrameDifference(); //storing the difference into a matrix called "output"
    ci::Surface sur = fromOcv( output ); //create a surface
    gl::TextureRef tex = gl::Texture::create( sur ); //create a texture
    gl::draw( tex ); //draw the texture
}

void OpticalFlowExampleApp::opticalFlow()
{
    if (!mSurface) return; //check if mSurface exists
    if (!mBGFrame.data) return; //check if backkground frame exists
    
    //cv::Mat currFrame = toOcv( Channel( *mSurface ) ); //stores the current frame in a matrix, taking out color information
    cv::Mat currFrame = bsFrameDifference();
    
    if (mPrevFrame.data)
    {
        //find features (points @ corners in the image)
        if( mFeatures.size() < 5 || getElapsedFrames() % ELAPSED_FRAMES == 0 ) //refresh if number of features are less than 5 or every 300 frames
        {
            cv::goodFeaturesToTrack( currFrame, mFeatures, MAX_CORNERS, QUALITY_LEVEL, MIN_DISTANCE ); //corner finder function
        }
    }
        
    mPrevFeatures = mFeatures;
    
     //check the optical flow
    if( !mFeatures.empty() )
    {
        cv::calcOpticalFlowPyrLK(mPrevFrame, currFrame, mPrevFeatures, mFeatures, mFeatureStatuses, errors);
    }
    
    mPrevFrame = currFrame;
    
    if (keyPressed == 'f') { squareFeat.countFeatures(mFeatures); }
}

void OpticalFlowExampleApp::update()
{
    if (mCapture && mCapture->checkNewFrame()) //is there a new image?
    {
        mSurface = mCapture->getSurface(); //will get its most recent surface/whatever it is capturing
        mCurrFrame = toOcv( Channel( *mSurface ) );
        if ( !mTexture ) //if texture doesn't exist
        {
            mTexture = gl::Texture::create( *mSurface ); //create a texture from the surface that we got from the camera
        } else {
            mTexture->update( *mSurface ); //if it does exist, update the surface
        }
    }
    
    opticalFlow();
}

void OpticalFlowExampleApp::drawBGDots() {
    gl::color(1, 1, 1, 1);
    //gl::draw(mTexture); //draws the camera image
    displayBSDiff(); //display subtraction
    gl::color(1, 0, 1, 0.5f); //dots are purple
    
    for(int i = 0; i < mFeatures.size(); i++) //iterating through features array
    {
        gl::drawSolidCircle( fromOcv( mFeatures[i] ), 3 ); //drawing the first features found in the image
        
    }
}

void OpticalFlowExampleApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    //drawBGDots();
    if (keyPressed == 'f') { squareFeat.displaySquares( keyPressed ); }
    else if (keyPressed == 'd') { squareDiff.displaySquares( keyPressed ); }
}

CINDER_APP( OpticalFlowExampleApp, RendererGl )
