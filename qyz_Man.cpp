#include "basketball.h"
#include "object.h"

GLfloat angle = 0.0f;	//用于手的旋转
GLfloat angle1 = 0.0f; //用于视角的旋转
GLfloat angleforleft = 0;	//用于视角的旋转
GLfloat xmouse = 0, ymouse = 0;	//用于存储鼠标上一帧的位置，判断鼠标拖动方向
//人物的位置
float x = 83;
float y = 101;
float z = 22.3;
//视线向量
float xl = -1;
float yl = 0;
float zl = -1.3;
//一些判断状态的变量
int ballmove = 0;
int showhand = 0;
GLfloat HandAngle;

typedef struct {
	GLfloat x, y, z;
}vertex;

typedef struct {
	GLfloat u, v;
}uvs;

typedef struct {
	GLfloat nx, ny, nz;
}normal;

GLuint tex_front = 0;
GLuint tex_back = 0;
GLuint tex_left = 0;
GLuint tex_right = 0;
GLuint tex_top = 0;
GLuint tex_bottom = 0;
extern int score;
extern GLfloat Power;
extern bool pause;
extern bool ballFly;
extern bool resFlag;
extern bool GameOver;

vector<vertex> v;
vector<uvs> uv;
vector<normal> nm;
vector<unsigned int> VI;
vector<unsigned int> NI;
//以下为右手
vector<vertex> vr;
vector<uvs> uvr;
vector<normal> nmr;
vector<unsigned int> VIr;
vector<unsigned int> NIr;
GLUquadric *qobj;

extern map<string, Object> objmap;
extern set<string> objname;
extern string cd;
extern map<string, Material> matname;

void showGameOver()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 500, 0, 500);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1,1,1);
	char buffer[30];
	GLfloat pos1[] = { 210, 235 };
	GLfloat pos2[] = { 210, 265 };
	GLfloat colorvec[] = { 1.0, 1.0, 1.0 };
	sprintf(buffer,"Your Score: %d",score);
	showString("GAME OVER", pos1, colorvec, 20, "Comic Sans MS");
	showString(buffer, pos2, colorvec, 20, "Comic Sans MS");
	glFlush();
	glutSwapBuffers();
}

GLuint LoadTexture_ZZ(char *fn)
{
	int Max;
	GLint width, height, total_bytes;
	GLubyte *pixels = NULL;
	GLuint texture_ID;
	GLint last_texture_ID = 0;

	FILE *fp;
	fp = fopen(fn, "rb");
	if (fp == NULL) {
		cerr << "open file failed" << endl;
		return 0;
	}
	fseek(fp, 0x0012, SEEK_SET);
	fread(&width, 4, 1, fp);
	fread(&height, 4, 1, fp);
	if (width < 0)width = -width;
	if (height < 0)height = -height;
	fseek(fp, 54, SEEK_SET);
	{
		GLint line_bytes = width * 3; //RGB 24bit
		while (line_bytes % 4)line_bytes++;
		total_bytes = line_bytes*height;
	}
	pixels = (GLubyte *)malloc(total_bytes);
	if (pixels == NULL) {
		cerr << "malloc wrong pixels" << endl;
		fclose(fp);
		return 0;
	}
	int count;
	count = fread(pixels, total_bytes, 1, fp);
	if (count <= 0) {
		cerr << "read file error" << endl;
		fclose(fp);
		free(pixels);
		return 0;
	}
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &Max);
	{
		if (!power_of_tw(width) || !power_of_tw(height) || width>Max || height>Max) {
			const GLint width_zoom = 64;
			const GLint height_zoom = 64;
			GLint new_line_bytes, new_total_bytes;
			GLubyte *pixels_zoom = NULL;
			new_line_bytes = width_zoom * 3;
			while (new_line_bytes % 4 != 0)
				++new_line_bytes;
			new_total_bytes = new_line_bytes * height_zoom;
			pixels_zoom = (GLubyte *)malloc(new_total_bytes);
			if (pixels_zoom == NULL) {
				cerr << "malloc wrong pixels_zoom" << endl;
				free(pixels);
				fclose(fp);
				return 0;
			}
			gluScaleImage(GL_RGB, \
				width, height, GL_UNSIGNED_BYTE, pixels, \
				width_zoom, height_zoom, GL_UNSIGNED_BYTE, pixels_zoom);
			free(pixels);
			pixels = pixels_zoom;
			width = width_zoom;
			height = height_zoom;
		}
	}

	glGenTextures(1, &texture_ID);
	if (texture_ID == 0)
	{
		cout << "generate texture_ID failed" << endl;
		free(pixels);
		fclose(fp);
		return 0;
	}

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture_ID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture_ID);

	free(pixels);
	return texture_ID;
}

void getModelforleft()
{
	float xmax = 0;
	float ymax = 0;
	float zmax = 0;
	float xmin = 100;
	float ymin = 100;
	float zmin = 100;
	FILE *fp;
	fp = fopen("leftarm2.obj", "r");
	if (fp == NULL)printf("Open Files Failed!\n");
	while (1) {
		char type[100];
		int response = fscanf(fp, "%s", type);
		if (response == EOF) {
			break;
		}
		if (strcmp(type, "v") == 0) {
			vertex temp;
			fscanf(fp, "%f %f %f", &temp.x, &temp.y, &temp.z);
			v.push_back(temp);
			if (temp.x > xmax)
				xmax = temp.x;
			if (temp.y > ymax)
				ymax = temp.y;
			if (temp.z > zmax)
				zmax = temp.z;
			if (temp.x < xmin)
				xmin = temp.x;
			if (temp.y < ymin)
				ymin = temp.y;
			if (temp.z < zmin)
				zmin = temp.z;
		}
		else if (strcmp(type, "vt") == 0)
		{
			uvs temp;
			fscanf(fp, "%f %f", &temp.u, &temp.v);
			uv.push_back(temp);
		}
		else if (strcmp(type, "vn") == 0) {
			normal temp;
			fscanf(fp, "%f %f %f", &temp.nx, &temp.ny, &temp.nz);
			nm.push_back(temp);
		}
		else if (strcmp(type, "f") == 0)
		{
			unsigned int vi[4], ni[4];
			fscanf(fp, "%d//%d %d//%d %d//%d %d//%d", &vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2], &vi[3], &ni[3]);
			for (int j = 0; j<4; j++) {
				VI.push_back(vi[j]);
				NI.push_back(ni[j]);
			}
		}
		else {
			char buffer[1024];
			//if(strcmp(buffer,"\r\n")==0)break;
			fgets(buffer, 1024, fp);
		}
	}
	fclose(fp);
	cout << "xmax is " << xmax << endl;
	cout << "ymax is " << ymax << endl;
	cout << "zmax is " << zmax << endl;
	cout << "xmin is " << xmin << endl;
	cout << "ymin is " << ymin << endl;
	cout << "zmin is " << zmin << endl;
}

void getModelforright()
{
	float xmax = 0;
	float ymax = 0;
	float zmax = 0;
	float xmin = 10000;
	float ymin = 10000;
	float zmin = 10000;
	FILE *fp;
	fp = fopen("rightarm2.obj", "r");
	if (fp == NULL)printf("Open Files Failed!\n");
	while (1) {
		char type[100];
		int response = fscanf(fp, "%s", type);
		if (response == EOF) {
			break;
		}
		if (strcmp(type, "v") == 0) {
			vertex temp;
			fscanf(fp, "%f %f %f", &temp.x, &temp.y, &temp.z);
			vr.push_back(temp);
			if (temp.x > xmax)
				xmax = temp.x;
			if (temp.y > ymax)
				ymax = temp.y;
			if (temp.z > zmax)
				zmax = temp.z;
			if (temp.x < xmin)
				xmin = temp.x;
			if (temp.y < ymin)
				ymin = temp.y;
			if (temp.z < zmin)
				zmin = temp.z;
		}
		else if (strcmp(type, "vt") == 0)
		{
			uvs temp;
			fscanf(fp, "%f %f", &temp.u, &temp.v);
			uvr.push_back(temp);
		}
		else if (strcmp(type, "vn") == 0) {
			normal temp;
			fscanf(fp, "%f %f %f", &temp.nx, &temp.ny, &temp.nz);
			nmr.push_back(temp);
		}
		else if (strcmp(type, "f") == 0)
		{
			unsigned int vi[4], ni[4];
			fscanf(fp, "%d//%d %d//%d %d//%d %d//%d", &vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2], &vi[3], &ni[3]);
			for (int j = 0; j<4; j++) {
				VIr.push_back(vi[j]);
				NIr.push_back(ni[j]);
			}
		}
		else {
			char buffer[1024];
			//if(strcmp(buffer,"\r\n")==0)break;
			fgets(buffer, 1024, fp);
		}
	}
	fclose(fp);
	cout << "xmax is " << xmax << endl;
	cout << "ymax is " << ymax << endl;
	cout << "zmax is " << zmax << endl;
	cout << "xmin is " << xmin << endl;
	cout << "ymin is " << ymin << endl;
	cout << "zmin is " << zmin << endl;
}

void myDisplay(void)
{
	// 清除屏幕
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//show Powew.Time.String
	showPower();

	GLfloat pos[] = { 0, 470 };
	GLfloat color[] = { 0.4, 0.7, 0.9 };
	int FontSize = 48;
	char buffer[30];
	sprintf(buffer, "Score: %d", score);
	showString(buffer, pos, color, FontSize, "Comic Sans MS");

	showTime();

	if (!resFlag)showResponse(0);
	//show Powew.Time.String

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75, 1, 1, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(x, y, z, x + xl, y + yl, z + zl, 0, 0, 1);

	//以下是设置光照
	{
		GLfloat light_position[] = { 1, 1, 1, 0 };
		GLfloat light_ambient[] = { 0, 0, 0, 1 };
		GLfloat light_diffuse[] = { 1, 1, 1, 1 };
		GLfloat light_specular[] = { 1, 1, 1, 1 };

		glLightfv(GL_LIGHT0, GL_POSITION, light_position); //指定第0号光源的位置
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient); //指定最终遗留在环境中的光线强度
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);//漫反射后
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);//镜面反射后

		glEnable(GL_LIGHT0);//使用第0号光照
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);  //开启深度探测
		glShadeModel(GL_SMOOTH); //平滑着色
	}

	//以下是天空盒子
	{
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);

		glColor3f(1, 1, 1);

		glBindTexture(GL_TEXTURE_2D, tex_front);
		glBegin(GL_POLYGON);
		glTexCoord2d(0, 0);
		glVertex3f(-80, -75, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-80, -75, 300);
		glTexCoord2d(1, 1);
		glVertex3f(-80, 225, 300);
		glTexCoord2d(1, 0);
		glVertex3f(-80, 225, 0);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex_left);
		glBegin(GL_POLYGON);
		glTexCoord2d(0, 0);
		glVertex3f(-80, 225, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-80, 225, 300);
		glTexCoord2d(1, 1);
		glVertex3f(220, 225, 300);
		glTexCoord2d(1, 0);
		glVertex3f(220, 225, 0);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex_top);
		glBegin(GL_POLYGON);
		glTexCoord2d(0, 0);
		glVertex3f(-80, -75, 300);
		glTexCoord2d(0, 1);
		glVertex3f(220, -75, 300);
		glTexCoord2d(1, 1);
		glVertex3f(220, 225, 300);
		glTexCoord2d(1, 0);
		glVertex3f(-80, 225, 300);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex_bottom);
		glBegin(GL_POLYGON);
		glTexCoord2d(0, 0);
		glVertex3f(220, -75, 0);
		glTexCoord2d(0, 1);
		glVertex3f(-80, -75, 0);
		glTexCoord2d(1, 1);
		glVertex3f(-80, 225, 0);
		glTexCoord2d(1, 0);
		glVertex3f(220, 225, 0);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex_right);
		glBegin(GL_POLYGON);
		glTexCoord2d(0, 0);
		glVertex3f(220, -75, 0);
		glTexCoord2d(0, 1);
		glVertex3f(220, -75, 300);
		glTexCoord2d(1, 1);
		glVertex3f(-80, -75, 300);
		glTexCoord2d(1, 0);
		glVertex3f(-80, -75, 0);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex_back);
		glBegin(GL_POLYGON);
		glTexCoord2d(0, 0);
		glVertex3f(220, 225, 0);
		glTexCoord2d(0, 1);
		glVertex3f(220, 225, 300);
		glTexCoord2d(1, 1);
		glVertex3f(220, -75, 300);
		glTexCoord2d(1, 0);
		glVertex3f(220, -75, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);	//在后面的渲染中使用光照
	}

	//绘制手
	if (showhand == 0)
	{
		// 绘制右手
		{
			glPushMatrix();
			//下面三行让手和视角一起动
			glTranslatef(x, y, z);
			glRotatef(angle1*57.2956, 0, 0, -1);
			glTranslatef(-x, -y, -z);
			//和人一起移动
			glTranslatef(x - 83, y - 101, z - 22.3);
			//下面只是为了调整角度
			glTranslatef(88.8, 105, 16);
			glRotatef(5, 0, 0, 1);
			glRotatef(angle, 0, -1, 0);
			glTranslatef(-88.8, -105, -16);


			GLfloat mat_ambient[] = { 0.8078, 0.4823, 0.3921, 1.0f };
			GLfloat mat_diffuse[] = { 0.8078, 0.4823, 0.3921, 1.0f };
			GLfloat mat_specular[] = { 0, 0, 0, 1 };
			GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };//本身不发光
			GLfloat mat_shininess = 5.0f;

			glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
			glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

			for (int i = 0; i < VIr.size() / 4; i++) {
				int t = i * 4;
				vertex v1 = vr[VIr[t] - 1];
				vertex v2 = vr[VIr[t + 1] - 1];
				vertex v3 = vr[VIr[t + 2] - 1];
				vertex v4 = vr[VIr[t + 3] - 1];
				normal n1 = nmr[NIr[t] - 1];
				normal n2 = nmr[NIr[t + 1] - 1];
				normal n3 = nmr[NIr[t + 2] - 1];
				normal n4 = nmr[NIr[t + 3] - 1];

				glBegin(GL_POLYGON);
				glNormal3f(n1.nx, n1.ny, n1.nz);
				glVertex3f(v1.x, v1.y, v1.z);
				glNormal3f(n2.nx, n2.ny, n2.nz);
				glVertex3f(v2.x, v2.y, v2.z);
				glNormal3f(n3.nx, n3.ny, n3.nz);
				glVertex3f(v3.x, v3.y, v3.z);
				glNormal3f(n4.nx, n4.ny, n4.nz);
				glVertex3f(v4.x, v4.y, v4.z);
				glEnd();
			}

			glPopMatrix();
		}

		//以下是画左手
		{
		glPushMatrix();

		//这三行是为了让手和视角一起动
		glTranslatef(x, y, z);
		glRotatef(angle1*57.2956, 0, 0, -1);
		glTranslatef(-x, -y, -z);
		//和人一起移动
		glTranslatef(x - 83, y - 101, z - 22.3);
		//这三行只是为了让手靠近一点
		glTranslatef(88.8, 97, 16);
		glRotatef(-5, 0, 0, 1);
		glRotatef(angleforleft, 0, 0, 1);
		glTranslatef(-88.8, -97, -16);


		for (int i = 0; i < VI.size() / 4; i++) {
			int t = i * 4;
			vertex v1 = v[VI[t] - 1];
			vertex v2 = v[VI[t + 1] - 1];
			vertex v3 = v[VI[t + 2] - 1];
			vertex v4 = v[VI[t + 3] - 1];
			normal n1 = nm[NI[t] - 1];
			normal n2 = nm[NI[t + 1] - 1];
			normal n3 = nm[NI[t + 2] - 1];
			normal n4 = nm[NI[t + 3] - 1];

			glBegin(GL_POLYGON);
			glNormal3f(n1.nx, n1.ny, n1.nz);
			glVertex3f(v1.x, v1.y, v1.z);
			glNormal3f(n2.nx, n2.ny, n2.nz);
			glVertex3f(v2.x, v2.y, v2.z);
			glNormal3f(n3.nx, n3.ny, n3.nz);
			glVertex3f(v3.x, v3.y, v3.z);
			glNormal3f(n4.nx, n4.ny, n4.nz);
			glVertex3f(v4.x, v4.y, v4.z);
			glEnd();
		}
		glPopMatrix();
	}
	}
	loadbasket();
	//LLC

	Display();
	//HB

	glFlush();
	glutSwapBuffers();
}


void myIdle()
{

	if (GameOver){
		showGameOver();
		cout << "Game Over" << endl;
		return;
	}

	if (ballmove == 0)
	{
		angle = angle + 1;
		angleforleft += 1;
		if (angle >= 15)
		{
			angle = 15;
			ballmove = 1;
		}
	}
	if (ballmove == 1)
	{
		angle = angle - 1;
		angleforleft -= 1;
		if (angle <= 0)
		{
			angle = 0;
			ballmove = 0;
		}
	}
	glutPostRedisplay();
	HandAngle = angle;
}

void orientMe(float angle2)
{
	xl = -cos(angle2);
	yl = sin(angle2);
	glLoadIdentity();
	//gluLookAt(x, y, z, x + xl, y + yl, z + zl, 0, 0, 1);
}

void processMousePassiveMotion(int x, int y)
{
	if ((x >= 50 && x <= 550) && (y >= 50 && y <= 550)) {
		if (pause == false)ShowCursor(false);
		else ShowCursor(true);
	}
	else{
		SetCursorPos(350, 350);
	}
	if (x < xmouse)
		angle1 = angle1 - 0.1;
	if (x > xmouse)
		angle1 = angle1 + 0.1;
	if (y < ymouse)
		zl = zl + 0.1;
	if (y > ymouse)
		zl = zl - 0.1;
	xmouse = x;
	ymouse = y;
	orientMe(angle1);
}

void move(int choice)
{
	if (x >= -70 && x <= 210 && y >= -65 && y <= 215)
		switch (choice)
	{
		case 1:
			x = x + xl * 5;
			y = y + yl * 5;
			//cout << "w";
			break;
		case 2:
			x = x - xl * 5;
			y = y - yl * 5;
			//cout << "s";
			break;
		case 3:
			x = x - yl * 5;
			y = y + xl * 5;
			//cout << "a";
			break;
		case 4:
			x = x + yl * 5;
			y = y - xl * 5;
			//cout << "d";
			break;
		default:break;
	}
	if (x < -70)
		x = -70;
	if (x > 210)
		x = 210;
	if (y < -65)
		y = -65;
	if (y > 215)
		y = 215;
}

void inputKey(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	else if (key == 'w' || key == 'W')	//W
	{
		move(1);
	}
	else if (key == 's' || key == 'S')	//S
	{
		move(2);
	}
	else if (key == 'a' || key == 'A')	//A
	{
		move(3);
	}
	else if (key == 'd' || key == 'D')	//D
	{
		move(4);
	}
	if ((key == 'j' || key == 'J') && ballFly == false){
		if (Power<100)Power += 2;
		else Power = 0;
	}
	else if (key == 'p' || key == 'P'){
		pause = !pause;
	}
}

extern GLuint gnd_Texture;

int main(int argc, char* argv[])
{
	getModelforleft();
	getModelforright();
	ReadObj(cd, "./e.obj", objmap, objname, matname);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(10, 10);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("凌晨四点的洛杉矶");
	glutDisplayFunc(myDisplay);
	glutIdleFunc(myIdle);
	glutPassiveMotionFunc(processMousePassiveMotion);
	glutKeyboardFunc(inputKey);
	tex_front = LoadTexture_ZZ("./Front.bmp");
	tex_back = LoadTexture_ZZ("./Back.bmp");
	tex_left = LoadTexture_ZZ("./Left.bmp");
	tex_right = LoadTexture_ZZ("./Right.bmp");
	tex_top = LoadTexture_ZZ("./Top.bmp");
	tex_bottom = LoadTexture_ZZ("./Bottom.bmp");
	glutKeyboardUpFunc(&keyboardUp);
	//gnd_Texture = LoadTexture("./kobe.bmp");
	glutMainLoop();
	return 0;
}