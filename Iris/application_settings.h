#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H
#include <string>
#include <vector>
#include <boost\program_options.hpp>
#include <Logger\Logger.h>
#include <boost\filesystem.hpp>
#include <boost\algorithm\string.hpp>
#include <CGMath.h>

enum DISPLAY_TYPE{
	POINT_CLOUD,
	MESH_WIRE_FRAME,
	MESH_FILLED,
	UNKNOWN_TYPE
};
static const std::string DISPLAY_TYPE_STR[DISPLAY_TYPE::UNKNOWN_TYPE] = { "POINT_CLOUD",
"MESH_WIRE_FRAME",
"MESH_FILLED" };

static inline DISPLAY_TYPE get_display_type(const std::string &_str){
	for (int i = 0; i < DISPLAY_TYPE::UNKNOWN_TYPE; ++i){
		if (boost::iequals(DISPLAY_TYPE_STR[i], _str)){
			return DISPLAY_TYPE(i);
		}
	}
	return DISPLAY_TYPE::UNKNOWN_TYPE;
}
struct app_settings{
	
	float p_fov;
	float p_zNear;
	float p_zFar;
	float p_zoom_factor;
	bool p_auto_rotate;
	float p_rotate_x;
	float p_rotate_y;
	float p_rotate_z;
	float p_rotate_about_x;
	float p_rotate_about_y;
	float p_rotate_about_z;
	float p_rotate_speed;

	DISPLAY_TYPE p_display_type;

	app_settings(){

		p_fov = 50.0;
		p_zNear = 0.01;
		p_zFar = 1000.0;
		p_zoom_factor = 5.0;

		p_auto_rotate = false;

		p_rotate_x = 0;
		p_rotate_y = 1;
		p_rotate_z = 0;

		p_rotate_about_x = 0;
		p_rotate_about_y = 1;
		p_rotate_about_z = 0;

		p_rotate_speed = 0.5;

		p_display_type = DISPLAY_TYPE::POINT_CLOUD;

	};

	bool read_settings(const std::string &_settings_file){
		try
		{
			std::vector<std::string> options;
			if (!read_file(_settings_file, options)){
				AceLogger::Log("Cannot read settings as the settings file not found.", AceLogger::LOG_ERROR);
				return false;
			}

			namespace po = boost::program_options;
			po::options_description desc("Options");


			std::string auto_rotate;
			std::string display_type;

			desc.add_options()
				("help", "Print help messages")
				("zoom_factor", po::value<float>(&p_zoom_factor)->default_value(5), "camera zoom factor")
				("fov", po::value<float>(&p_fov)->default_value(0), "camera fov")
				("znear", po::value<float>(&p_zNear)->default_value(0), "camera closest z rendered")
				("zfar", po::value<float>(&p_zFar)->default_value(0), "camera furthest z rendered")
				("auto_rotate", po::value<std::string>(&auto_rotate)->default_value("No"), "auto rotate the view")
				("rotate_x", po::value<float>(&p_rotate_x)->default_value(0), "rotate about point x")
				("rotate_y", po::value<float>(&p_rotate_y)->default_value(0), "rotate about point y")
				("rotate_z", po::value<float>(&p_rotate_z)->default_value(0), "rotate about point z")
				("rotate_about_x", po::value<float>(&p_rotate_about_x)->default_value(0), "rotate about x")
				("rotate_about_y", po::value<float>(&p_rotate_about_y)->default_value(0), "rotate about y")
				("rotate_about_z", po::value<float>(&p_rotate_about_z)->default_value(0), "rotate about z")
				("rotate_speed", po::value<float>(&p_rotate_speed)->default_value(0), "rotate speed degrees per second")
				("display_type", po::value<std::string>(&display_type)->default_value(DISPLAY_TYPE_STR[DISPLAY_TYPE::POINT_CLOUD].c_str()), "display type(POINT_CLOUD, MESH_WIRE_FRAME or MESH_FILLED)")
				;

			po::variables_map vm;
			try
			{
				po::store(po::command_line_parser(options).options(desc).run(),
					vm);
				if (vm.count("help"))
				{
					AceLogger::Log("Help not available", AceLogger::LOG_WARNING);
					return false;
				}

				po::notify(vm);
				p_display_type = get_display_type(display_type);
				if (p_display_type == DISPLAY_TYPE::UNKNOWN_TYPE){
					AceLogger::Log("invalid option for display_type( POINT_CLOUD, MESH_WIRE_FRAME or MESH_FILLED)", AceLogger::LOG_ERROR);
					return false;
				}
				if (!get_bool(auto_rotate, p_auto_rotate)){
					AceLogger::Log("invalid option for auto_rotate( choose yes or no)", AceLogger::LOG_ERROR);
					return false;
				}

				return validate_settings();
			}
			catch (po::error& e)
			{
				AceLogger::Log(e.what(), AceLogger::LOG_ERROR);
				return false;
			}


		}
		catch (std::exception& e)
		{
			AceLogger::Log("Unhandled Exception", AceLogger::LOG_ERROR);
			AceLogger::Log(e.what(), AceLogger::LOG_ERROR);
			return false;

		}
	}
private:
	bool read_file(const std::string &_file_path,
		std::vector<std::string> &_data)const{
		if (!boost::filesystem::exists(boost::filesystem::path(_file_path))){
			AceLogger::Log("file doesn't exist: " + _file_path, AceLogger::LOG_ERROR);
			return false;
		}
		std::ifstream ifs(_file_path);
		std::string temp;
		while (std::getline(ifs, temp)){
			_data.push_back(temp);
		}
		ifs.close();
		return true;
	};

	bool get_bool(std::string _str, bool &_val){
		boost::trim(_str);
		if (boost::iequals(_str, "Yes")){
			_val = true;
			return true;
		}
		else if (boost::iequals(_str, "No")){
			_val = false;
			return true;
		}
		return false;

	}

	bool validate_settings()const {
		bool returnVal = true;
		if (returnVal)
			AceLogger::Log("input data validated");

		return returnVal;
	}
};
#endif