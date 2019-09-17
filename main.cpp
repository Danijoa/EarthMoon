#include <vgl.h>
#include <InitShader.h>
#include <MyCube.h>
#include "MySphere.h"
#include "Targa.h"
#include <vec.h>
#include <mat.h>

MyCube cube;
MySphere sphere;

GLuint program;
GLuint phong_prog;
GLuint cube_prog;


mat4 g_Mat = mat4(1.0f);

float time = 0;

float g_aspect = 1;

mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	mat4 V = mat4(1.0f);
	
	up = normalize(up);
	vec3 n = normalize(at - eye);
	float a = dot(up, n);
	vec3 v = normalize(up-a*n);
	vec3 w = cross(n, v);

	V[0] = vec4(w, dot(-w, eye));
	V[1] = vec4(v, dot(-v, eye));
	V[2] = vec4(-n, dot(n, eye));
	
	return V;
}

mat4 myOrtho(float l, float r, float b, float t, float zNear, float zFar)
{
	vec3 center = vec3((l+r)/2, (b+t)/2, -(zNear)/2);
	mat4 T = Translate(-center);
	mat4 S = Scale(2/(r-l), 2/(t-b), -1/(-zNear+zFar));
	mat4 V = S*T;

	return V;
}

mat4 myPerspective(float angle, float aspect, float zNear, float zFar)
{
	float rad = angle*3.141592/180.0f;
	mat4 V(1.0f);
	float h = 2*zFar*tan(rad/2);
	float w = aspect*h;
	mat4 S = Scale(2/w, 2/h, 1/zFar);

	float c = -zNear/zFar;
	
	mat4 Mpt(1.0f);
	Mpt[2] = vec4(0, 0, 1/(c+1), -c/(c+1));
	Mpt[3] = vec4(0, 0, -1, 0);
	
	V = Mpt*S;

	return V;

}

void myInitTexture()
{
	//우주 바탕
	STGA img;
	
	GLuint mySpace;
	glGenTextures(1, &mySpace); //이미지는 6개지만 1개의 텍스쳐로
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mySpace);//cubemap이라고 알려줌

	img.loadTGA("space_posx.tga");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, img.width, img.height, 0,GL_BGR, GL_UNSIGNED_BYTE, img.data);
	img.loadTGA("space_negy.tga");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, img.width, img.height, 0,GL_BGR, GL_UNSIGNED_BYTE, img.data);
	img.loadTGA("space_posz.tga");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, img.width, img.height, 0,GL_BGR, GL_UNSIGNED_BYTE, img.data);

	img.loadTGA("space_negx.tga");
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, img.width, img.height, 0,GL_BGR, GL_UNSIGNED_BYTE, img.data);
	img.loadTGA("space_posy.tga");	 
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, img.width, img.height, 0,GL_BGR, GL_UNSIGNED_BYTE, img.data);
	img.loadTGA("space_negz.tga");	 
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, img.width, img.height, 0,GL_BGR, GL_UNSIGNED_BYTE, img.data);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//커진 경우
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//작아진 경우

	//지구색
	STGA image;
	GLuint myEarth[4];
	glGenTextures(4, myEarth);

	image.loadTGA("earthmap_dif.tga");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, myEarth[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width,image.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

	image.destroy();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//지구범프
	GLuint bumpTex;
	glGenTextures(1, &bumpTex);

	image.loadTGA("earthmap_bump.tga");
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, bumpTex);

	GLubyte * slope = new GLubyte[image.width*image.height*3];

	for(int y=0; y<image.height; y++)
		for(int x=0; x<image.width; x++)
		{
			int x2 = (x+1)%image.width;
			int y2 = (y+1)%image.height;

			int d = image.data[(image.width*y+x)*3];
			int d1 = image.data[(image.width*y+x2)*3];
			int d2 = image.data[(image.width*y2+x)*3];

			int du = d1 - d;
			int dv = d2 - d;

			du = du/2 + 128;
			dv = dv/2 + 128;
			
			slope[(image.width*y + x)*3+0]=(GLubyte)du;
			slope[(image.width*y + x)*3+1]=0;
			slope[(image.width*y + x)*3+2]=(GLubyte)dv;
		}
		

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width,image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, slope);

	image.destroy();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//구름
	image.loadTGA("earthmap_cld.tga");
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, myEarth[1]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width,image.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

	image.destroy();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//밤조명
	image.loadTGA("earthmap_light.tga");
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, myEarth[2]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width,image.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

	image.destroy();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	//바다
	image.loadTGA("earthmap_spec.tga");
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, myEarth[3]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width,image.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

	image.destroy();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//달색
	STGA image2;
	GLuint myMoon;
	glGenTextures(1, &myMoon);

	image2.loadTGA("moonmap_dif_s.tga");
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, myMoon);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image2.width,image2.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image2.data);

	image2.destroy();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	//달범프
	GLuint bumpTex2;
	glGenTextures(1, &bumpTex2);

	image2.loadTGA("moonmap_bump_s.tga");
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, bumpTex2);

	GLubyte * slope2 = new GLubyte[image2.width*image2.height*3];

	for(int y=0; y<image2.height; y++)
		for(int x=0; x<image2.width; x++)
		{
			int x2 = (x+1)%image2.width;
			int y2 = (y+1)%image2.height;

			int d = image2.data[(image2.width*y+x)*3];
			int d1 = image2.data[(image2.width*y+x2)*3];
			int d2 = image2.data[(image2.width*y2+x)*3];

			int du = d1 - d;
			int dv = d2 - d;

			du = du/2 + 128;
			dv = dv/2 + 128;
			
			slope2[(image2.width*y + x)*3+0]=(GLubyte)du;
			slope2[(image2.width*y + x)*3+1]=0;
			slope2[(image2.width*y + x)*3+2]=(GLubyte)dv;
		}
		

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image2.width,image2.height, 0, GL_RGB, GL_UNSIGNED_BYTE, slope2);

	image2.destroy();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void myInit()
{
	myInitTexture();

	cube.Init();
    sphere.Init(40,40);

	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	
	phong_prog = InitShader("vPhong.glsl", "fPhong.glsl");
	glUseProgram(phong_prog);

	cube_prog = InitShader("vcube.glsl", "fcube.glsl");	
}

void display()
{
	glClearColor(0,0,0,1);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	mat4 ModelMat = myLookAt(vec3(4,0.7,4), vec3(0,0,0), vec3(0,1,0))*RotateY(time);
	mat4 ProjMat = myPerspective(30, g_aspect, 0.01, 100.0f);

	vec3 ePos = vec3(4*cos(time/200),1,4*sin(time/200));
	mat4 ViewMat = myLookAt(ePos, vec3(0,0,0), vec3(0,1,0));

	g_Mat = ProjMat*ViewMat*ModelMat;

	//우주 그리기
	glUseProgram(cube_prog);
	GLuint uMat = glGetUniformLocation(cube_prog, "uMat");
	GLuint uCubeTexture = glGetUniformLocation(cube_prog, "uCubeTexture");

	glUniformMatrix4fv(uMat, 1, GL_TRUE, ProjMat*ViewMat*Scale(20,20,20));
	glUniform1i(uCubeTexture, 0);
	cube.Draw(cube_prog);

	//지구 그리기
	glUseProgram(phong_prog);
	
	// 1. Define Light Properties
	// 
	vec4 lPos = vec4(20*sin(-time*0.3), 10, 20*cos(-time*0.3), 1);		 
	vec4 lAmb = vec4(0.5, 0.5, 0.5, 1);		
	vec4 lDif = vec4(1, 1, 1, 1);
	vec4 lSpc = lDif; 

	// 2. Define Material Properties
	//
	vec4 mAmb = vec4(0.3, 0.3, 0.3, 1);		
	vec4 mDif = vec4(0.8, 0.8, 0.8, 1);		
	vec4 mSpc = vec4(0.8, 0.8, 0.8, 1);	
	float mShiny = 100;										//1~100;

	// I = lAmb*mAmb + lDif*mDif*(N·L) + lSpc*mSpc*(V·R)^n; 
	vec4 amb = lAmb*mAmb;					
	vec4 dif = lDif*mDif;					
	vec4 spc = lSpc*mSpc; 

	// 3. Send Uniform Variables to the shader
	//
	GLuint uModelMat = glGetUniformLocation(phong_prog, "uModelMat");
	GLuint uViewMat = glGetUniformLocation(phong_prog, "uViewMat");
	GLuint uProjMat = glGetUniformLocation(phong_prog, "uProjMat"); 
	GLuint uLPos = glGetUniformLocation(phong_prog, "uLPos"); 
	GLuint uAmb = glGetUniformLocation(phong_prog, "uAmb"); 
	GLuint uDif = glGetUniformLocation(phong_prog, "uDif"); 
	GLuint uSpc = glGetUniformLocation(phong_prog, "uSpc"); 
	GLuint uShininess = glGetUniformLocation(phong_prog, "uShininess");
	GLuint uTime = glGetUniformLocation(phong_prog, "uTime");
	GLuint uTexture = glGetUniformLocation(phong_prog, "uTexture");
	GLuint uTexBump = glGetUniformLocation(phong_prog, "uTexBump");
	GLuint uCldTexture = glGetUniformLocation(phong_prog, "uCldTexture");
	GLuint uLgtTexture = glGetUniformLocation(phong_prog, "uLgtTexture");
	GLuint uSpecTexture = glGetUniformLocation(phong_prog, "uSpecTexture");

	glUniformMatrix4fv(uModelMat, 1, true, ModelMat); 
	glUniformMatrix4fv(uViewMat, 1, true, ViewMat); 
	glUniformMatrix4fv(uProjMat, 1, true, ProjMat); 
	glUniform4f(uLPos, lPos[0], lPos[1], lPos[2], lPos[3]); 
	glUniform4f(uAmb, amb[0], amb[1], amb[2], amb[3]); 
	glUniform4f(uDif, dif[0], dif[1], dif[2], dif[3]); 
	glUniform4f(uSpc, spc[0], spc[1], spc[2], spc[3]); 
	glUniform1f(uShininess, mShiny); 
	glUniform1i(uTexture, 1);
	glUniform1i(uTexBump, 4);
	glUniform1i(uCldTexture, 2);
	glUniform1i(uLgtTexture, 3);
	glUniform1i(uSpecTexture, 5);

	//속도
	glUniform1f(uTime, time/6);

	sphere.Draw(phong_prog);

	//달 그리기
	mat4 M(1.0);
	M = Translate(-3.5,0,0) * Scale(0.35, 0.35, 0.35);

	mat4 ModelMat2 = RotateZ(time*0.6)*myLookAt(vec3(4,0.7,4), vec3(0,0,0), vec3(0,1,0))*RotateY(time*10);
	glUseProgram(phong_prog);
	
	GLuint uModelMat2 = glGetUniformLocation(phong_prog, "uModelMat");
	
	glUniformMatrix4fv(uModelMat2, 1, true, ModelMat2*M); 
	glUniform1i(uTexture, 6);
	glUniform1i(uTexBump, 7);
	glUniform1i(uCldTexture, NULL);
	glUniform1i(uLgtTexture, NULL);
	glUniform1i(uSpecTexture, NULL);

	sphere.Draw(phong_prog);	
	
	glutSwapBuffers();
}

bool play = true;
void idle()
{
	if(play)
	{
		time += 0.1;
		Sleep(16);					// for vSync
		glutPostRedisplay();
	}
}

void keyboard(unsigned char ch, int x, int y)
{
	if(ch == ' ')
		play = ! play;

}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	
	g_aspect = w/float(h);
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(1000,600);

	glutCreateWindow("우주우우");

	glewExperimental = true;
	glewInit();

	printf("OpenGL %s, GLSL %s\n",	glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	myInit();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}

