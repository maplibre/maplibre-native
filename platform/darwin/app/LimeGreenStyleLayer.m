#import "LimeGreenStyleLayer.h"
#import <GLKit/GLKit.h>

@implementation LimeGreenStyleLayer {
    GLuint _program;
    GLuint _vertexShader;
    GLuint _fragmentShader;
    GLuint _buffer;
    GLuint _aPos;
}

- (void)didMoveToMapView:(MLNMapView *)mapView {
    static const GLchar *vertexShaderSource = "#version 300 es\nlayout (location = 0) in vec2 a_pos; void main() { gl_Position = vec4(a_pos, 1, 1); }";
    static const GLchar *fragmentShaderSource = "#version 300 es\nout highp vec4 fragColor; void main() { fragColor = vec4(0, 0.5, 0, 0.5); }";

    _program = glCreateProgram();
    _vertexShader = glCreateShader(GL_VERTEX_SHADER);
    _fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(_vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(_vertexShader);
    glAttachShader(_program, _vertexShader);
    glShaderSource(_fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(_fragmentShader);
    glAttachShader(_program, _fragmentShader);
    glLinkProgram(_program);
    _aPos = glGetAttribLocation(_program, "a_pos");

    GLfloat triangle[] = { 0, 0.5, 0.5, -0.5, -0.5, -0.5 };
    glGenBuffers(1, &_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, _buffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), triangle, GL_STATIC_DRAW);
}

- (void)drawInMapView:(MLNMapView *)mapView withContext:(MLNStyleLayerDrawingContext)context {
    glUseProgram(_program);
    glBindBuffer(GL_ARRAY_BUFFER, _buffer);
    glEnableVertexAttribArray(_aPos);
    glVertexAttribPointer(_aPos, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
}

- (void)willMoveFromMapView:(MLNMapView *)mapView {
    if (!_program) {
        return;
    }

    glDeleteBuffers(1, &_buffer);
    glDetachShader(_program, _vertexShader);
    glDetachShader(_program, _fragmentShader);
    glDeleteShader(_vertexShader);
    glDeleteShader(_fragmentShader);
    glDeleteProgram(_program);
}

@end
