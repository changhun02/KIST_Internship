/*
 *
 * HaptiCube Simulation
 *
 *
 * Todo
 * install librealsense(https://github.com/IntelRealSense/librealsense/releases)
 *		   OpenCV 3.4.10
 *		   OpenGL(GLSL, GLFW, GLAD, GLEW), Assimp >> (https://heinleinsgame.tistory.com/category/OpenGL)
 *		   Freetype(https://github.com/ubawurinna/freetype-windows-binaries)
 *
 *
 * OpenCV : Detect HaptiCube
 * OpenGL : Make simulation
 *
 */

#include "header.h"

#define PI 3.141592
#define numberOfSides 100

using namespace cv;
using namespace std;
using namespace rs2;
using namespace glm;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

//##################### OpenCV
int th = 50, hue_;
Vec3b l_blue1, u_blue1, l_blue2, u_blue2, l_blue3, u_blue3;
Mat img_hsv, img_color, dst_f;
vector<vector<Point>> contours;
vector<Vec4i> hierarchy;
vec3 center_haptic, ex_center_haptic = vec3(0.0f);
float center_x = 0.0f, center_y = 2.0f, center_z = 0.0f, ex_center_y = 0.0f, center_y_gap = 0.0f;
vec3 dots[3];
colorizer color_map;

//##################### OpenGL
float cube_vertices[] = {
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,

	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,

	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f
};
vec3 lightPos(3.0f, 5.0f, 4.0f);
vec3 center_object(0.0);
int obj_type = 0;
char mode_tip[16] = "T,A,F100.00,D00";
char mode_tilt[16] = "T,1,F100.00,D00";
char mode_tip_default[4] = "OFF";
char mode_tilt_default[4] = "OFF";
char end_ = 0xFF;
bool destroyed = false;
bool success_fail = true;
bool display_success_fail = false;
bool countdown = false;
bool SimulEnd = false;
float threshold_haptic_0 = 0.045f, threshold_haptic_1 = -0.045f;
int normalized_data = 0;

// camera
Camera camera(vec3(0.0f, 1.3f, 8.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing & serial communication
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float currentTime = 0.0f;
float startTime = 3.0f;
float ttt = 0.0f;
int powerTime = 0.0f;
float exTime = 0.0f;
bool timer_start = false;
string Countdowntimer = "";
string timer = "0.000";
char buf[3];
double offset = 0.001;

SerialPort serial("\\\\.\\COM7");
bool serial_error;

// Data Export
float length_data_0[500], time_data_0[500];
float length_data_1[500], time_data_1[500];
int dataCnt = 0;
bool data_out = false;
int data_out_cnt = 0;
bool data_out_fail = false;
int data_out_fail_cnt = 0;

//##################### Function
void mouse_callback(int event, int x, int y, int flag, void *param);
void cv_realsense(pipeline pipe);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback_gl(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void obj_model(Shader shd, Model t1, Model t1_d, Model t2, Model t2_d, int type, bool destroyed = false);
void drawCircle(unsigned int VAO, unsigned int VBO, GLfloat x, GLfloat y, GLfloat z, GLfloat radius);
void Output_data(float *y, float *time, int cnt);
void Output_data_fail();
Mat equalize(Mat src);

int main()
{
	//###### Realsense ######
	pipeline pipe;
	config cfg;
	cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
	cfg.enable_stream(RS2_STREAM_COLOR);
	cfg.enable_stream(RS2_STREAM_INFRARED, 1);
	cfg.enable_stream(RS2_STREAM_INFRARED, 2);
	pipe.start(cfg);

	//######   OpenGL   ######
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HaptiCube", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback_gl);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//######################################### GLSL(Shader Program : Model, Light, Haptic)
	Shader Obj_Shader("shader/model_shader.vs", "shader/model_shader.fs");
	Shader Light_Shader("shader/light.vs", "shader/light.fs");
	Shader HaptiCube_Shader("shader/shader.vs", "shader/shader.fs");
	Shader Timer_Shader("shader/text.vs", "shader/text.fs");
	Shader SF_Shader("shader/sf.vs", "shader/sf.fs");

	//#########################################
	//################ Vertex ################# 
	//#########################################

	//######################################### Model Object(Sphere)
	Model tilt("obj/tilt.obj");
	Model tilt_d("obj/tilt(destroyed).obj");
	Model tip("obj/tip.obj");
	Model tip_d("obj/tip(destroyed).obj");
	vec3 origin(0.0f);

	//######################################### Light Object
	unsigned int VBO, Light_VAO;
	glGenVertexArrays(1, &Light_VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
	glBindVertexArray(Light_VAO);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//######################################### Haptic Cube
	unsigned int HaptiCube_VAO, EBO;
	glGenVertexArrays(1, &HaptiCube_VAO);
	glGenBuffers(1, &VBO);
	drawCircle(HaptiCube_VAO, VBO, 0.0f, 0.1f, 0.0f, 0.035f);

	//######################################### Timer
	TextRendering textRendering(SCR_WIDTH, SCR_HEIGHT);
	TextRendering Success_Fail(SCR_WIDTH, SCR_HEIGHT);
	glfwSetTime(0.0);

	while (!glfwWindowShouldClose(window))
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// RealSense, OpenCV를 통해 Haptic 좌표 연산 
		cv_realsense(pipe);
		
		// Create Window
		processInput(window);

		// Window Initialize
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//################################# Sphere
		obj_model(Obj_Shader, tip, tip_d, tilt, tilt_d, obj_type, destroyed);

		//################################# Haptic
		HaptiCube_Shader.use();
		drawCircle(HaptiCube_VAO, VBO, center_x, center_y, 0.0f, 0.035f);

		//################################# Light
		Light_Shader.use();
		mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		mat4 view = camera.GetViewMatrix();
		mat4 model = mat4(1.0f);
		Light_Shader.setMat4("projection", projection);
		Light_Shader.setMat4("view", view);
		model = mat4(0.5f);
		model = translate(model, lightPos);
		model = scale(model, vec3(0.2f));
		Light_Shader.setMat4("model", model);
		glBindVertexArray(Light_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//################################# Timer
		if (countdown) { // 3 2 1 Count Down
			startTime = glfwGetTime();
			// 0이하로 떨어질 경우
			if (3 - (int)startTime == 0) {
				Success_Fail.RenderText(SF_Shader, " ", 750, SCR_HEIGHT / 2, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				glfwSetTime(0.0);
				Countdowntimer = " ";
				countdown = false, timer_start = true;
				startTime = 0.0f;
			}
			else {
				Countdowntimer = to_string(3 - (int)startTime);
				Success_Fail.RenderText(SF_Shader, Countdowntimer, 800.0f, 300.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			}
		}
		// 접촉시간 구간
		else {
			// 접촉하는시간 측정
			if (timer_start) {
				currentTime = glfwGetTime();
				timer = to_string(currentTime);
				if (timer.size() == 9)
					timer = to_string(currentTime).substr(0, 6);
				else
					timer = to_string(currentTime).substr(0, 5);
		
				textRendering.RenderText(Timer_Shader, timer, 1080.0f, 10.0f, 0.5f, glm::vec3(0.15f, 0.23f, 0.96f));
				Success_Fail.RenderText(SF_Shader, " ", SCR_WIDTH / 2, SCR_HEIGHT / 2, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));

				if (obj_type == 0) {
					length_data_0[dataCnt] = center_y;
					time_data_0[dataCnt] = currentTime;
					dataCnt++;
				}
				else if (obj_type == 1) {
					length_data_1[dataCnt] = center_y;
					time_data_1[dataCnt] = currentTime;
					dataCnt++;
				}
			}
			else {
				
				// 성공 실패 유무 판별
				if (display_success_fail) {
					if (obj_type == 0) {
						length_data_0[dataCnt] = center_y;
						time_data_0[dataCnt] = currentTime;
						dataCnt++;
					}
					else if (obj_type == 1) {
						length_data_1[dataCnt] = center_y;
						time_data_1[dataCnt] = currentTime;
						dataCnt++;
					}
					// 성공 & 시뮬레이션 종료
					if (success_fail && SimulEnd && data_out_cnt > 1) {
						Success_Fail.RenderText(SF_Shader, "Finish", 850.0f, 10.0f, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));

						// 성공시 길이 및 시간 데이터 추출
						if (data_out && data_out_cnt == 1) {
							if (obj_type == 0)
								Output_data(length_data_0, time_data_0, dataCnt);

							else if (obj_type == 1) 
								Output_data(length_data_1, time_data_1, dataCnt);

							dataCnt = 0;
							data_out = false;
						}
					}
					// 실패 & 시뮬레이션 종료
					else if (!success_fail && SimulEnd) {
						//Success_Fail.RenderText(SF_Shader, "Fail..", 850.0f, 10.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
						if (data_out_fail && data_out_fail_cnt == 1) {
							Output_data_fail();
						}
						// 성공시 길이 및 시간 데이터 추출
						if (data_out && data_out_cnt == 1) {
							if (obj_type == 0)
								Output_data(length_data_0, time_data_0, dataCnt);

							else if (obj_type == 1)
								Output_data(length_data_1, time_data_1, dataCnt);

							dataCnt = 0;
							data_out = false;
						}
					}
					else
						Success_Fail.RenderText(SF_Shader, " ", 850.0f, 10.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				}
				else
					Success_Fail.RenderText(SF_Shader, " ", 850.0f, 10.0f, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
			}
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &HaptiCube_VAO);
	glDeleteVertexArrays(1, &Light_VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

//############################################ OpenCV HSV 변환을 위해 색상 선택하는 부분
void mouse_callback(int event, int x, int y, int flag, void *param) {
	if (event == EVENT_LBUTTONDOWN) {
		Vec3b color_pix1 = img_color.at <Vec3b>(y, x);

		Mat bgr_color = Mat(1, 1, CV_8UC3, color_pix1);
		Mat hsv_color;
		cvtColor(bgr_color, hsv_color, COLOR_BGR2HSV);

		int hue = hsv_color.at<Vec3b>(0, 0)[0];
		int saturation = hsv_color.at<Vec3b>(0, 0)[1];
		int value = hsv_color.at<Vec3b>(0, 0)[2];

		cout << "hue = " << hue << endl;
		cout << "saturation = " << saturation << endl;
		cout << "value = " << value << endl;

		if (hue < 10) {
			//cout << "case 1" << endl;
			l_blue1 = Vec3b(hue - 10 + 180, th, th);
			u_blue1 = Vec3b(180, 255, 255);
			l_blue2 = Vec3b(0, th, th);
			u_blue2 = Vec3b(hue, 255, 255);
			l_blue3 = Vec3b(hue, th, th);
			u_blue3 = Vec3b(hue + 10, 255, 255);
		}
		else if (hue > 170) {
			//cout << "case 2" << endl;
			l_blue1 = Vec3b(hue, th, th);
			u_blue1 = Vec3b(180, 255, 255);
			l_blue2 = Vec3b(0, th, th);
			u_blue2 = Vec3b(hue + 10 - 180, 255, 255);
			l_blue3 = Vec3b(hue - 10, th, th);
			u_blue3 = Vec3b(hue, 255, 255);
		}
		else {
			//cout << "case 3" << endl;
			l_blue1 = Vec3b(hue, th, th);
			u_blue1 = Vec3b(hue + 10, 255, 255);
			l_blue2 = Vec3b(hue - 10, th, th);
			u_blue2 = Vec3b(hue, 255, 255);
			l_blue3 = Vec3b(hue - 10, th, th);
			u_blue3 = Vec3b(hue, 255, 255);
		}

		hue_ = hue;
	}
}

void mouse_callback_gl(GLFWwindow* window, double xpos, double ypos) {
	//if (firstMouse)
	//{
	//	lastX = xpos;
	//	lastY = ypos;
	//	firstMouse = false;
	//}

	//float xoffset = xpos - lastX;
	//float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	//lastX = xpos;
	//lastY = ypos;
	float xoffset = 0.0f;
	float yoffset = 0.0f;
	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// 창의 크기가 바뀔 때 마다 호출하여 GLFW에 등록, 창을 처음 표시할 때 초기의 창 크기로 이 콜백함수 출력
	glViewport(0, 0, width, height); //앞의 두개는 화면 좌측 아래 시작점, 뒤 두개는 끝점
}

void processInput(GLFWwindow *window) {
	//glfwGetKey는 키보드입력을 받는다
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // GLFW_KEY_ESCAPE = esc
		glfwSetWindowShouldClose(window, true); //GLFW 종료
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	//초기화
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		if (obj_type == 0) {
			serial_error = serial.writeSerialPort(mode_tip_default, 4);
		}
		else if (obj_type == 1) {
			serial_error = serial.writeSerialPort(mode_tilt_default, 4);
		}
		timer_start = false;
		destroyed = false;
		SimulEnd = false;
		display_success_fail = false;
		success_fail = false;
		powerTime = 0;
		currentTime = 0.0f;
		glfwSetTime(0.0);
		data_out = false;
		data_out_fail = false;
		data_out_cnt = 0;
		data_out_fail_cnt = 0;
		dataCnt = 0;
		timer = "0.000";
	}
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		timer_start = !timer_start;
	}
	// 다시 제자리로 컴백
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camera.Position = vec3(0.0f, 0.5f, 8.0f), camera.Yaw = -90.0f, camera.Pitch = 0.0f, camera.Zoom = 45.0f;
		camera.ProcessMouseMovement(0.0f, 0.0f);
	}
	// 물체 변경
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		// obj_type == 0 >> tilt
		// obj_type == 1 >> tip
		if (obj_type == 0) {
			obj_type = 1;
			destroyed = false;
			timer_start = false;
			timer = "0.000";
			center_x = 0.06f;
		}
		else {
			obj_type = 0;
			destroyed = false;
			timer_start = false;
			timer = "0.000";
			center_x = 0.0f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		if (countdown) {
			display_success_fail = false;
			SimulEnd = false;
			countdown = false;
			glfwSetTime(0.0);
			powerTime = 0;
		}
		else {
			display_success_fail = false;
			SimulEnd = false;
			countdown = true;
			data_out = true;
			glfwSetTime(0.0);
			powerTime = 0;
		}
	}
}

Mat equalize(Mat src) {
	// Histogram Equalization
	Mat src_hsv;
	cvtColor(src, src_hsv, COLOR_BGR2HSV);

	vector<Mat> hsv_planes;
	split(src_hsv, hsv_planes);

	equalizeHist(hsv_planes[2], hsv_planes[2]);

	Mat dst_hsv;
	merge(hsv_planes, dst_hsv);

	Mat dst;
	cvtColor(dst_hsv, dst, COLOR_HSV2BGR);

	return dst;
}

void obj_model(Shader shd, Model t1, Model t1_d, Model t2, Model t2_d, int type, bool destroyed) {
	// Change Tip, Tilt model & Change normal, destroyed model
	if (type == 0) {
		// Tip Mode
		if (!destroyed) {
			shd.use();
			shd.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
			shd.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
			shd.setVec3("lightPos", lightPos);
			shd.setVec3("viewPos", camera.Position);

			mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			mat4 view = camera.GetViewMatrix();
			mat4 model = mat4(1.0f);
			model = scale(model, vec3(0.7f));
			shd.setMat4("projection", projection);
			shd.setMat4("view", view);
			shd.setMat4("model", model);
			t1.Draw(shd);
		}
		else {
			shd.use();
			shd.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
			shd.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
			shd.setVec3("lightPos", lightPos);
			shd.setVec3("viewPos", camera.Position);

			mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			mat4 view = camera.GetViewMatrix();
			mat4 model = mat4(1.0f);
			model = scale(model, vec3(0.7f));
			shd.setMat4("projection", projection);
			shd.setMat4("view", view);
			shd.setMat4("model", model);
			t1_d.Draw(shd);
		}
	}
	else if (type == 1) {
		// Tip Mode
		if (!destroyed) {
			shd.use();
			shd.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
			shd.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
			shd.setVec3("lightPos", lightPos);
			shd.setVec3("viewPos", camera.Position);

			mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			mat4 view = camera.GetViewMatrix();
			mat4 model = mat4(1.0f);
			model = scale(model, vec3(0.7f));
			shd.setMat4("projection", projection);
			shd.setMat4("view", view);
			shd.setMat4("model", model);
			t2.Draw(shd);
		}
		else {
			shd.use();
			shd.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
			shd.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
			shd.setVec3("lightPos", lightPos);
			shd.setVec3("viewPos", camera.Position);

			mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			mat4 view = camera.GetViewMatrix();
			mat4 model = mat4(1.0f);
			model = scale(model, vec3(0.7f));
			shd.setMat4("projection", projection);
			shd.setMat4("view", view);
			shd.setMat4("model", model);
			t2_d.Draw(shd);
		}
	}
}

void drawCircle(unsigned int VAO, unsigned int VBO, GLfloat x, GLfloat y, GLfloat z, GLfloat radius){
	//Draw Circle on the center of HaptiCube
	GLfloat twicePi = 2.0f * PI;

	GLfloat circleVerticesX[numberOfSides + 2];
	GLfloat circleVerticesY[numberOfSides + 2];
	GLfloat circleVerticesZ[numberOfSides + 2];

	circleVerticesX[0] = x;
	circleVerticesY[0] = y;
	circleVerticesZ[0] = z;

	for (int i = 1; i < numberOfSides + 2; i++)
	{
		circleVerticesX[i] = x + (radius * cos(i *  twicePi / numberOfSides));
		circleVerticesY[i] = y + (radius * sin(i * twicePi / numberOfSides));
		circleVerticesZ[i] = z;
	}

	GLfloat allCircleVertices[(numberOfSides + 2) * 3];

	for (int i = 0; i < numberOfSides + 2; i++)
	{
		allCircleVertices[i * 3] = circleVerticesX[i];
		allCircleVertices[(i * 3) + 1] = circleVerticesY[i];
		allCircleVertices[(i * 3) + 2] = circleVerticesZ[i];
	}

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(allCircleVertices), allCircleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLE_FAN, 0, numberOfSides + 2);
}

void Output_data(float *y, float *time, int cnt) {
	fstream fout;
	fout.open("Simul_data.csv", ios::out | ios::app);
	cout << "Export Length and Time data" << endl;

	fout << "Exported Data" << "\n" << "Time, Length" << "\n";
	for (int i = 0; i < cnt ; i++) {
		fout << time[i] << ", " << y[i] << "\n";
	}
}

void Output_data_fail() {
	fstream fout;
	fout.open("Simul_data.csv", ios::out | ios::app);

	fout << "fail" << "\n";
}

//############################################
//############################################
//				Haptic 좌표 연산
//############################################
//############################################

void cv_realsense(pipeline pipe) {
	//Color , Depth Frame & Align Both
	auto frames = pipe.wait_for_frames();
	rs2::align align(RS2_STREAM_COLOR);
	auto aligned_frames = align.process(frames);
	rs2::video_frame color_frame = aligned_frames.first(RS2_STREAM_COLOR);
	rs2::depth_frame aligned_depth_frame = aligned_frames.get_depth_frame();
	Mat dm = depth_frame_to_meters(aligned_depth_frame);

	// Filter depth & View depth frame by cv::imshow
	auto depth_d = aligned_depth_frame.apply_filter(color_map);

	const int w = color_frame.as<rs2::video_frame>().get_width();
	const int h = color_frame.as<rs2::video_frame>().get_height();
	Mat image_d(Size(w, h), CV_8UC3, (void*)depth_d.get_data(), Mat::AUTO_STEP);

	//Frame > cv::Mat
	img_color = frame_to_mat(color_frame);
	imshow("Color", img_color);

	//HSV, Filter
	img_color = equalize(img_color);
	bilateralFilter(img_color, dst_f, -1, 10, 5);
	cvtColor(dst_f, img_hsv, COLOR_BGR2HSV);
	GaussianBlur(img_hsv, img_hsv, Size(5, 5), 0);

	//Masking
	Mat img_mask1, img_mask2, img_mask3, img_mask;
	inRange(img_hsv, l_blue1, u_blue1, img_mask1);
	inRange(img_hsv, l_blue2, u_blue2, img_mask2);
	inRange(img_hsv, l_blue3, u_blue3, img_mask3);
	img_mask = img_mask1 | img_mask2 | img_mask3;

	//Open, Close
	erode(img_mask, img_mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
	dilate(img_mask, img_mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));

	dilate(img_mask, img_mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
	erode(img_mask, img_mask, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));

	//Masking
	Mat img_result;
	bitwise_and(img_color, img_color, img_result, img_mask);

	Mat dst = Mat::zeros(img_mask.rows, img_mask.cols, CV_8UC3);
	Mat res = Mat::zeros(dst.rows, dst.cols, CV_8UC3);

	// Contour, Find HaptiCube(Rect)
	findContours(img_mask.clone(), contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> contours_poly(contours.size());
	vector<Point3f>dot(4);

	for (int i = 0; i < contours.size(); i++) {
		double area = contourArea(contours[i], true);
		if (area < -500) {
			// Draw Contour & find angular point
			drawContours(dst, contours, i, CV_RGB(255, 0, 0), 1, 8, hierarchy);
			approxPolyDP(Mat(contours[i]), contours_poly[i], arcLength(Mat(contours[i]), true)*0.02, true);

			if (contours_poly[i].size() == 4) {
				center_haptic = vec3(0.0f);
				int cnt = 4, ex_cnt = 0;
				for (int j = 0; j < 4; j++) {
					//Draw purple dot on angular point & Calculate Center point
					line(dst, contours_poly[i][j], contours_poly[i][j], CV_RGB(255, 0, 255), 5);
					dot[j].x = (contours_poly[i][j].x);
					center_haptic.x += dot[j].x;
					dot[j].z = (contours_poly[i][j].y);
					center_haptic.z += dot[j].z;
				}
				// Measure distance between Center point and Camera
				center_haptic.y = aligned_depth_frame.get_distance(center_haptic.x / 4, center_haptic.z / 4);

				// 거리 값이 벗어나는 것을 방지
				if (center_haptic.y == 0.0)
					center_y = 0.0;
				else if (center_haptic.y >= 2.0)
					center_y = 2.0;
				else 
					center_y = center_haptic.y;
				
				if (obj_type == 0)
					center_y -= 0.2f;
				else if(obj_type == 1)
					center_y -= 0.3f;
			}
		}
		else {
			center_haptic = ex_center_haptic;
		}
	}
	
	/*
	 *----------------------------------------------------------
	 *
	 * ### Obj_type = 0 : Tip Mode ###
	 *		0.045
	 *		  y > 0.05 : Not Touch ,Not Destroyed, Timer Stop
	 * 0.04 < y < 0.05 : Touched, Not Destroyed, Timer Start
	 *		  y < 0.04 : Destroyed, Timer Stop
	 *
	 *	Diff : 0.015
	 *
	 *----------------------------------------------------------
	 *
	 * ### Obj_type = 1 : Tilt Mode ###
	 *		-0.045
	 *		   y > -0.04 : Not Touch ,Not Destroyed, Timer Stop
	 * -0.05 < y < -0.04 : Touched, Not Destroyed, Timer Start
	 *		   y < -0.05 : Destroyed, Timer Stop
	 *
	 *  Diff : 0.02
	 *
	 *----------------------------------------------------------
	 *
	 * normalization (Max, Min) >> PWM (Max : 70%)
	 *
	 */
	
	// Tip 모션
	if (obj_type == 0) {
		// 접촉 상황( 0.03 < y < 0.05 )
		if ((center_y <= threshold_haptic_0 + 0.005) && (center_y >= threshold_haptic_0 - offset)) {
			if (!destroyed) {
				destroyed = false;
				normalized_data = 20.0f + (((0.05 - center_y) / (0.05 + offset)) * 30.0f);
				if (normalized_data / 10 == 0) {
					sprintf(buf, "%d", normalized_data);
					buf[1] = buf[0];
					buf[0] = '0';
				}
				else
					sprintf(buf, "%d", normalized_data);

				mode_tip[13] = buf[0], mode_tip[14] = buf[1], mode_tip[15] = end_, mode_tip_default[4] = end_;
				serial_error = serial.writeSerialPort(mode_tip, sizeof(mode_tip) / sizeof(mode_tip[0]));
				// 3초이내에 접촉했을 경우(성공)
				if (currentTime <= 3) {
					timer_start = false;
					display_success_fail = true;
					success_fail = true;
					glfwSetTime(0.0);
					powerTime++;
				}
				// 접촉 성공 후, 약 3초뒤 자동 정지
				if (powerTime > 40) {
					serial_error = serial.writeSerialPort(mode_tip_default, 4);
					cout << mode_tip_default << endl;
					SimulEnd = true;
					data_out = true;
					glfwSetTime(0.0);
					data_out_cnt++;
				}
			}
		}
		// Destroyed 상황(실패)
		else if (center_y < threshold_haptic_0 - offset) {
			serial_error = serial.writeSerialPort(mode_tip_default, 4);
			destroyed = true;
			timer_start = false;
			SimulEnd = true;
			success_fail = false;
			glfwSetTime(0.0);
			data_out_fail_cnt++;
			powerTime++;
			if (powerTime > 40) {
				serial_error = serial.writeSerialPort(mode_tip_default, 4);
				cout << mode_tip_default << endl;
				data_out = true;
				glfwSetTime(0.0);
				data_out_cnt++;
			}
		}
		// 3초 초과상황(실패)
		if (currentTime > 3) {
			serial_error = serial.writeSerialPort(mode_tip_default, 4);
			timer_start = false;
			display_success_fail = true;
			SimulEnd = true;
			success_fail = false;
			currentTime = 0.0f;
			glfwSetTime(0.0);
			data_out_fail_cnt++;
		}
	}

	// Tilt 모션
	else if (obj_type == 1) {
		// 접촉 상황( 0.03 < y < 0.05 )
		if ((center_y <= threshold_haptic_1 + 0.005) && (center_y >= threshold_haptic_1 - offset)) {
			if (!destroyed) {
				destroyed = false;
				normalized_data = ((-0.05f - center_y) / (-0.05 + offset)) * 80.0f;
				if (normalized_data / 10 == 0) {
					sprintf(buf, "%d", normalized_data);
					buf[1] = buf[0];
					buf[0] = '0';
				}
				else
					sprintf(buf, "%d", normalized_data);
				
				mode_tilt[13] = buf[0], mode_tilt[14] = buf[1], mode_tilt[15] = end_, mode_tilt_default[4] = end_;
				serial_error = serial.writeSerialPort(mode_tilt, sizeof(mode_tilt) / sizeof(mode_tilt[0]));

				// 3초이내에 접촉했을 경우(성공)
				if (currentTime <= 3) {
					timer_start = false;
					display_success_fail = true;
					SimulEnd = true;
					success_fail = true;
					glfwSetTime(0.0);
					powerTime++;
				}
				// 접촉 성공 후, 약 3초뒤 자동 정지
				if (powerTime > 40) {
					serial_error = serial.writeSerialPort(mode_tilt_default, 4);
					cout << mode_tilt_default << endl;
					data_out = true;
					glfwSetTime(0.0);
					data_out_cnt ++;
				}
			}
		}
		// Destroyed 상황(실패)
		else if (center_y < threshold_haptic_1 - offset) {
			serial_error = serial.writeSerialPort(mode_tilt_default, 4);
			destroyed = true;
			timer_start = false;
			SimulEnd = true;
			success_fail = false;
			glfwSetTime(0.0);
			data_out_fail_cnt++;
			powerTime++;
			if (powerTime > 40) {
				serial_error = serial.writeSerialPort(mode_tip_default, 4);
				cout << mode_tip_default << endl;
				data_out = true;
				glfwSetTime(0.0);
				data_out_cnt++;
			}
		}
		// 3초 초과상황(실패)
		if (currentTime > 3) {
			serial_error = serial.writeSerialPort(mode_tilt_default, 4);
			timer_start = false;
			display_success_fail = true;
			SimulEnd = true;
			success_fail = false;
			currentTime = 0.0f;
			glfwSetTime(0.0);
			data_out_fail_cnt++;
		}
	}

	ex_center_y = center_y;
	imshow("Result", dst);
	imshow("Depth", image_d);
	setMouseCallback("Color", mouse_callback);
}