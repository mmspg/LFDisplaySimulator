#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>
#include <highgui.h>
#include <cv.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <string>
#include <math.h>
#include <GLFW/glfw3.h>
//#include <OpenGL/gl3.h>
//#include <iomanip>
using namespace cv;

#define WINDOW_NAME "Display Simulation"

#define WINSIZE 512

int LayerNum;
int Frame;
double Bright;
char leftfilename[128];
char rightfilename[128];
const double LayerDis = 0.02;
int score = 0;
bool scoreflag = false;
bool testflag = false;

void loadTxt(char *texfile);
void init(void);
void glut_display(void);
void glut_keyboard(unsigned char key, int x, int y);
void glut_specialinput(int key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
void glut_idle(void);
void draw_layer(void);
void draw_backlight(void);
void set_texture(void);
void loadNext(void);

int TexSize, VerSize;
double Angle1 = 0;
double Angle2 = 0;
int nowFrame = 0;
bool LeftButtonOn = false;
bool RightButtonOn = false;
int move = 0;
double maxAngle1 = 0;
double maxAngle2 = 0;
int widthMM, heightMM;
int nstimulus = 1;
int side = 0;
int flipflag = 0;
int offset = 0;

//double maxangle = 0.1;
double p = 1;

std::ifstream stimuli;

std::ofstream csvfile;
std::ofstream trackfile;

GLuint *Handle, *Handle2;
clock_t initime;

int main(int argc, char *argv[]){
    
    if (!glfwInit())
    {
        // Initialization failed
    }

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    glfwGetMonitorPhysicalSize(primary, &widthMM, &heightMM);
    
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    std::stringstream ss;
    std::stringstream ss2;
    std::string line;
    
    std::string sfn, rfn;
    std::string d = " ";
    
    ss << "output/" << (now->tm_year + 1900) << '-'
    << (now->tm_mon + 1) << '-'
    <<  now->tm_mday << '-'
    << now->tm_hour << '-'
    << now->tm_min << '-'
    << now->tm_sec
    << "-results.csv";
    
    ss2 << "output/" << (now->tm_year + 1900) << '-'
    << (now->tm_mon + 1) << '-'
    <<  now->tm_mday << '-'
    << now->tm_hour << '-'
    << now->tm_min << '-'
    << now->tm_sec
    << "-tracking.csv";
    
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WINSIZE, WINSIZE);
	glutCreateWindow(WINDOW_NAME);
    glutFullScreen();
    loadTxt(argv[1]);


    glutDisplayFunc(glut_display);
	glutKeyboardFunc(glut_keyboard);
	glutMouseFunc(glut_mouse);
	glutMotionFunc(glut_motion);
	glutIdleFunc(glut_idle);
    glutSpecialFunc(glut_specialinput);
    

    csvfile.open(ss.str());
    trackfile.open(ss2.str());
    
    // OPEN STIMULI FILE
    stimuli.open(argv[2]);
    
    std::getline(stimuli, line);
    
    if (nstimulus == 2){
        sfn = line.substr(0, line.find(d));
        rfn = line.substr(line.find(d)+1);
        strcpy(leftfilename, sfn.c_str());
        strcpy(rightfilename, rfn.c_str());

    }
    else if (nstimulus == 1){
        strcpy(leftfilename, line.c_str());
    }
    
    initime = clock();
    
    init();
    
	glutMainLoop();


	return 0;
}

void loadNext(){
    std::string line;
    initime = clock();
    std::string sfn, rfn;
    std::string d = " ";
    
    if (std::getline(stimuli, line)){
        if (nstimulus == 2){
            sfn = line.substr(0, line.find(d));
            rfn = line.substr(line.find(d)+1);
            strcpy(leftfilename, sfn.c_str());
            strcpy(rightfilename, rfn.c_str());
        }
        else if (nstimulus == 1){
            strcpy(leftfilename, line.c_str());
        }
        Angle1 = 0;
        Angle2 = 0;
        init();
        glutPostRedisplay();
    }
    else{
        csvfile.close();
        exit(0);
    }
}

void loadTxt(char *texfile){

	FILE *fp;
	char s[256];

	fp = fopen(texfile, "r");

	int varnum, hornum, iteration;
	char loadfilename[128];
	
//    fscanf(fp, "%s\t%d", s, &mode);
	fscanf(fp, "%s\t%d", s, &varnum);
	fscanf(fp, "%s\t%d", s, &hornum);
	fscanf(fp, "%s\t%d", s, &LayerNum);
	
	fscanf(fp, "%s\t%d", s, &Frame);
	fscanf(fp, "%s\t%lf", s, &Bright);
//    fscanf(fp, "%s\t%d", s, &iteration);
	fscanf(fp, "%s\t%d", s, &nstimulus);
    
    fscanf(fp, "%s\t%d", s, &side);
    fscanf(fp, "%s\t%d", s, &flipflag);
    
    p = sqrt(pow(widthMM,2) + pow(heightMM, 2))/sqrt(pow(glutGet(GLUT_WINDOW_WIDTH),2)+ pow(glutGet(GLUT_WINDOW_HEIGHT), 2))/1000;

    maxAngle1 = atan2(((varnum-1)/2)*p, LayerDis);
    maxAngle2 = atan2(((hornum-1)/2)*p, LayerDis);
//    mode = ((varnum-1)/2)*p;
//    std::cout << std::to_string(p) << std::endl;
//    std::cout <<std::to_string(maxAngle1) <<std::endl;
	fclose(fp);
}

void init(void){

	char Filename[128];
	sprintf(Filename, "%slayer-%d-%d.png", leftfilename, 0, 0);
	Mat img_load = imread(Filename, 1);
	TexSize = img_load.cols;
    VerSize = img_load.rows;
    

	glClearColor(0.5, 0.5, 0.5, 0.0);

	Handle = new GLuint[LayerNum * Frame *nstimulus];
	glGenTextures(LayerNum * Frame*nstimulus, Handle);

	for (int i = 0; i < LayerNum * Frame * nstimulus; i++){

        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Handle[i]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TexSize, VerSize, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
    
    set_texture();
}

void set_texture(void){
    int i = 0;
    for (int side = 0; side < nstimulus; side++) {
        for (int layer = 0; layer < LayerNum; layer++){
            for (int frame = 0; frame < Frame; frame++){
//                int i = layer * Frame + frame;
//                std::cout << "Here it works " << std::string(savefilename) << std::endl;

                char Filename[128];
                if (side == 0) sprintf(Filename, "%slayer-%d-%d.png", leftfilename, layer, frame);
                else sprintf(Filename, "%slayer-%d-%d.png", rightfilename, layer, frame);
                

                Mat img_load = imread(Filename, 1);
                cvtColor(img_load, img_load, CV_BGR2RGB);

                glBindTexture(GL_TEXTURE_2D, Handle[i]);
                
    //            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img_load.cols, img_load.rows, GL_RGB, GL_UNSIGNED_BYTE, img_load.data);
                
//                std::cout << "Here it works " << std::to_string(i) << std::endl;
                i++;
            }
        }
    }

}

void glut_keyboard(unsigned char key, int x, int y){
    clock_t nowtime;
    
    if (!flipflag || testflag){
        
	switch (key){
	
        case '1':
            score = 1;
            scoreflag = true;
            break;
        case '2':
            score = 2;
            scoreflag = true;
            break;
        case '3':
            score = 3;
            scoreflag = true;
            break;
        case '4':
            score = 4;
            scoreflag = true;
            break;
        case '5':
            score = 5;
            scoreflag = true;
            break;
        case 13:
            if (scoreflag){
             nowtime = clock() - initime;
            if (side == 0) {
                //Reference is on the left side of the screen
                trackfile << std::string(leftfilename) << ',' << std::to_string((float)nowtime/CLOCKS_PER_SEC) << "\n";
            
                csvfile << std::string(leftfilename) << ',' << score << "\n";
            }
            else {
                //Reference is on the right side
                trackfile << std::string(rightfilename) << ',' << std::to_string((float)nowtime/CLOCKS_PER_SEC) << "\n";
                csvfile << std::string(rightfilename) << ',' << score << "\n";
            }
                scoreflag = false;
                testflag = false;
                loadNext();
            }
//            break;
//        case 'q':
//        case 'Q':
//        case '\033':
//            exit(0);
	}
	glutPostRedisplay();
    }
}

void glut_specialinput(int key, int x, int y)
{
    if (flipflag == 1){
    switch(key)
    {
        case GLUT_KEY_LEFT:
            //go to the left side
            offset = 0;
                if (!side) //if this is the test side
                    testflag = true;
                else
                    testflag = false;
            break;
        case GLUT_KEY_RIGHT:
            //go to the right side
            offset = 1;
            if (side) //if this is the test side
                testflag = true;
            else
                testflag = false;
            break;
    }
    
    glutPostRedisplay();
    }
}

void glut_mouse(int button, int state, int x, int y){

	if (button == GLUT_LEFT_BUTTON){
		if (state == GLUT_UP){
			LeftButtonOn = false;
		}
		else if (state == GLUT_DOWN){
			LeftButtonOn = true;
		}
	}

	if (button == GLUT_RIGHT_BUTTON){
		if (state == GLUT_UP){
			RightButtonOn = false;
		}
		else if (state == GLUT_DOWN){
			RightButtonOn = true;
		}
	}

}

void glut_motion(int x, int y){

	static int px = -1, py = -1;

	if (LeftButtonOn == true){

		if (px >= 0 && py >= 0){
            
			Angle1 += (double)-(x - px) / 360;
            Angle2 += (double)(y - py) / 360;
            
            if (Angle1 > maxAngle1) Angle1 = maxAngle1;
            if (Angle2 > maxAngle2) Angle2 = maxAngle2;
            
            
            if (Angle1 < -maxAngle1) Angle1 = -maxAngle1;
            if (Angle2 < -maxAngle2) Angle2 = -maxAngle2;
		}

		px = x;
		py = y;

	}
	else{
		px = -1;
		py = -1;
	}

    glutPostRedisplay();
}

void glut_display(void)
{
    glEnable(GL_SCISSOR_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glViewport((glutGet(GLUT_WINDOW_WIDTH))/2 - TexSize/(2/(nstimulus-flipflag)) - 10*((nstimulus-flipflag)-1),(glutGet(GLUT_WINDOW_HEIGHT)-VerSize)/2,TexSize,VerSize);
    glScissor(0, 0, glutGet(GLUT_WINDOW_WIDTH)/(nstimulus-flipflag), glutGet(GLUT_WINDOW_HEIGHT));
    glOrtho(-1.0, 1.0, -1.0, 1.0, 0, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cos(Angle2) * sin(Angle1), sin(Angle2), cos(Angle2) * cos(Angle1), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
//    set_texture(0);

	double depth = -(double)LayerNum / 2.0 * LayerDis;

	glPushMatrix();
	glTranslatef(0.0, 0.0, depth);
	draw_backlight();
	glPopMatrix();
	depth += LayerDis;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	for (int layer = LayerNum -1; layer >= 0; layer--){

		int i = (layer + LayerNum*offset) * Frame + nowFrame;
        
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Handle[i]);

		glPushMatrix();
		glTranslatef(0.0, 0.0, depth);
		draw_layer();
		glPopMatrix();
		depth += LayerDis;
	}

    glDisable(GL_BLEND);
//
    glFlush();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
//
//    glutSwapBuffers();
    if (nstimulus == 2 && flipflag == 0){
        glEnable(GL_SCISSOR_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport((glutGet(GLUT_WINDOW_WIDTH))/2 + 10,(glutGet(GLUT_WINDOW_HEIGHT)-VerSize)/2,TexSize,VerSize);
        
        
        glScissor(glutGet(GLUT_WINDOW_WIDTH)/2, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        
        glOrtho(-1.0, 1.0, -1.0, 1.0, 0, 100);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(cos(Angle2) * sin(Angle1), sin(Angle2), cos(Angle2) * cos(Angle1), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
        double depth = -(double)LayerNum / 2.0 * LayerDis;
        
        glPushMatrix();
        glTranslatef(0.0, 0.0, depth);
        draw_backlight();
        glPopMatrix();
        depth += LayerDis;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);
        
        for (int layer = LayerNum -1 ; layer >= 0; layer--){
            
//            int i = layer * Frame + nowFrame;
            int i = (layer + LayerNum)*Frame + nowFrame;
            
//            std::cout << std::to_string(i) << std::endl;
//            glActiveTexture(GL_TEXTURE0+1);
            glBindTexture(GL_TEXTURE_2D, Handle[i]);
            
            glPushMatrix();
            glTranslatef(0.0, 0.0, depth);
            draw_layer();
            glPopMatrix();
            depth += LayerDis;
        }
        
        glDisable(GL_BLEND);
        
        glFlush();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        
//
    }
    glutSwapBuffers();
}

void glut_idle(){

    nowFrame++;
    nowFrame %= Frame;

    static int cnt = 0;
    if(move == 1){
        if(cnt < 40){
            Angle1 += 0.005;
        }
        else if(cnt < 80){
            Angle2 += 0.005;
        }
        else if(cnt < 120){
            Angle1 -= 0.005;
        }
        else{
            Angle2 -= 0.005;
        }
        //printf("%d\n", cnt);
        cnt = (cnt + 1) % 160;
    }

    glutPostRedisplay();
}

void draw_layer(){

	GLdouble pointA[] = { -1, -1, 0.0 };
	GLdouble pointB[] = { 1, -1, 0.0 };
	GLdouble pointC[] = { 1, 1, 0.0 };
	GLdouble pointD[] = { -1, 1, 0.0 };

	glColor4d(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    

	glBegin(GL_POLYGON);
	glTexCoord2d(0.0, 1.0);
	glVertex3dv(pointA);
	glTexCoord2d(1.0, 1.0);
	glVertex3dv(pointB);
	glTexCoord2d(1.0, 0.0);
	glVertex3dv(pointC);
	glTexCoord2d(0.0, 0.0);
	glVertex3dv(pointD);
	glEnd();

    glDisable(GL_TEXTURE_2D);
}

void draw_backlight(void){

	GLdouble pointA[] = { -1, -1, 0.0 };
	GLdouble pointB[] = { 1, -1, 0.0 };
	GLdouble pointC[] = { 1, 1, 0.0 };
	GLdouble pointD[] = { -1, 1, 0.0 };

	glColor4d(1.0, 1.0, 1.0, 1.0);

	glBegin(GL_POLYGON);
	glVertex3dv(pointA);
	glVertex3dv(pointB);
	glVertex3dv(pointC);
	glVertex3dv(pointD);
	glEnd();
}
//
//void glutFullScreen() {
////    glutInitWindowSize(1000, 1000);
//
//    glViewport(0,0,626,434);
//}
