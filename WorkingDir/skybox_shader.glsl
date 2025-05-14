#ifdef SKYBOX

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;

out vec3 vTexCoord;

uniform mat4 uWorldViewProjectionMatrix;

void main()
{
	vTexCoord = aPosition;
	gl_Position = uWorldViewProjectionMatrix *  vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec3 vTexCoord;
uniform samplerCube uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture, vTexCoord);
}

#endif
#endif