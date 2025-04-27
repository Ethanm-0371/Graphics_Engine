struct Light
{
	unsigned int type;
	unsigned int strength;
	vec3 color;
	vec3 direction;
	vec3 position;
};

#ifdef LIGHT_VISUALIZATION

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition; //In worldspace

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;
in vec3 vPosition;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = vec4(1,1,0,0);
}

#endif
#endif