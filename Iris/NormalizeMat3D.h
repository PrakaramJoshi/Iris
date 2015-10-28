#pragma once
#include <Datastructures\DataStructure.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <Datastructures\DataStructure.h>
#include <OpenGL\ShadersUtility.h>
#include <Geometry\DrawingUtils.h>
#include "application_settings.h"
#include <OpenGL\glfwWindow.h>
#include <vector>
#include <thread>
#include <CameraView.h>
class NormalizeMat3D
{
	typedef CameraView CurrentCamera;
	struct normalized_frame{
		std::vector<float> m_data;
		std::vector<float>m_colors;
		unsigned int m_rows;
		unsigned int m_cols;
	};

	BlockingQueue<cv::Mat> &m_input_frames;
	BlockingQueue<normalized_frame> m_normalized_data_q;

	GLuint m_programID;

	app_settings m_app_settings;

	AceWindow m_window;

	ViewPort &m_viewPort;

	std::thread *m_thread;

	vao_state m_points;
	DrawingUtils::DrawnPoints *m_draw_points;

	vao_state m_mesh_filled;
	DrawingUtils::DrawnSheetStripWireFrameNoCopy *m_sheet_wire_frame_data;

	vao_state m_mesh_wire_frame;
	DrawingUtils::DrawnSheetStripFilledNoCopy *m_sheet_filled_data;

	DrawingUtils::Sheet *m_sheet_data;

	vao_state m_lines_persistent;
	DrawingUtils::DrawnLines *m_drawn_lines_persistent;

	normalized_frame *m_current_frame;

	DISPLAY_TYPE m_display_type;

	void normalize_data();

	void render_pixels();

	void load_shaders();

	void render(CurrentCamera &_camera,
		normalized_frame *_frame,
		bool _frame_updated);

	void render_normalized_frame(normalized_frame *_frame);

	normalized_frame* get_frame_to_render(bool &_new_frame);

	void flatten_val(float &_val);

	NormalizeMat3D();
public:
	NormalizeMat3D(app_settings &_app_seetings, ViewPort &_viewPort, BlockingQueue<cv::Mat> &_input_frames);
	
	~NormalizeMat3D();

	BlockingQueue<cv::Mat> & get_input_q();

	bool setup(unsigned int _width,
				unsigned int _height);

	void run();
};

