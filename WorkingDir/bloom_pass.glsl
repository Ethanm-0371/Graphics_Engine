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

layout(location = 0) out vec4 oColor;

void main()
{             
	float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec2 tex_offset = 1.0 / textureSize(brightColorImage, 0);
    vec3 result = texture(brightColorImage, vTexCoord).rgb * weight[0];

	for(int i = 1; i < 5; ++i)
	{
		result += texture(brightColorImage, vTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		result += texture(brightColorImage, vTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
	}

	for(int i = 1; i < 5; ++i)
	{
		result += texture(brightColorImage, vTexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		result += texture(brightColorImage, vTexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
	}    

    oColor = vec4(result, 1.0);
}
#endif
#endif