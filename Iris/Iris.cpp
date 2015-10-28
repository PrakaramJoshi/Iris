// Iris.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <CameraIn.h>


#include <iostream>
#include <stdio.h>
#include <thread>

#include <Logger\Logger.h>
#include <OpenGL\glfwWindow.h>
#include <OpenGL\ShadersUtility.h>
#include "application_settings.h"
#include "NormalizeMat3D.h"
REGISTER_LOGGER(R"(B:\Logs)", "Iris", "0.0.0.1");

using namespace std;
using namespace AceLogger;
void exitApp(){
	// exit sequence 

	//close the logger
	FinishLog();
};

bool read_app_settings(app_settings &_app_ssettings_val){
	std::string app_setting_file = R"(B:\Workspace\Iris\app_settings.txt)";
	Log("Reading app settings file at : " + app_setting_file);
	return _app_ssettings_val.read_settings(app_setting_file);
}

void normalized_view(app_settings *_app_ssettings_val, ViewPort *_viewPort, BlockingQueue<cv::Mat> *_frames,int *_width,int *_height){
	NormalizeMat3D normalize_data(*_app_ssettings_val, *_viewPort, *_frames);
	if (normalize_data.setup(*_width, *_height)){
		Log("Starting the normalize view...");
		normalize_data.run();
	}
	else
		Log("unable to complete pre start sequence!", LOG_ERROR);
	
}

int main(int argc, const char** argv)
{
	
	atexit(exitApp);
	app_settings app_ssettings_val;
	if (!read_app_settings(app_ssettings_val)){
		Log("Cannot continue as unable to read settings file.", LOG_ERROR);
		exit(0);
	}
	
	BlockingQueue<cv::Mat> frames;
	
	CameraIn cameraIn(frames);
	int width, height;
	if (cameraIn.setup("webcam", width, height)){
		ViewPort view_port(0, 0, width, height);
		std::thread normal_view(normalized_view, &app_ssettings_val, &view_port, &frames,&width,&height);
		cameraIn.run();
		frames.ShutDown();
		normal_view.join();
	}
	else
		frames.ShutDown();
	

	return 0;
}
