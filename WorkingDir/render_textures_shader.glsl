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
	vNormal = vec3( uWorldMatrix * vec4(aNormal, 0.0) );
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

layout(location = 0) out vec4 rt0; //Albedo
layout(location = 1) out vec4 rt1; //Normals
layout(location = 2) out vec4 rt2; //Position
//layout(location = 3) out vec4 rt3; //Deferred

void main()
{
	rt0 = texture(uTexture, vTexCoord);
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
	// retrieve data from G-buffer
    vec3 FragPos = texture(positionTexture, vTexCoord).rgb;
    vec3 Normal = texture(normalTexture, vTexCoord).rgb;
    vec3 Albedo = texture(albedoTexture, vTexCoord).rgb;
    
    // then calculate lighting as usual
    vec3 lighting = Albedo * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(uCameraPosition - FragPos);
    for(int i = 0; i < uLightCount; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(uLight[i].position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * uLight[i].color;
        lighting += diffuse;
    }
    
    oColor = vec4(lighting, 1.0);
    //oColor = vec4(1,1,0, 1.0);
}

#endif
#endif