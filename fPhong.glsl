#version 330

in vec3 N3; 
in vec3 L3; 
in vec3 V3; 
in vec2 texCoord;
in vec3 T3;

out vec4 fColor;

uniform mat4 uModelMat; 
uniform mat4 uProjMat; 
uniform vec4 uLPos; 
uniform vec4 uAmb; 
uniform vec4 uDif; 
uniform vec4 uSpc; 
uniform float uShininess; 

uniform samplerCube uCubeTexture;

uniform sampler2D uTexture;
uniform sampler2D uTexBump;
uniform sampler2D uCldTexture;
uniform sampler2D uLgtTexture;
uniform sampler2D uSpecTexture;

uniform float uTime;

void main()
{
	//지구, 달
	vec4 tex = texture2D(uTexture, texCoord);

	//구름
	vec4 cld = texture2D(uCldTexture, texCoord+vec2(-uTime/10,0));
	//그림자
	vec4 sdw = texture2D(uCldTexture, texCoord+vec2(-uTime/10+V3.x*0.1,0));

	//밤조명
	vec4 lgt = texture2D(uLgtTexture, texCoord);

	//바다
    vec4 spec = texture2D(uSpecTexture, texCoord); 

	//구름 우주 반사
	vec4 refl = texture(uCubeTexture, (N3-L3)); 

	vec3 N = normalize(N3); 

	vec3 T = normalize(T3);
	vec3 B = cross(N, T);
	vec4 slope = texture2D(uTexBump, texCoord);
	float du = (slope.r-0.5)*2;
	float dv = (slope.b-0.5)*2;
	//bump 세기
	N = normalize(N - (du*T + dv*B)*2);

	vec3 L = normalize(L3); 
	vec3 V = normalize(V3); 
	vec3 H = normalize(V+L); 
	
    float NL = 1-pow(1-max(dot(N, L), 0),4); 
	float NL2 = max(-dot(N, L), 0); 
	float VR = pow(max(dot(H, N), 0), uShininess); 
	
	float w = cld.r;
	float w1 = 1-0.8*sdw.r;
	tex = (1-w)*tex*w1 + w*vec4(1,1,1,1);

	float w2 = spec.r;
	tex += ((1-w2)*tex + w2*vec4(1,1,1,1))*0.2;

	fColor = uAmb + tex*0.8*NL + lgt*NL2*2 + uSpc*VR*(w/2+w2/2) + refl*0.8*NL2*w;

}
