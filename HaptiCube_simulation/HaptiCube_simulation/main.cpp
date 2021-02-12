/*
 *
 * 해야할 것
 * 1. 난수 설정하여 1 ~ 10사이 구의 갯수 구하기
 * 2. 3가지 물체중 랜덤하게 물체 생성
 * 3. 전반적인 글 관련 UI 정리
 *
 */

#include "header.h"

#define PI 3.141592
#define numberOfSides 100

using namespace std;
using namespace glm;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

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
char mode_tilt_left[16] = "T,4,F100.00,D00";
char mode_tilt_right[16] = "T,1,F100.00,D00"; /// 수정
char mode_default[4] = "OFF";

//char *Haptic_mode[3] = { mode_tilt_left, mode_tip, mode_tilt_right};
char Haptic_mode[16];
char end_ = 0xFF;
int sp_cnt = 0;
float sphere_y[20] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
float sphere_ran_y[20] = { 0.0f, };
vec3 half_position[] = {
	vec3(-5.0f, 0.0f, 0.0f),
	vec3(5.0f, 0.0f, 0.0f),
	vec3(5.0f, 0.0f, 0.0f)
};
bool sphere_pos_flag[20] = { false, };
vec3 sphere_pos[20] = {  /// 4.5가 마지막!
	vec3( 0.5f, sphere_y[0],   0.0f),
	vec3(-0.5f, sphere_y[1],   1.0f),
	vec3(-0.5f, sphere_y[2],   2.0f),
	vec3( 0.5f, sphere_y[3],   3.0f),

	vec3( 4.5f, sphere_y[4],   0.5f),
	vec3(-4.5f, sphere_y[5],   1.5f),
	vec3(-4.5f, sphere_y[6],   2.5f),
	vec3( 4.5f, sphere_y[7],   3.5f),

	vec3( 3.5f, sphere_y[8],   2.0f),
	vec3( 3.5f, sphere_y[9],   0.0f),
	vec3(-3.5f, sphere_y[10],  1.0f),
	vec3(-3.5f, sphere_y[11],  3.0f),

	vec3( 2.5f, sphere_y[12],  0.5f),
	vec3( 2.5f, sphere_y[13],  1.5f),
	vec3(-2.5f, sphere_y[14],  2.5f),
	vec3(-2.5f, sphere_y[15],  3.5f),

	vec3( 1.5f, sphere_y[16],  0.0f),
	vec3( 1.5f, sphere_y[17],  1.0f),
	vec3(-1.5f, sphere_y[18],  2.0f),
	vec3(-1.5f, sphere_y[19],  3.0f),
};
string text_abc[3] = { "A", "B", "C" };
GLuint pos_text[3] = { 200, 640, 1080 };

//##################### Flag
bool countdown = false;

// camera
Camera camera(vec3(0.0f, 1.0f, 8.0f));
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
char buf[3] = {};
int normalized_data = 0;
double offset = 0.001;
bool start_ = false;
SerialPort serial("\\\\.\\COM6");
bool serial_error;

// Data Export
float length_data_0[500], time_data_0[500];
float length_data_1[500], time_data_1[500];
int dataCnt = 0;
bool finish = false;

//##################### Function
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback_gl(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void obj_model(Shader shd, Shader shd_t, Model *basic, Model *half, TextRendering t, string *s, GLuint *pos_t, int type, int mode);
void Output_data(float *y, float *time, int cnt);
void Falling_Sphere(Shader shd, Model mod, vec3 *pos, int num, int mode);

int main()
{
	//######   Random   ######
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> ran_num(11, 20);
	uniform_int_distribution<int> ran_mode(1, 3);
	uniform_int_distribution<int> ran_dist(1, 20);
	int mode_controll = ran_mode(gen);
	//int sphere_num = ran_num(gen);
	int sphere_num = 20;
	for (int i = 0; i < sphere_num; i++) {
		sphere_ran_y[i] = ran_dist(gen);
	}
	//cout << mode_controll << ", " << sphere_num << endl;
	int mode = 0;
	string mode_s;
	double mode_num = 0;

	while (1) {
		//cout << "Mode Input : " ;
		cin >> mode_s;
		if (mode_s == "q") {
			mode = 1;
			break;
		}
		else if (mode_s == "w") {
			mode = 2;
			break;
		}
		else if (mode_s == "e") {
			mode = 3;
			break;
		}
		
		/*
		cin >> mode;
		mode = mode % 3;
		if (mode != 4)
			return 0;
		else {
			cout << mode << endl;
			break;
		}
		*/
	}

	// ##################################################################
	// ########################### Duty Ratio ###########################
	// ##################################################################

	if (mode == 1) {
		mode_num = 5;
		copy(mode_tilt_left, mode_tilt_left + 16, Haptic_mode);
	}
	else if (mode == 2) {
		mode_num = 2.5;
		copy(mode_tip, mode_tip + 16, Haptic_mode);
	}
	else if (mode == 3) {
		mode_num = 3.5;
		copy(mode_tilt_right, mode_tilt_right + 16, Haptic_mode);
	}

	// ##################################################################
	// ##################################################################
	// ##################################################################

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
	Shader Sphere_Shader("shader/model_shader.vs", "shader/model_shader.fs");
	Shader Light_Shader("shader/light.vs", "shader/light.fs");
	Shader Timer_Shader("shader/text.vs", "shader/text.fs");
	Shader SF_Shader("shader/sf.vs", "shader/sf.fs");

	//######################################### Model(Sphere, Bowl Series)
	Model sphere("obj/rn.obj");
	Model basic("obj/basic.obj");
	Model basic_half("obj/basic_half.obj");
	Model left("obj/left.obj");
	Model right("obj/right.obj");
	Model left_half("obj/left_half.obj");
	Model right_half("obj/right_half.obj");

	Model model_half_array[] = { left_half, basic_half, right_half };
	Model model_normal[] = { basic, left, right };

	//#########################################
	//################ Vertex ################# 
	//#########################################

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

	//######################################### Timer
	TextRendering textRendering(SCR_WIDTH, SCR_HEIGHT);
	TextRendering Success_Fail(SCR_WIDTH, SCR_HEIGHT);
	
	glfwSetTime(0.0);

	while (!glfwWindowShouldClose(window))
	{
		float c_Frame;

		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Create Window
		processInput(window);

		// Window Initialize
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		// OBJ model output(obj_type = 1 >> Example 3 models, obj_type = 2 >> Single model)
		obj_model(Obj_Shader, SF_Shader, model_normal, model_half_array, Success_Fail, text_abc, pos_text, obj_type, mode_controll); // Sample View
		
		if (countdown) { // sphere_pos.y는 각각 1.0으로 수렴 즉, 모든 점이 수렴하는 중간중간 마다의 개수 파악 및 최대 개수 도달시 일정 시간 이후 종료 되는 지점 도출
			startTime = glfwGetTime();
			// 0이하로 떨어질 경우
			if (3 - (int)startTime == 0) {
				// Count Down finish & Falling mode ON
				Success_Fail.RenderText(SF_Shader, " ", 750, SCR_HEIGHT / 2, 1.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				glfwSetTime(0.0f);
				Countdowntimer = " ";
				countdown = false, timer_start = true, start_ = true;
				startTime = 0.0f;
				c_Frame = 0.0f;
			}
			else {
				// 3, 2, 1 Count Down display part
				Countdowntimer = to_string(3 - (int)startTime);
				Success_Fail.RenderText(SF_Shader, Countdowntimer, 1000.0f, 300.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			}
		}
		else if(!countdown){
			if(start_){
				// Falling Mode Start
				if (timer_start) {
					//Falling Spheres in the Box and Send PWM - Direction Data to activate HaptiCube
					c_Frame = glfwGetTime();
					deltaTime = c_Frame - lastFrame;
					lastFrame = c_Frame;
					for (int i = 0 ; i < sphere_num ; i++) {
						// Sphere Num : 20 , Start at y = 25, Falling randomly 
						sphere_y[i] += (sphere_ran_y[i] * 0.2) * 4;

						// Sphere's Present Position 
						sphere_pos[i].y = 25.0 - sphere_y[i];
						
						// if Sphere's Present Y Position = 1.0 > Stop & Increase Count
						if (sphere_pos[i].y < 1.0) {
							sphere_pos[i].y = 1.0;
							if (!sphere_pos_flag[i]) {
								sp_cnt++;
								sphere_pos_flag[i] = true;
								if (sp_cnt == sphere_num) {
									timer_start = false;
									glfwSetTime(0.0f);
								}
							}

							// PWM( increased Count ) 
							normalized_data = sp_cnt * mode_num;

							// Data Passing
							if (normalized_data / 10 == 0) {
								sprintf(buf, "%d", normalized_data);
								buf[1] = buf[0];
								buf[0] = '0';
								Haptic_mode[13] = buf[0], Haptic_mode[14] = buf[1], Haptic_mode[15] = end_;
								serial_error = serial.writeSerialPort(Haptic_mode, 16);
							}
							else {
								sprintf(buf, "%d", normalized_data);
								Haptic_mode[13] = buf[0], Haptic_mode[14] = buf[1], Haptic_mode[15] = end_;
								serial_error = serial.writeSerialPort(Haptic_mode, 16);
							}
							cout << Haptic_mode << endl;
						}
						
						// Draw Spheres on the OpenGL Simulatoin environment
						Falling_Sphere(Sphere_Shader, sphere, sphere_pos, sphere_num, mode_controll);
					}
								
				}
				else {
					// if all spheres are in end point(y = 1.0), Stop communication after 1.5s
					c_Frame = glfwGetTime();
					if (c_Frame > 1.5) {
						serial_error = serial.writeSerialPort(mode_default, 4);
					}
					sp_cnt = 0;
				}
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

void mouse_callback_gl(GLFWwindow* window, double xpos, double ypos) {
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
			serial_error = serial.writeSerialPort(mode_default, 4);
		}
		else if (obj_type == 1) {
			serial_error = serial.writeSerialPort(mode_default, 4);
		}
		timer_start = false;
		glfwSetTime(0.0);
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
		glfwWaitEventsTimeout(0.4);
		// obj_type == 0 >> tilt
		// obj_type == 1 >> tip
		if (obj_type == 0) {
			obj_type = 1;
			countdown = false;
			timer_start = false;
			start_ = false;
			timer = "0.000";
		}
		else {
			obj_type = 0;
			countdown = false;
			timer_start = false;
			start_ = false;
			timer = "0.000";
		}
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		glfwWaitEventsTimeout(0.4);
		if (countdown) {
			countdown = false;
			glfwSetTime(0.0);
			powerTime = 0;
		}
		else {
			countdown = true;
			glfwSetTime(0.0);
			powerTime = 0;
		}
	}
}

void obj_model(Shader shd, Shader shd_t, Model *basic, Model *half, TextRendering t, string *s, GLuint *pos_t, int type, int mode) {
	// Change Tip, Tilt model & Change normal, destroyed model
	if (type == 0) {
		// View Sample(ex : Half model)
		shd.use();
		shd.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		shd.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shd.setVec3("lightPos", lightPos);
		shd.setVec3("viewPos", camera.Position);

		mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		mat4 view = camera.GetViewMatrix();
		mat4 model = mat4(1.0f);
		model = scale(model, vec3(0.7f));
		
		for (int i = 0; i < 3; i++) {
			model = translate(model, half_position[i]);
			shd.setMat4("projection", projection);
			shd.setMat4("view", view);
			shd.setMat4("model", model);
			half[i].Draw(shd);
		}
		t.RenderText(shd_t, "A                B                C", 265, 450.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	}
	else if (type == 1) {
		// View normal
		shd.use();
		shd.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		shd.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shd.setVec3("lightPos", lightPos);
		shd.setVec3("viewPos", camera.Position);

		mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		mat4 view = camera.GetViewMatrix();
		mat4 model = mat4(0.7f);
		//model = scale(model, vec3(0.7f));
		model = translate(model, vec3(0.0f, -0.3f, 0.0f));
		shd.setMat4("projection", projection);
		shd.setMat4("view", view);
		shd.setMat4("model", model);
		basic[2].Draw(shd);
	}
}

void Output_data(float *y, float *time, int cnt) {
	fstream fout;
	fout.open("Simul_data.csv", ios::out | ios::app);
	cout << "Export Length and Time data" << endl;

	fout << "Exported Data" << "\n" << "Time, Length" << "\n";
	for (int i = 0; i < cnt; i++) {
		fout << time[i] << ", " << y[i] << "\n";
	}
}

void Output_data_fail() {
	fstream fout;
	fout.open("Simul_data.csv", ios::out | ios::app);

	fout << "fail" << "\n";
}

void Falling_Sphere(Shader shd, Model mod, vec3 *pos, int num, int mode) {
	// 일정시간마다 움직이면서 반복(20개로 한정)하여 떨어지게 만들기
	shd.use();
	shd.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
	shd.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shd.setVec3("lightPos", lightPos);
	shd.setVec3("viewPos", camera.Position);

	mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	mat4 view = camera.GetViewMatrix();
	
	shd.setMat4("projection", projection);
	shd.setMat4("view", view);

	for (int i = 0; i < num ; i++) {
		mat4 model = mat4(1.0f);
		model = scale(model, vec3(0.1f));
		model = translate(model, pos[i]);

		shd.setMat4("model", model);
		mod.Draw(shd);
	}
}