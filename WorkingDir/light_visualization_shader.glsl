#ifdef LIGHT_VISUALIZATION

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;

layout(binding = 0, std140) uniform LocalParams
{
	mat4 uWorldViewProjectionMatrix;
};

void main()
{
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

uniform vec3 lightColor;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = vec4(lightColor, 1);
}

#endif
#endif