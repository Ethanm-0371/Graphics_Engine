#ifdef BLUR_PASS

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
  
uniform sampler2D brightColorImage;

uniform float strength;
uniform int iterations;

layout(location = 0) out vec4 oColor;

void main()
{             
	vec2 tex_offset = 1.0 / textureSize(brightColorImage, 0);
    vec3 result = texture(brightColorImage, vTexCoord).rgb * exp(-(pow(1,2)/strength));

	for(int x = 0; x < 10; ++x)
	{
		for(int i = 1; i < iterations; ++i)
		{
			result += texture(brightColorImage, vTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * exp(-(pow(i,2)/pow(strength,2)));
			result += texture(brightColorImage, vTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * exp(-(pow(i,2)/pow(strength,2)));
		}

		for(int i = 1; i < iterations; ++i)
		{
			result += texture(brightColorImage, vTexCoord + vec2(0.0, tex_offset.y * i)).rgb * exp(-(pow(i,2)/pow(strength,2)));
			result += texture(brightColorImage, vTexCoord - vec2(0.0, tex_offset.y * i)).rgb * exp(-(pow(i,2)/pow(strength,2)));
		}
	}

    oColor = vec4(result, 1.0);
}
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef MIX_BLOOM

#if defined(VERTEX) ///////////////////////////////////////////////////

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
  
uniform sampler2D blurredImage;
uniform sampler2D originalColor;

layout(location = 0) out vec4 oColor;

void main()
{
	float exposure = 2;
	const float gamma = 0.4;
    vec3 hdrColor = texture(originalColor, vTexCoord).rgb;      
    vec3 bloomColor = texture(blurredImage, vTexCoord).rgb;
    hdrColor += bloomColor; // additive blending
	
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
	   
    result = pow(result, vec3(1.0 / gamma));
    oColor = vec4(result, 1.0);
}
#endif
#endif