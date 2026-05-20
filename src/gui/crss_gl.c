#include "crss_gl.h"

typedef struct {
    float x, y;
} offset_t;

typedef struct {
    uint16_t x, y, z;
    uint8_t type;
} rendered_node_t;

typedef struct {
    uint16_t x1, y1, z1;
    uint16_t x2, y2, z2;
} rendered_edge_t;

static char compile_log[512];

bool compile_shader(char *fnpath, const char *shadername, uint32_t shader_id) {
    FUNCPATH("compile_shader");
    int success;
    glCompileShader(shader_id);
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader_id, 512, NULL, compile_log);
        LFATAL("%s shader failed to compile!", shadername);
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

static rendered_node_t sample_nodes[10] = {
    { .x = 0, .y = 0, .z = 7, .type = 0 },
    { .x = 1, .y = 0, .z = 7, .type = 0 },
    { .x = 2, .y = 0, .z = 7, .type = 0 },
    { .x = 3, .y = 0, .z = 7, .type = 0 },
    { .x = 3, .y = 0, .z = 6, .type = 0 },
    { .x = 3, .y = 0, .z = 5, .type = 0 },
    { .x = 3, .y = 0, .z = 4, .type = 0 },
    { .x = 3, .y = 0, .z = 3, .type = 0 },
    { .x = 3, .y = 0, .z = 2, .type = 0 },
    { .x = 3, .y = 0, .z = 1, .type = 0 }
};

static rendered_edge_t sample_edges[3] = {
    { .x1 = 1, .y1 = 0, .z1 = 7, .x2 = 0, .y2 = 0, .z2 = 7 },
    { .x1 = 1, .y1 = 0, .z1 = 7, .x2 = 2, .y2 = 0, .z2 = 7 },
    { .x1 = 2, .y1 = 0, .z1 = 7, .x2 = 3, .y2 = 0, .z2 = 6 }
};

static int32_t create_shader_program(
    char *fnpath, const char *vshader_path, const char *fshader_path
) {
    FUNCPATH("create_shaders");
    // Shader sources
    char *vshader_src = load_file(vshader_path);
    char *fshader_src = load_file(fshader_path);

    // Create shaders
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertexShader, 1, (const char**)&vshader_src, NULL);
    glShaderSource(fragmentShader, 1, (const char**)&fshader_src, NULL);
    if (!compile_shader((char*)fnpath, "Vertex", vertexShader)) goto err;
    if (!compile_shader((char*)fnpath, "Fragment", fragmentShader)) goto err;

    // Shader program
    uint32_t shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    if (!link_program((char*)fnpath, shaderProgram)) goto err;

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free(vshader_src);
    free(fshader_src);
    return shaderProgram;

    err:
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free(vshader_src);
    free(fshader_src);
    return -1;
}

void setup_openGL(CrssAppWindow *win) {
    GUI_PATH("graph_area_realize.setup_openGL");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    win->node_shader_program = create_shader_program(
        (char*)fnpath, "src/gui/shaders/vs_node.glsl", "src/gui/shaders/fs_node.glsl"
    );
    win->edge_shader_program = create_shader_program(
        (char*)fnpath, "src/gui/shaders/vs_edge.glsl", "src/gui/shaders/fs_edge.glsl"
    );

    /* Nodes */

    static offset_t node_quad_offsets[4] = {
        { .x =  0.5, .y =  0.5 },
        { .x =  0.5, .y = -0.5 },
        { .x = -0.5, .y =  0.5 },
        { .x = -0.5, .y = -0.5 }
    };

    // Setup node (instance) VBO
    uint32_t nodeVBO;
    glGenBuffers(1, &nodeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, nodeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sample_nodes), sample_nodes, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Setup Quad VBO + VAO
    uint32_t nodeQuadVBO;
    glGenBuffers(1, &nodeQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, nodeQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(node_quad_offsets), node_quad_offsets, GL_STATIC_DRAW);

    uint32_t nodeQuadVAO;
    glGenVertexArrays(1, &nodeQuadVAO);
    glBindVertexArray(nodeQuadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(offset_t), (void*)offsetof(offset_t, x));
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, nodeVBO); // !
    glVertexAttribIPointer(1, 3, GL_UNSIGNED_SHORT, sizeof(rendered_node_t), (void*)offsetof(rendered_node_t, x));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(rendered_node_t), (void*)offsetof(rendered_node_t, type));

    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    win->node_vao = nodeQuadVAO;

    /* Edges */

    static offset_t edge_quad_offsets[4] = {
        { .x = 0.0, .y =  1.0 },
        { .x = 0.0, .y = -1.0 },
        { .x = 1.0, .y =  1.0 },
        { .x = 1.0, .y = -1.0 }
    };

    // Setup edge (instance) VBO
    uint32_t edgeVBO;
    glGenBuffers(1, &edgeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sample_edges), sample_edges, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Setup Quad VBO + VAO
    uint32_t edgeQuadVBO;
    glGenBuffers(1, &edgeQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(edge_quad_offsets), edge_quad_offsets, GL_STATIC_DRAW);

    uint32_t edgeQuadVAO;
    glGenVertexArrays(1, &edgeQuadVAO);
    glBindVertexArray(edgeQuadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(offset_t), (void*)offsetof(offset_t, x));
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, edgeVBO); // !
    glVertexAttribIPointer(1, 3, GL_UNSIGNED_SHORT, sizeof(rendered_edge_t), (void*)offsetof(rendered_edge_t, x1));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 3, GL_UNSIGNED_SHORT, sizeof(rendered_edge_t), (void*)offsetof(rendered_edge_t, x2));

    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    win->edge_vao = edgeQuadVAO;

    // Setup camera
    init_camera(&win->camera);
}

gboolean render_graph_area(GtkGLArea *area, GdkGLContext *ctx, gpointer user_data) {

    /* Setup */

    CrssAppWindow *win = user_data;

    rs_graph_t *graph = win->graph;

    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Camera matrices
    mat4 view, proj;
    get_view_matrix(&win->camera, view);
    get_projection_matrix(&win->camera, proj);

    int viewMatLocation = glGetUniformLocation(win->node_shader_program, "view");
    int projMatLocation = glGetUniformLocation(win->node_shader_program, "projection");

    /* Draw */

    /* Nodes */
    glUseProgram(win->node_shader_program);
    glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, (float*)view);
    glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, (float*)proj);
    glBindVertexArray(win->node_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, sizeof(sample_nodes)/sizeof(rendered_node_t));
    glBindVertexArray(0);

    /* Edges */
    glUseProgram(win->edge_shader_program);
    glUniformMatrix4fv(viewMatLocation, 1, GL_FALSE, (float*)view);
    glUniformMatrix4fv(projMatLocation, 1, GL_FALSE, (float*)proj);
    glBindVertexArray(win->edge_vao);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, sizeof(sample_edges)/sizeof(rendered_edge_t));
    glBindVertexArray(0);

    return true;
}
