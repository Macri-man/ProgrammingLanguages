GLuint framebuffer;
GLuint textureColorBuffer;

glGenFramebuffers(1, &framebuffer);
glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

// Create a texture to hold the color buffer
glGenTextures(1, &textureColorBuffer);
glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

// Check if framebuffer is complete
if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Framebuffer is not complete!" << std::endl;
}
glBindFramebuffer(GL_FRAMEBUFFER, 0);


glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
// Clear the framebuffer
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Render your scene here
glUseProgram(shaderProgramBasic);
// Set uniforms, bind VAO, and draw your objects

glBindFramebuffer(GL_FRAMEBUFFER, 0); // Switch back to default framebuffer


// Vertices for a full-screen quad
float quadVertices[] = {
    // positions      // texture coords
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f
};

GLuint quadVAO, quadVBO;
glGenVertexArrays(1, &quadVAO);
glGenBuffers(1, &quadVBO);
glBindVertexArray(quadVAO);
glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
glEnableVertexAttribArray(1);


//blur effect

#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D screenTexture;
uniform float blurSize;

void main()
{
    vec2 texOffsets[9] = vec2[](
        vec2(-1.0,  1.0) * blurSize,
        vec2( 0.0,  1.0) * blurSize,
        vec2( 1.0,  1.0) * blurSize,
        vec2(-1.0,  0.0) * blurSize,
        vec2( 0.0,  0.0) * blurSize,
        vec2( 1.0,  0.0) * blurSize,
        vec2(-1.0, -1.0) * blurSize,
        vec2( 0.0, -1.0) * blurSize,
        vec2( 1.0, -1.0) * blurSize
    );

    vec3 result = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        vec2 offset = texOffsets[i];
        result += texture(screenTexture, TexCoord.st + offset).rgb;
    }
    result /= 9.0;
    FragColor = vec4(result, 1.0);
}


glUseProgram(shaderProgramPostProcessing);
glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
glBindVertexArray(quadVAO);
glDrawArrays(GL_TRIANGLES, 0, 6);