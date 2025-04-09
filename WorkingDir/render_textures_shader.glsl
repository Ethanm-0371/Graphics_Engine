struct Light
{
	unsigned int type;
	unsigned int strength;
	vec3 color;
	vec3 direction;
	vec3 position;
};

#ifdef RENDER_TEXTURES

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBitangent;

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
out vec3 vNormal; //In worldspace
out vec3 vViewDir;

void main()
{
	vTexCoord = aTexCoord;

	// We will usually not define the clipping scale manually...
	// it is usually computed by the projection matrix. Because
	// we are not passing uniform transforms yet, we increase 
	// the clipping scale so that Patrick fits the screen.
	//float clippingScale = 5.0;
	
	vTexCoord = aTexCoord;
	vPosition = vec3( uWorldMatrix * vec4(aPosition, 1.0) );
	vNormal = vec3( uWorldMatrix * vec4(aNormal, 1.0) );
	vViewDir = uCameraPosition - vPosition;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout(location = 0) out vec4 rt0; //Albedo, AO
layout(location = 1) out vec4 rt1; //Specular, roughness
layout(location = 2) out vec4 rt2; //Normals
layout(location = 3) out vec4 rt3; //Emissive + lightmaps

void main()
{
	rt0 = texture(uTexture, vTexCoord);
	rt1 = vec4(1,0,0,1);
	rt2 = vec4(0,1,0,1);
	rt3 = vec4(0,1,1,1);
}

#endif
#endif