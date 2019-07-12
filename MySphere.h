#pragma once

#include <vgl.h>
#include <vec.h>

struct MySphereVertex
{
	vec4 position;
	vec4 color;
	vec3 normal;
	vec2 texCoord;
	vec3 tangent;
};

class MySphere
{
public:
	MySphere(void);
	~MySphere(void);

	int NumVertices;

	MySphereVertex * Vertices;
	GLuint Init(int la_slice, int lo_slice, vec4 color=vec4(0.5,0.5,0.5,1));
	void SetPositionAndOtherAttributes(GLuint program);

	GLuint vao;
	GLuint buffer;
	bool bInitialized;

	void Draw(GLuint program);
};

MySphere::MySphere(void)
{
	bInitialized = false;
	NumVertices = 0;
	Vertices = NULL;
}

MySphere::~MySphere(void)
{
	if(Vertices != NULL) 
		delete [] Vertices;
}

GLuint MySphere::Init(int la_slice, int lo_slice, vec4 color)
{
	// The Cube should be initialized only once;
	if(bInitialized == true) return vao;

	NumVertices = (la_slice-2)*lo_slice*2*3 + 2*lo_slice*3;
	Vertices = new MySphereVertex [NumVertices];

	float da = 3.141592/la_slice;
	float db = 3.141592*2.0/lo_slice;
	int cur = 0;

	for(int i=0; i<la_slice; i++)
	{
		float y1 = cos(da*i);
		float r1 = sqrt(1-y1*y1);
		float y2 = cos(da*(i+1));
		float r2 = sqrt(1-y2*y2);
		for(int j=0; j<lo_slice; j++)
		{
			vec3 a(r1*cos(db*j), y1, r1*sin(db*j));
			vec3 b(r2*cos(db*j), y2, r2*sin(db*j));
			vec3 c(r2*cos(db*(j+1)), y2, r2*sin(db*(j+1)));
			vec3 d(r1*cos(db*(j+1)), y1, r1*sin(db*(j+1)));

		
			vec2 ta(1-(j)/float(lo_slice), 1-(i)/float(la_slice));
			vec2 tb(1-(j)/float(lo_slice), 1-(i+1)/float(la_slice));
			vec2 tc(1-(j+1)/float(lo_slice), 1-(i+1)/float(la_slice));
			vec2 td(1-(j+1)/float(lo_slice), 1-(i)/float(la_slice));
						
			vec3 ga(sin(db*j), 0, -cos(db*j));
			vec3 gb(sin(db*j), 0, -cos(db*j));
			vec3 gc(sin(db*(j+1)), 0, -cos(db*(j+1)));
			vec3 gd(sin(db*(j+1)), 0, -cos(db*(j+1)));

			if(i!=lo_slice-1)
			{
				Vertices[cur].position = a;	Vertices[cur].color = color; Vertices[cur].normal = a; Vertices[cur].texCoord = ta; 
				Vertices[cur].tangent = ga; cur ++;
				Vertices[cur].position = b;	Vertices[cur].color = color; Vertices[cur].normal = b; Vertices[cur].texCoord = tb;
				Vertices[cur].tangent = gb; cur ++;
				Vertices[cur].position = c;	Vertices[cur].color = color; Vertices[cur].normal = c; Vertices[cur].texCoord = tc;
				Vertices[cur].tangent = gc; cur ++;
			}
			if(i!=0)
			{
				Vertices[cur].position = c;	Vertices[cur].color = color; Vertices[cur].normal = c; Vertices[cur].texCoord = tc;
				Vertices[cur].tangent = gc; cur ++;
				Vertices[cur].position = d;	Vertices[cur].color = color; Vertices[cur].normal = d; Vertices[cur].texCoord = td;
				Vertices[cur].tangent = gd; cur ++;
				Vertices[cur].position = a;	Vertices[cur].color = color; Vertices[cur].normal = a; Vertices[cur].texCoord = ta;
				Vertices[cur].tangent = ga; cur ++;
			}
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MySphereVertex)*NumVertices, Vertices, GL_STATIC_DRAW);

	bInitialized = true;
	return vao;

}

void MySphere::SetPositionAndOtherAttributes(GLuint program)
{
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(MySphereVertex), BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, sizeof(MySphereVertex), BUFFER_OFFSET(sizeof(vec4)));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(MySphereVertex), BUFFER_OFFSET(sizeof(vec4)+sizeof(vec4)));

	GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(MySphereVertex), BUFFER_OFFSET(sizeof(vec4)+sizeof(vec4)+sizeof(vec3)));

	GLuint vTangent = glGetAttribLocation(program, "vTangent");
	glEnableVertexAttribArray(vTangent);
	glVertexAttribPointer(vTangent, 3, GL_FLOAT, GL_FALSE, sizeof(MySphereVertex), BUFFER_OFFSET(sizeof(vec4)+sizeof(vec4)+sizeof(vec3)+sizeof(vec2)));

}


void MySphere::Draw(GLuint program)
{
	if(!bInitialized) return;			// check whether it is initiazed or not. 
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	SetPositionAndOtherAttributes(program);
	
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

}