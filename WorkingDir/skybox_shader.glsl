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

#ifdef SKYBOX_REFLECTION

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;

out vec3 vPosition;
out vec3 vNormal;

uniform mat4 uWorldMatrix;
uniform mat4 uWorldViewProjectionMatrix;

void main()
{
	vNormal = mat3(transpose(inverse(uWorldMatrix))) * aNormal;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	gl_Position = uWorldViewProjectionMatrix *  vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 oColor;

in vec3 vPosition;
in vec3 vNormal;

uniform vec3 uCameraPosition;
uniform samplerCube uTexture;

void main()
{
	//oColor = texture(uTexture, vTexCoord);
	vec3 I = normalize(vPosition - uCameraPosition);
	vec3 R = reflect(I, normalize(vNormal));
	oColor = vec4(texture(uTexture, R).rgb, 1.0);
}

#endif
#endif
