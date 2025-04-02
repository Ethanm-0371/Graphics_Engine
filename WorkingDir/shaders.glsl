///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
struct Light
{
	unsigned int type;
	unsigned int strength;
	vec3 color;
	vec3 direction;
	vec3 position;
};

#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture, vTexCoord);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.

#ifdef SHOW_TEXTURED_MESH

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

layout(location = 0) out vec4 oColor;

void main()
{
	//Ambient
	float ambientStrength = 0.2;
	vec3 ambientColor = vec3(1.0f);
    vec3 ambient = ambientStrength * ambientColor;

	vec3 result = vec3(0);
	
	for(int i = 0; i < uLightCount; i++)
	{
		//Check if point or dir
		vec3 lightDir; 
		if (uLight[i].type == 0) //Directional
		{
			lightDir = normalize(uLight[i].direction);
		}
		else if (uLight[i].type == 1)
		{
			lightDir = normalize(uLight[i].position - vPosition);
		}

		//Diffuse
		vec3 norm = normalize(vNormal);

		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * uLight[i].color * uLight[i].strength;

		//Specular
		float specularStrength = 0.5f;
		int specularShininess = 8;
		vec3 viewDir = normalize(uCameraPosition - vPosition);
		vec3 reflectDir = reflect(-lightDir, norm);

		float spec = pow(max(dot(viewDir, reflectDir), 0.0), pow(specularShininess, 2));
		vec3 specular = specularStrength * spec * uLight[i].color * uLight[i].strength;

		//Final
		vec3 texColor = texture(uTexture, vTexCoord).rgb;

		result += (ambient + diffuse + specular) * texColor;
	}

	oColor = vec4(result, 1.0);
}

#endif
#endif