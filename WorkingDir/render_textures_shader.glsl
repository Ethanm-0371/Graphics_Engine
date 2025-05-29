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
	int reflectiveness;
};

out vec2 vTexCoord;
out vec3 vPosition; //In worldspace
out vec3 vNormal; //In worldspace
out vec3 vViewDir;

out float entityReflectiveness;

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
	vNormal = vec3( uWorldMatrix * vec4(aNormal, 0.0) );
	vViewDir = uCameraPosition - vPosition;
	entityReflectiveness = float(reflectiveness) / 100.0f;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;

in float entityReflectiveness;

uniform sampler2D uTexture;
uniform samplerCube cubeTexture;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout(location = 0) out vec4 rt0; //Albedo
layout(location = 1) out vec4 rt1; //Normals
layout(location = 2) out vec4 rt2; //Position
//layout(location = 3) out vec4 rt3; //Deferred

void main()
{
	//Skybox reflection
	vec3 I = normalize(vPosition - uCameraPosition);
	vec3 R = reflect(I, normalize(vNormal));
	vec3 skyboxColor = texture(cubeTexture, R).rgb;
	vec3 albedoColor = texture(uTexture, vTexCoord).rgb;

	rt0 = vec4(mix(albedoColor, skyboxColor, entityReflectiveness), 1.0);
	rt1 = vec4(normalize(vNormal), 1.0);
	rt2 = vec4(vPosition, 1.0);

	//rt3 = vec4(1.0, 1.0, 0.0, 1.0);
}

#endif
#endif

////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////

#ifdef DEFERRED_LIGHTING_PASS

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D albedoTexture;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

layout(location = 0) out vec4 oColor;

void main()
{
	vec3 position = texture(positionTexture, vTexCoord).rgb;
	vec3 norm = texture(normalTexture, vTexCoord).rgb;
	vec3 texColor = texture(albedoTexture, vTexCoord).rgb;

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
			lightDir = normalize(uLight[i].position - position);
		}

		//Diffuse
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * uLight[i].color * uLight[i].strength;

		//Specular
		float specularStrength = 0.5f;
		int specularShininess = 8;
		vec3 viewDir = normalize(uCameraPosition - position);
		vec3 reflectDir = reflect(-lightDir, norm);

		float spec = pow(max(dot(viewDir, reflectDir), 0.0), pow(specularShininess, 2));
		vec3 specular = specularStrength * spec * uLight[i].color * uLight[i].strength;

		//Final
		result += (ambient + diffuse + specular) * texColor;
	}
	
    oColor = vec4(result, 1.0);
}

#endif
#endif