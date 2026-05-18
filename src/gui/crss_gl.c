#include "crss_gl.h"

const char *vshader_src = ""
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "}\0";

const char *fshader_src = ""
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

typedef struct {
    float x, y, z;
} vertex_t;

static char compile_log[512];

bool compile_shader(char *fnpath, uint32_t shader_id) {
    FUNCPATH("compile_shader");
    int success;
    glCompileShader(shader_id);
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader_id, 512, NULL, compile_log);
        LFATAL("Shader failed to compile!");
        LFATAL("Log: %s", compile_log);
        return false;
    }
    return true;
}

bool link_program(char *fnpath, uint32_t program_id) {
    FUNCPATH("link_program");
    int success;
    glLinkProgram(program_id);
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program_id, 512, NULL, compile_log);
        LFATAL("Shader program failed to link!");
        LFATAL("Log: %s", compile_log);
        return false;
    }
    return true;
}

void setup_openGL(CrssAppWindow *win) {
    GUI_PATH("graph_area_realize.setup_openGL");

    static vertex_t vertices[2] = {
        { .x = 0, .y = 0, .z = 0.5 },
        { .x = 0.7, .y = -0.3, .z = 0.5 }
    };


    // Vertex shader
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vshader_src, NULL);
    if (!compile_shader((char*)fnpath, vertexShader)) return;

    // Fragment shader
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fshader_src, NULL);
    if (!compile_shader((char*)fnpath, fragmentShader)) return;

    // Shader program
    uint32_t shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    if (!link_program((char*)fnpath, shaderProgram)) return;
    win->graph_shader_program = shaderProgram;

    // Cleanup shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // Setup VBO
    uint32_t VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Setup VAO
    uint32_t VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, x));
    glEnableVertexAttribArray(0);
    win->graph_vao = VAO;
}

gboolean render_graph_area(GtkGLArea *area, GdkGLContext *ctx, gpointer user_data) {

    CrssAppWindow *win = user_data;

    rs_graph_t *graph = win->graph;

    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(win->graph_shader_program);
    glBindVertexArray(win->graph_vao);
    glDrawArrays(GL_POINTS, 0, 2);

    return true;
}
