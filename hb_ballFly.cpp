#include "basketball.h"
#include "time.h"

GLfloat ball[3] = { 88.8, 105, 14.77 };
GLfloat handHeight = 20;
GLfloat ballRadius = 1.23;
GLfloat basketInnerRadius = 2.25, basketOuterRadius = 2.35;
GLfloat basketHeight = 30.5, basketX = 0, basketY = 75;
GLfloat counterWidth = 150, counterHeight = 140;
bool ballFly=false;
GLfloat Power=0;
GLfloat eye[3]={150,75,20};
GLfloat center[3]={0,75,30.5};
GLfloat Vxy,Vz;
GLfloat Lxy,Time;
bool Collision;
GLfloat destAngle;
GLint Max=1024;
GLuint gnd_Texture;
int score = 0;
bool Wait=false,pause=false;
string response;
bool resFlag;
bool GameOver=false;
extern GLfloat x;
extern GLfloat y;
extern GLfloat z;
extern GLfloat HandAngle;
extern GLfloat angle1;
extern int showhand;

/*int main(int argc, char * argv[])
{
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE|GLUT_DEPTH);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("Basketball");
    glutDisplayFunc(&Display);
    glutIdleFunc(&Idle);
    glutKeyboardFunc(&keyboard);
    glutKeyboardUpFunc(&keyboardUp);
    gnd_Texture=LoadTexture("./kobe.bmp");
    glutMainLoop();
    return 0;
}*/

void Display()
{
	calculateBall();

    glPushMatrix();
    {
        GLfloat ball_ambient[]={0.2,0,0,1.0f};
        GLfloat ball_diffuse[]={0.3,0,0,1.0f};
        GLfloat ball_specular[]={0.5f,0,0,1.0f};
        GLfloat ball_emission[]={0.3f,0.0f,0.0f,1.0f};
        GLfloat ball_shininess=30;
        
        glMaterialfv(GL_FRONT, GL_AMBIENT,    ball_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,    ball_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR,   ball_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION,   ball_emission);
        glMaterialf (GL_FRONT, GL_SHININESS, ball_shininess);
        
		if (!ballFly){
			glTranslatef(x, y, z);
			glRotatef(angle1*57.2956, 0, 0, -1);
			glTranslatef(-x, -y, -z);
		}
        glTranslatef(ball[0],ball[1],ball[2]);
        glutSolidSphere(ballRadius, 30, 30);
    }
    glPopMatrix();
    
}
void calculateBall()
{
	if (pause)return;
    static GLfloat tm,sinxy,cosxy,init_pos[3];
	static bool In;
	static bool flag; 
	static GLfloat destX,destY;
	static GLfloat CollisionTime;
	static double currentTime;
	GLfloat hp[3];
	hp[0] = x-4;
	hp[1] = y+3.5;
	hp[2] = z - HandAngle*15.7 / 15 - 7 - ballRadius;
	if (hp[2] <= 0)hp[2] = 0.001;
	GLfloat angle;
	if (!ballFly && hp[0] != ball[0] && hp[1] != ball[1]){
		ball[0] = hp[0];
		ball[1] = hp[1];
	}
	if (!ballFly)ball[2] = hp[2];
    if(ball[2]<=0)
    {
		ball[0] = hp[0];
		ball[1] = hp[1];
		ball[2] = hp[2];
        ballFly=false;
        tm=0;
		Time = 0;
		flag = false;
        Power=0;
		if (In) score++;
		currentTime = time(NULL);
		showhand = 0;
    }
    if(ballFly&&tm==0)
    {
		ball[2] = 20;
		ball[0] = x;
		ball[1] = y;
        In=calculateDest(destX,destY);
		angle = calculateAngle();
		//calculateVelo(destX,destY,angle);
		getBestVelo(destX, destY, angle);
        sinxy=(destY-ball[1])/sqrt((destY-ball[1])*(destY-ball[1])+(destX-ball[0])*(destX-ball[0]));
        cosxy=(destX-ball[0])/sqrt((destY-ball[1])*(destY-ball[1])+(destX-ball[0])*(destX-ball[0]));
        init_pos[2]=ball[2];
        init_pos[0]=ball[0];
        init_pos[1]=ball[1];
        tm=Time/1000;
    }
    else if(ballFly)
    {
		if ((ball[2] >= 31.115 || Vz - 100 * tm > 0)&&!flag){
			ball[2] = Vz*tm - 50 * tm*tm + init_pos[2];
			ball[1] = Vxy*tm*sinxy + init_pos[1];
			ball[0] = Vxy*tm*cosxy + init_pos[0];
		}
		else if(In&&Collision){
			if (flag==false){
                GLfloat agl;
                agl=-destAngle-Pi/2+Pi*(rand()%90)/180.0;
                sinxy=sin(agl);
                cosxy=cos(agl);
				init_pos[0] = ball[0];
				init_pos[1] = ball[1];
				Vxy *= ((rand()%3)+6)/9.0;
				flag = true;
				CollisionTime = tm;
			}
			ball[2] = Vz*tm - 50 * tm*tm + init_pos[2];
			ball[1] = Vxy*(tm - CollisionTime)*sinxy + init_pos[1];
			ball[0] = Vxy*(tm - CollisionTime)*cosxy + init_pos[0];
		}
        else if(!In&&Collision){
            if (flag==false){
                GLfloat agl;
                agl=Pi*(rand()%180-90)/180;
                sinxy=sin(agl);
                cosxy=cos(agl);
                init_pos[0] = ball[0];
                init_pos[1] = ball[1];
                init_pos[2] = ball[2];
                Vz = fabs(Vz*((rand()%2)+2)/5.0);
                if(cos(agl-destAngle)>=0.8)Vz=-Vz;
                Vxy *= ((rand()%4)+2)/9.0;
                flag = true;
                CollisionTime = tm;
            }
            GLfloat tempTime=tm - CollisionTime;
            ball[2] = Vz*tempTime - 50*tempTime*tempTime + init_pos[2];
            ball[1] = Vxy*tempTime*sinxy + init_pos[1];
            ball[0] = Vxy*tempTime*cosxy + init_pos[0];
        }
		else if(!In){
            ball[1] = Vxy*tm*sinxy + init_pos[1];
            ball[0] = Vxy*tm*cosxy + init_pos[0];
			ball[2] = Vz*tm - 50 * tm*tm + init_pos[2];
		}
        else{
            ball[2] = Vz*tm - 50 * tm*tm + init_pos[2];
        }
		if (ball[0] <= 0)ball[0] = fabs(ball[0]*0.9);
        if(In&&ball[2]<=30.5&&ball[2]>=27.5){
			tm += Time / 120;
        }
        else tm += Time / 100;
		if (tm > Time ){
			resFlag = true;
		}
		else{
			resFlag = false;
		}
    }

    //Display();
}

void showPower()
{
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 500, 0, 500);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(0.7, 0.7, 0.5);
    glBegin(GL_LINE_LOOP);
    glVertex2f(469, 490);
    glVertex2f(469, 359);
    glVertex2f(490, 359);
    glVertex2f(490, 490);
    glEnd();
    
    GLfloat top=360+1.3*Power;
    GLfloat topRed=0.01*Power;
    glBegin(GL_POLYGON);
    glColor3f(0.05f, 0.1f, 1.0f);
    glVertex2f(489.5, 360);
    glColor3f(0.05f, 0.1f, 1.0f);
    glVertex2f(469.5, 360);
    glColor3f(topRed, 0.1, 0.05);
    glVertex2f(469.5, top);
    glColor3f(topRed, 0.1, 0.05);
    glVertex2f(489.5, top);
    glEnd();
}

/*void keyboard(unsigned char key,int x,int y)
{
	if ((key == 'd' || key == 'D')&&ballFly==false){
		if (Power<100)Power += 2;
		else Power = 0;
	}
    else if(key=='q'||key=='Q')exit(0);
}*/

void keyboardUp(unsigned char key,int x,int y)
{
    if(key=='j'||key=='J'){
        ballFly=true;
		showhand = 1;
        cout<<Power<<endl;
    }
}

bool calculateDest(GLfloat &destX,GLfloat &destY)
{
    bool Rt;
    GLfloat delta=0;
    if(Power>=98){
        delta=(rand()%30)/100.0;
        destX=basketInnerRadius/2;
        destY=75;
        Collision=false;
		response = PERFECT;
        return true;
    }
    else if(Power>=92){
        if(rand()%100<95){
            delta=(rand()%30)/100.0;
            Collision=false;
			response = PERFECT;
            Rt=true;
        }
        else{
            delta=(rand()%61)/100.0+2.25;
            Collision=true;
			response = GOOD;
            Rt=true;
        }
    }
    else if(Power>=60){
        GLfloat prob=rand()%100;
        if(prob>95){
            delta=(rand()%30)/100.0;
            Collision=false;
            response = PERFECT;
            Rt=true;
        }
        else if(prob>40){
            delta=(rand()%30)/100.0+2.25;
            Collision=true;
			response = GOOD;
            Rt=true;
        }
        else if(prob>5){
            delta=(rand()%61)/100.0+2.87;
            Collision=true;
			response = BAD;
            Rt=false;
        }
        else{
            delta=(rand()%100)/100.0+4;
            Collision=false;
			response = TERRIBLE;
            Rt=false;
        }
    }
    else{
        GLfloat prob=rand()%100;
        if(prob==99){
            delta=(rand()%30)/100.0;
            Collision=false;
			response = PERFECT;
            Rt=true;
        }
        else if(prob>90){
            delta=(rand()%61)/100.0+2.25;
            Collision=true;
			response = GOOD;
            Rt=true;
        }
        else if(prob>30){
            delta=(rand()%61)/100.0+2.87;
            Collision=true;
			response = BAD;
            Rt=false;
        }
        else{
            delta=(rand()%100)/100.0+4;
            Collision=false;
			response = TERRIBLE;
            Rt=false;
        }
    }
    destAngle=Pi*(rand()%180-90)/180;
    destX=basketInnerRadius/2+delta*cos(destAngle)+1.5;
	//1.5是篮筐的篮脖子，长15厘米
    destY=75+delta*sin(destAngle);
    if(rand()%100>=90){
        destX=-destX;
    }
    return Rt;
}

GLfloat calculateAngle()
{
    //GLfloat tan;
    //tan=(center[2]-eye[2])/sqrt((eye[0]-center[0])*(eye[0]-center[0])+(eye[1]-center[1])*(eye[1]-center[1]));
    GLfloat angle;
    if(rand()%2) angle=0.25;
    else angle=(rand()%30+37.5)/180;
    return angle*Pi;
}

void getBestVelo(GLfloat &destX,GLfloat &destY,GLfloat angle)
{
    //L*tan(angle)-5*t^2=delta_height
    Lxy=sqrt((ball[0]-destX)*(ball[0]-destX)+(ball[1]-destY)*(ball[1]-destY));
    Time=sqrt((Lxy*tan(angle)-(30.5-handHeight))/50);
    Vxy=Lxy/Time;
    Vz=Vxy*tan(angle);
}

GLuint LoadTexture(char *fn)
{
    GLint width, height, total_bytes;
    GLubyte *pixels = NULL;
    GLuint texture_ID;
    GLint last_texture_ID = 0;
    
    FILE *fp;
    fp = fopen(fn,"rb");
    if (fp == NULL){
        cerr << "open file failed" << endl;
        return 0;
    }
    fseek(fp, 0x0012, SEEK_SET);
    fread(&width, 4, 1, fp);
    fread(&height, 4, 1, fp);
    if (width < 0)width = -width;
    if (height < 0)height = -height;
    fseek(fp, BMP_Header_Length, SEEK_SET);
    {
        GLint line_bytes = width * 3; //RGB 24bit
        while (line_bytes % 4)line_bytes++;
        total_bytes = line_bytes*height;
    }
    pixels = (GLubyte *)malloc(total_bytes);
    if (pixels == NULL){
        cerr << "malloc wrong pixels" << endl;
        fclose(fp);
        return 0;
    }
    int count;
    count = fread(pixels, total_bytes, 1 , fp);
    if (count <= 0){
        cerr << "read file error" << endl;
        fclose(fp);
        free(pixels);
        return 0;
    }
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &Max);
    {
        if (!power_of_tw(width) || !power_of_tw(height) || width>Max || height>Max){
            const GLint width_zoom = 64;
            const GLint height_zoom = 64;
            GLint new_line_bytes, new_total_bytes;
            GLubyte *pixels_zoom = NULL;
            new_line_bytes = width_zoom * 3;
            while (new_line_bytes % 4 != 0)
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * height_zoom;
            pixels_zoom = (GLubyte *)malloc(new_total_bytes);
            if (pixels_zoom == NULL){
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
    cout<<texture_ID;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, last_texture_ID);
    
    free(pixels);
    return texture_ID;
}

bool power_of_tw(GLuint num)
{
    if (num <= 0)
        return false;
    return (num & (num - 1)) == 0;
}

void showPicture()
{
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    {
        GLfloat gnd_ambient[] = { 1,1,1,1 };
        GLfloat gnd_diffuse[] = { 1,1,1,1 };
        GLfloat gnd_specular[] = { 1,1,1,1 };
        GLfloat gnd_emission[] = { 1,1,1,1 };
        GLfloat gnd_shininess = 30;
        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, gnd_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, gnd_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gnd_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, gnd_emission);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, gnd_shininess);
        
        glBindTexture(GL_TEXTURE_2D, gnd_Texture);
        glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0, 0, 50);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0, 150, 50);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0, 150, 162.5);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0, 0, 162.5);
        glEnd();
		/* 
		glBindTexture(GL_TEXTURE_2D, gnd_Texture);
        glBegin(GL_POLYGON);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(10,430);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(90,430);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(90,490);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(10,490);
        glEnd();
		*/
    }
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void showString(const char* str, GLfloat *pos, GLfloat *color, int fontSize, char *FontFamily)
{
	selectFont(fontSize, ANSI_CHARSET, FontFamily);

	glColor3fv(color);
	glRasterPos2fv(pos);
	drawString(str);
}

void drawString(const char* str)
{
	static int isFirstCall = 1;
	static GLuint lists;
	if (isFirstCall) { 
		isFirstCall = 0;
		lists = glGenLists(MAX_CHAR);
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	for (; *str != '\0'; ++str)
		glCallList(lists + *str);
}

void selectFont(int size, int charset, const char* face)
{
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);
}

void showTime()
{
	static time_t startTime;
	static bool IsFirstTime = true;
	static float remainedTime, TotalTime = TOTALTIME;
	if (IsFirstTime){
		startTime = startTime = time(NULL);
		IsFirstTime = false;
		remainedTime = TOTALTIME;
	}
	static int colorIndex = 0;
	int FontSize = 48;
	GLfloat colorvec[16][3] = {
		0.3, 0.2, 0.9,
		0.3, 0.2, 0.9,
		0.4, 0.2, 0.8,
		0.4, 0.2, 0.8,
		0.5, 0.2, 0.7,
		0.5, 0.2, 0.7,
		0.6, 0.2, 0.6,
		0.6, 0.2, 0.6,
		0.7, 0.2, 0.5,
		0.7, 0.2, 0.5,
		0.8, 0.2, 0.4,
		0.8, 0.2, 0.4,
		0.9, 0.2, 0.3,
		0.9, 0.2, 0.3,
		1.0, 0.2, 0.2,
		1.0, 0.2, 0.2
	};
	if (Wait){
		startTime=time(NULL);
		remainedTime = TOTALTIME;
	}
	else if (pause){
		startTime = time(NULL);
		TotalTime = remainedTime;
	}
	else{
		remainedTime = TotalTime - difftime( time(NULL) , startTime);
	}
	
	if (remainedTime < 10){
		colorIndex++;
		if (colorIndex == 15)colorIndex = 0;
	}
	else colorIndex = 0;
	if (remainedTime <= 0){
		Wait = false;
		remainedTime = TOTALTIME;
		TotalTime = TOTALTIME;
		pause = true;
		GameOver = true;
	}
	char buffer[30];
	GLfloat pos[] = { 200, 470 };
	sprintf(buffer, "TIME: %02d : %02.0f S", int(remainedTime / 60), remainedTime - int(remainedTime / 60) * 60);
	showString(buffer, pos, colorvec[colorIndex], FontSize, "Comic Sans MS");
}

void showResponse(int size)
{
	char buffer[30];
	sprintf(buffer, response.c_str());
	GLfloat pos[] = { 240, 430 };
	GLfloat colorvec[16][3] = {
		0.3, 0.9, 0.3,
		0.3, 0.9, 0.3,
		0.4, 0.8, 0.3,
		0.4, 0.8, 0.2,
		0.5, 0.7, 0.2,
		0.5, 0.7, 0.2,
		0.6, 0.6, 0.1,
		0.6, 0.6, 0.1,
		0.7, 0.5, 0.1,
		0.7, 0.5, 0.2,
		0.8, 0.4, 0.2,
		0.8, 0.4, 0.2,
		0.9, 0.3, 0.3,
		0.9, 0.3, 0.3,
		1.0, 0.2, 0.3,
		1.0, 0.2, 0.3
	};
	static int colorindex = 0;
	if(colorindex<150)colorindex++;
	else colorindex = 0;
	showString(buffer, pos, colorvec[colorindex/10], size%60, "Comic Sans MS");
}