#include "NormalizeMat3D.h"
#include <OpenGL\ShadersUtility.h>
#include <CGPolygon.h>
#include <CGAlgorithms.h>
//#define TEST_CODE
#define TEST_ROWS 775
#define TEST_COLS 1000

int window_width;
int window_height;
void window_resize(GLFWwindow *_window, int _width, int _height){
	window_width = _width;
	window_height = _height;
	glViewport(0, 0, _width, _height);
}
NormalizeMat3D::NormalizeMat3D(app_settings &_app_seetings, 
								ViewPort &_viewPort, 
								BlockingQueue<cv::Mat> &_input_frames) :m_viewPort(_viewPort), m_input_frames(_input_frames)
{
	m_app_settings = _app_seetings;
	m_programID = 0;
	m_drawn_lines_persistent = nullptr;
	m_current_frame = nullptr;
	m_thread = nullptr;
	m_draw_points = nullptr;
	m_sheet_filled_data = nullptr;
	m_sheet_wire_frame_data = nullptr;
	m_sheet_data = nullptr;
	m_display_type= _app_seetings.p_display_type;
	m_viewPort.p_start_x = 0;
	m_viewPort.p_start_y = 0;
	window_width = m_viewPort.p_width;
	window_height = m_viewPort.p_height;
}


NormalizeMat3D::~NormalizeMat3D()
{
	if (m_thread)
		m_thread->join();
	delete m_thread;
	delete m_drawn_lines_persistent;
	delete m_draw_points;
	delete m_sheet_filled_data;
	delete m_sheet_wire_frame_data;
	delete m_sheet_data;
	m_thread = nullptr;
	m_sheet_filled_data = nullptr;
	m_sheet_wire_frame_data = nullptr;
	m_draw_points = nullptr;
	m_drawn_lines_persistent = nullptr;;
	m_sheet_data = nullptr;
}

void NormalizeMat3D::flatten_val(float &_val){
	return;
	int factor = 10;
	float flaten_incremebt = 1.0 / (float)(factor);
	for (int i = 0; i < factor; i++){
		if (_val < (i + 1)*flaten_incremebt){
			_val = i*flaten_incremebt;
			return;
		}
	
	}
	_val = 1;
}
double Distance(float &_r,float &_g,float &_b) {

	float r, g, b;
	float x1 = 0;
	float y1 = 0;
	float z1 = 0;
	float x2 = 255.0;
	float y2 = 0.0;
	float z2 = 0.0;
	float A = _r - x1;
	float B = _g - y1;
	float F = _b - z1;
	double C = x2 - x1;
	double D = y2 - y1;
	double E = z2 - z1;
	double dot = A*C + B*D + F*E;
	double len_sq = C*C + D*D + E*E;
	if (len_sq == 0) {
		r = x1;
		g = y1;
		b = z1;
	}
	else {
		double u = dot / len_sq;
		if (u < 0) {
			r = x1;
			g = y1;
			b = z1;
		}
		else if (u > 1) {
			r = x2;
			g = y2;
			b = z2;
		}
		else {
			r = x1 + u*C;
			g = y1 + u*D;
			b = z1 + u*E;
		}
	}
	float r_d = std::pow((_r - r), 2);
	float g_d = std::pow((_g - g), 2);
	float b_d = std::pow((_b - b), 2);
	return std::sqrt(r_d + g_d + b_d);
}
void NormalizeMat3D::normalize_data(){
	cv::Mat *frame;
	cv::Mat *previous_frame = nullptr;
	cv::Mat noise;
	bool noiseFound = false;
	std::vector<int> channels_read;
	channels_read.push_back(0);
	channels_read.push_back(1);
	channels_read.push_back(2);
	int readsize = [&channels_read]()->int {int count = 0; for (auto c : channels_read) {
		if (c)
			count++;
	}
	return count; }();
	float val_map = 100.0 / 60.0;
	while (m_input_frames.Remove(&frame)){
		if (frame){
			
			//cv::cvtColor(*frame, *frame, CV_BGR2);
			uchar *pixel = frame->data;
			normalized_frame *normalized=new normalized_frame();

#ifdef TEST_CODE
			normalized->m_cols = TEST_COLS;
			normalized->m_rows = TEST_ROWS;
			normalized->m_data.reserve(normalized->m_rows*normalized->m_cols*3);
			for (int r = 0; r < normalized->m_rows; r++){
				float y = ((float)r) / (float)normalized->m_rows;
				for (int c = 0; c < normalized->m_cols; c++){
					float x = ((float)c) / (float)normalized->m_cols;
					normalized->m_data.push_back(x);
					normalized->m_data.push_back(1 - y);
					normalized->m_data.push_back(0.5);
				}
			}
#else
			if (previous_frame) {
				cv::Mat dst =*frame;
				//cv::subtract(*frame, *previous_frame, dst);
				
				/*if (noiseFound) {
					cv::Mat temp;
					cv::subtract(dst, noise, temp);
					noise = dst;
					dst = temp;
				}
				else
					noise = dst;*/
				noiseFound = true;
				auto total_channels = dst.channels();
				normalized->m_data.reserve(dst.rows*dst.cols*readsize);
				normalized->m_cols = dst.cols;
				normalized->m_rows = dst.rows;
				normalized->m_colors.reserve(dst.rows*dst.cols * total_channels);

				for (int r = 0; r < dst.rows; r++) {
					float y = ((float)r) / (float)dst.rows;
					for (int c = 0; c < dst.cols; c++) {
						float x = ((float)c) / (float)dst.cols;

						normalized->m_data.push_back(x);
						normalized->m_data.push_back(1 - y);
						normalized->m_data.push_back(0);
						float total_color = 0;
						auto blue = (float)dst.data[r*total_channels* dst.cols + c*total_channels];
						auto green = (float)dst.data[r*total_channels* dst.cols + c*total_channels + 1];
						auto red = (float)dst.data[r*total_channels* dst.cols + c*total_channels + 2];

						float distance = Distance(red, green, blue)/442.0;
						normalized->m_colors.push_back(distance);
						normalized->m_colors.push_back(distance);
						normalized->m_colors.push_back(distance);

						/*if (blue <= red && green <= red&&red>20) {
							if (blue > 180 && green > 180) {
								float distance = Distance(red, green, blue);
								//AceLogger::Log(StringUtils::ToString(distance));
								if (distance < 50)
									total_color += red;


							}
							else
								total_color += red;
						}
						if (total_color == 0) {
							normalized->m_colors.push_back(0.0);
							normalized->m_colors.push_back(0.0);
							normalized->m_colors.push_back(0.0);/*
							normalized->m_colors.push_back(red / 255.0);
							normalized->m_colors.push_back(green / 255.0);
							normalized->m_colors.push_back(blue / 255.0);*/
						/*}
						else {
							normalized->m_colors.push_back(1.0);
							normalized->m_colors.push_back(0.0);
							normalized->m_colors.push_back(0.0);
						}*/
						

					}
				}
			}
			else {
				delete normalized;
				normalized = nullptr;
			}
			
#endif
			std::swap(previous_frame, frame);
			delete frame;
			if(normalized)
				m_normalized_data_q.Insert(normalized);
		}
	}
	m_normalized_data_q.ShutDown();
}

/*void NormalizeMat3D::get_laser_coordinates(normalized_frame * _frame) {
	|
}*/
void NormalizeMat3D::render_normalized_frame(normalized_frame *_frame){


	
	m_sheet_data->set(_frame->m_data,_frame->m_colors);

	m_sheet_filled_data->set_gpu_vertex_data_stale(true);
	m_sheet_wire_frame_data->set_gpu_vertex_data_stale(true);

	m_sheet_filled_data->set_gpu_color_data_stale(true);
	m_sheet_wire_frame_data->set_gpu_color_data_stale(true);

	if (m_display_type == DISPLAY_TYPE::POINT_CLOUD){

		m_draw_points->add_vertices(_frame->m_data);
		m_draw_points->add_colors(_frame->m_colors);
	}
}

void NormalizeMat3D::render(CurrentCamera &_camera, normalized_frame *_frame,bool _frame_updated){
	glUseProgram(m_programID);

	m_viewPort.p_width = window_width;
	m_viewPort.p_height = window_height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera for the view
	glEnable(GL_DEPTH_TEST);

	_camera.move(m_viewPort.p_width, m_viewPort.p_height);

	m_drawn_lines_persistent->draw(true);

	if (_frame&&_frame_updated)
		render_normalized_frame(_frame);

	if (m_display_type==DISPLAY_TYPE::POINT_CLOUD)
		m_draw_points->draw(true);
	else if (m_display_type==DISPLAY_TYPE::MESH_WIRE_FRAME)
		m_sheet_wire_frame_data->draw();
	else if (m_display_type==DISPLAY_TYPE::MESH_FILLED)
		m_sheet_filled_data->draw();	

}

NormalizeMat3D::normalized_frame* NormalizeMat3D::get_frame_to_render(bool &_new_frame){
	normalized_frame *normalized = nullptr;;
	m_normalized_data_q.Remove_try(&normalized);
	if (normalized){
		delete m_current_frame;
		m_current_frame = normalized;
		_new_frame = true;
	}
	else{
		normalized = m_current_frame;
		_new_frame = false;
	}
	return normalized;
}
void NormalizeMat3D::render_pixels(){
	float fov = m_app_settings.p_fov;
	float zNear = m_app_settings.p_zNear;
	float zFar = m_app_settings.p_zFar;
	float zoom_factor = m_app_settings.p_zoom_factor;
	auto window = m_window.get_window();
	CurrentCamera camera(window, fov, zNear, zFar, zoom_factor, m_app_settings.p_auto_rotate, m_app_settings.p_rotate_x, m_app_settings.p_rotate_y, m_app_settings.p_rotate_z,
		m_app_settings.p_rotate_about_x, m_app_settings.p_rotate_about_y, m_app_settings.p_rotate_about_z,
		m_app_settings.p_rotate_speed);
	bool frame_updated = false;
	while (!glfwWindowShouldClose(window)) {

		//render the scene
		normalized_frame *frame_to_render = get_frame_to_render(frame_updated);
		render(camera, frame_to_render, frame_updated);

		//other stuff : FPS, check for user input
		m_window.FPSStatus(window_width, window_height);
		glfwPollEvents();
		glfwSwapBuffers(window);
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}
	}

	normalized_frame *normalized = nullptr;
	while (m_normalized_data_q.Remove(&normalized))
		delete normalized;

}

BlockingQueue<cv::Mat> & NormalizeMat3D::get_input_q(){
	return m_input_frames;
}

void NormalizeMat3D::load_shaders(){

	if (m_programID)
		glDeleteProgram(m_programID);
	std::string shader_vs = R"(B:\Workspace\Iris\Iris\Shaders\vertexcolor_vs.glsl)";
	std::string shader_fs = R"(B:\Workspace\Iris\Iris\Shaders\vertexcolor_fs.glsl)";
	std::vector<std::pair<std::string, GLenum>> shaders;

	shaders.push_back(std::make_pair(shader_vs, GL_VERTEX_SHADER));
	shaders.push_back(std::make_pair(shader_fs, GL_FRAGMENT_SHADER));
	m_programID = GetProgramID_FileShaders(shaders);

	CHECK_GL_ERROR
}

bool NormalizeMat3D::setup(unsigned int _width,
							unsigned int _height){

	m_window.set_title("Iris");
	if (!m_window.init(m_viewPort.p_width, m_viewPort.p_height)){
		AceLogger::Log("unable to initialize the display!", AceLogger::LOG_ERROR);
		return false;
	}
	glfwSetFramebufferSizeCallback(m_window.get_window(), window_resize);

	CHECK_GL_ERROR
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CHECK_GL_ERROR

	load_shaders();

	generate_gl_buffers(m_lines_persistent);
	m_drawn_lines_persistent = new DrawingUtils::DrawnLines(m_lines_persistent.p_vertexbuffer, m_lines_persistent.p_colours_vbo, m_lines_persistent.p_vao);

	generate_gl_buffers(m_points);
	m_draw_points = new DrawingUtils::DrawnPoints(m_points.p_vertexbuffer, m_points.p_colours_vbo, m_points.p_vao);

	generate_gl_buffers(m_mesh_filled);
	m_sheet_filled_data = new DrawingUtils::DrawnSheetStripFilledNoCopy(m_mesh_filled.p_vertexbuffer, m_mesh_filled.p_colours_vbo, m_mesh_filled.p_vao);

	generate_gl_buffers(m_mesh_wire_frame);
	m_sheet_wire_frame_data = new DrawingUtils::DrawnSheetStripWireFrameNoCopy(m_mesh_wire_frame.p_vertexbuffer, m_mesh_wire_frame.p_colours_vbo, m_mesh_wire_frame.p_vao);

	CHECK_GL_ERROR
	glUseProgram(m_programID);

	CHECK_GL_ERROR
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	DrawingUtils::CoordinateAxisXYZ axis(0.5);
	axis.get_vertices(m_drawn_lines_persistent->get_vertices_ref());
	axis.get_colors(m_drawn_lines_persistent->get_colors_ref());

	DrawingUtils::CoordinatePlanes coordinate_planes(0.1, CG3DContants::OCTANTS::PLUS_PLUS_PLUS);
	coordinate_planes.get_vertices(m_drawn_lines_persistent->get_vertices_ref());
	coordinate_planes.get_colors(m_drawn_lines_persistent->get_colors_ref());
#ifdef TEST_CODE
	m_sheet_data = new DrawingUtils::Sheet(TEST_ROWS, TEST_COLS,true);
#else
	m_sheet_data = new DrawingUtils::Sheet(_height, _width,true);
#endif


	m_sheet_filled_data->set_vertex_vector(m_sheet_data->get_vertices());
	m_sheet_filled_data->set_color_vector(m_sheet_data->get_colors());

	m_sheet_wire_frame_data->set_vertex_vector(m_sheet_data->get_vertices());
	m_sheet_wire_frame_data->set_color_vector(m_sheet_data->get_colors());

	return true;
}

void NormalizeMat3D::run(){
	m_thread = new std::thread(&NormalizeMat3D::normalize_data, this);
	render_pixels();
}


