#version 330 core
in vec3 ourColor;
in vec2 TexCoord;

out vec4 color;

// Texture samplers
uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
	// Linearly interpolate between both textures (second texture is only slightly combined)
	color = mix(texture(ourTexture1, TexCoord), texture(ourTexture2, TexCoord), 0.2);
    color = mix(color, vec4(ourColor,1.0f), 0.05);

    //color = vec4(vec3(gl_FragCoord.z), 1.0f);    //输出颜色缓冲区的值
}