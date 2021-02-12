#pragma once

//Basic
#include <iostream>
#include <fstream>
#include <algorithm>
#include <charconv>

// Serial Communication
#include "SerialPort.h"

//OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Shader.h"
#include "camera.h"
#include "model.h"
#include "filesystem.h"

//OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "ShowImage.h"
#include "textrendering.h"

// Intel RealSense
#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
#include <librealsense2/rs_advanced_mode.hpp>
#include "cv_helper.h"
