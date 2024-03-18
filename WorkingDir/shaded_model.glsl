///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef SHADED_MODEL

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition; // www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)
// layout(location = 1) in vec2 aNormal;
layout(location = 1) in vec2 aTextCoord;
// layout(location = 3) in vec2 aTangent;
// layout(location = 4) in vec2 aBitangent;

layout(location = 5) out vec2 vTextCoord;

void main()
{
    vTextCoord = aTextCoord;
    
    float clippingScale = 5.0;
    gl_Position = vec4(aPosition, clippingScale);
    
    // Patrick looks away from the camera by default
    gl_Position.z = -gl_Position.z;
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

uniform sampler2D uTexture; // www.khronos.org/opengl/wiki/Uniform_(GLSL)

layout(location = 5) in vec2 sTextCoord;

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = texture(uTexture, sTextCoord);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
