#version 330 core

// eli5

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;
layout (location = 4) in mat4 iMat;

out vec3 ourColor;
out vec2 TexCoord;
out vec4 varyingColor;

out vec3 FragPos;
out vec3 Normal;

uniform float offset;
uniform mat4 pMat,vMat,mMat,mMat3,mMat2;

out vec3 viewPos;

void main()
{
    viewPos =-vec3(vMat[3][0],vMat[3][1],vMat[3][2]);

    FragPos = vec3(mMat*mMat2*iMat * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(mMat*mMat2*iMat))) * aNormal; 

    gl_Position = pMat*vMat*mMat*mMat2*iMat*vec4(aPos, 1.0);
	
    ourColor = vec3(0.5f,0.0f,0.0f);

    TexCoord = aTexCoord;
}













