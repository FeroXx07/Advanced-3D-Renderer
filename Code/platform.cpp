//
// platform.cpp : This file contains the 'main' function. Program execution begins and ends there.
// The platform layer is in charge to create the environment necessary so the engine disposes of what
// it needs in order to create the application (e.g. window, graphics context, I/O, allocators, etc).
//

#ifdef _WIN32
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "engine.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include "errors_support.h"
#include <ImGuizmo.h>

#define WINDOW_TITLE  "Advanced Graphics Programming"
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define GLOBAL_FRAME_ARENA_SIZE MB(16)
u8* GlobalFrameArenaMemory = NULL;
u32 GlobalFrameArenaHead = 0;

void OnGlfwError(int errorCode, const char *errorMessage)
{
	fprintf(stderr, "glfw failed with error %d: %s\n", errorCode, errorMessage);
}

void OnGlfwMouseMoveEvent(GLFWwindow* window, double xpos, double ypos)
{
    App* app = (App*)glfwGetWindowUserPointer(window);
    app->input.mouseDelta.x = xpos - app->input.mousePos.x;
    app->input.mouseDelta.y = ypos - app->input.mousePos.y;
    app->input.mousePos.x = xpos;
    app->input.mousePos.y = ypos;
}

void OnGlfwMouseEvent(GLFWwindow* window, int button, int event, int modifiers)
{
    App* app = (App*)glfwGetWindowUserPointer(window);

    switch (event) {
        case GLFW_PRESS:
            switch (button) {
                case GLFW_MOUSE_BUTTON_RIGHT: app->input.mouseButtons[RIGHT] = BUTTON_PRESS; break;
                case GLFW_MOUSE_BUTTON_LEFT:  app->input.mouseButtons[LEFT]  = BUTTON_PRESS; break;
            default: ;
            } break;
        case GLFW_REPEAT:
            switch (button) {
            case GLFW_MOUSE_BUTTON_RIGHT: app->input.mouseButtons[RIGHT] = BUTTON_PRESSED; break;
            case GLFW_MOUSE_BUTTON_LEFT:  app->input.mouseButtons[LEFT]  = BUTTON_PRESSED; break;
            default: ;
            } break;
        case GLFW_RELEASE:
            switch (button) {
                case GLFW_MOUSE_BUTTON_RIGHT: app->input.mouseButtons[RIGHT] = BUTTON_RELEASE; break;
                case GLFW_MOUSE_BUTTON_LEFT:  app->input.mouseButtons[LEFT]  = BUTTON_RELEASE; break;
            default: ;
            } break;
        default: ;
    }
}

void OnGlfwScrollEvent(GLFWwindow* window, double xoffset, double yoffset)
{
    // Nothing do yet... maybe zoom in/out in the future?
}

void OnGlfwKeyboardEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //std::cout << "Key: " << key << ", Scancode: " << scancode << ", Action: " << action << "\n";
    // Remap key to our enum values
    switch (key) {
        case GLFW_KEY_SPACE:  key = K_SPACE; break;
        case GLFW_KEY_0: key = K_0; break; case GLFW_KEY_1: key = K_1; break; case GLFW_KEY_2: key = K_2; break;
        case GLFW_KEY_3: key = K_3; break; case GLFW_KEY_4: key = K_4; break; case GLFW_KEY_5: key = K_5; break;
        case GLFW_KEY_6: key = K_6; break; case GLFW_KEY_7: key = K_7; break; case GLFW_KEY_8: key = K_8; break;
        case GLFW_KEY_9: key = K_9; break;
        case GLFW_KEY_A: key = K_A; break; case GLFW_KEY_B: key = K_B; break; case GLFW_KEY_C: key = K_C; break;
        case GLFW_KEY_D: key = K_D; break; case GLFW_KEY_E: key = K_E; break; case GLFW_KEY_F: key = K_F; break;
        case GLFW_KEY_G: key = K_G; break; case GLFW_KEY_H: key = K_H; break; case GLFW_KEY_I: key = K_I; break;
        case GLFW_KEY_J: key = K_J; break; case GLFW_KEY_K: key = K_K; break; case GLFW_KEY_L: key = K_L; break;
        case GLFW_KEY_M: key = K_M; break; case GLFW_KEY_N: key = K_N; break; case GLFW_KEY_O: key = K_O; break;
        case GLFW_KEY_P: key = K_P; break; case GLFW_KEY_Q: key = K_Q; break; case GLFW_KEY_R: key = K_R; break;
        case GLFW_KEY_S: key = K_S; break; case GLFW_KEY_T: key = K_T; break; case GLFW_KEY_U: key = K_U; break;
        case GLFW_KEY_V: key = K_V; break; case GLFW_KEY_W: key = K_W; break; case GLFW_KEY_X: key = K_X; break;
        case GLFW_KEY_Y: key = K_Y; break; case GLFW_KEY_Z: key = K_Z; break;
        case GLFW_KEY_ESCAPE: key = K_ESCAPE; break;
        case GLFW_KEY_ENTER:  key = K_ENTER; break;
    default: ;
    }

    App* app = (App*)glfwGetWindowUserPointer(window);
    switch (action) {
        case GLFW_PRESS:   app->input.keys[key] = BUTTON_PRESS; break;
        case GLFW_REPEAT:   app->input.keys[key] = BUTTON_PRESSED; break;
        case GLFW_RELEASE: app->input.keys[key] = BUTTON_RELEASE; break;
    default: ;
    }
}

void OnGlfwCharEvent(GLFWwindow* window, unsigned int character)
{
    // Nothing to do yet
}

void OnGlfwResizeFramebuffer(GLFWwindow* window, int width, int height)
{
    App* app = (App*)glfwGetWindowUserPointer(window);

    if (app->displaySizePrevious != app->displaySizeCurrent)
        app->displaySizePrevious = app->displaySizeCurrent;
    
    app->displaySizeCurrent = vec2(glm::max(width, 1), glm::max(height, 1));
    
    OnScreenResize(app);
}

void OnGlfwCloseWindow(GLFWwindow* window)
{
    App* app = (App*)glfwGetWindowUserPointer(window);
    app->isRunning = false;
}

void OnGlfwSetWindowPos(GLFWwindow* window, int xpos, int ypos)
{
    App* app = (App*)glfwGetWindowUserPointer(window);
    app->displayPos = vec2(glm::max(xpos, 0), glm::max(ypos, 0));
}

int main()
{
    App app         = {};
    app.deltaTime   = 1.0f/60.0f;
    app.displaySizeCurrent = ivec2(WINDOW_WIDTH, WINDOW_HEIGHT);
    app.displaySizePrevious = app.displaySizeCurrent;
    app.isRunning   = true;

    glfwSetErrorCallback(OnGlfwError);

    if (!glfwInit())
    {
        ELOG("glfwInit() failed\n");
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a centered window
    int count;
    int windowWidth, windowHeight;
    int monitorX, monitorY;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);
    windowWidth = videoMode->width / 1.5;
    windowHeight = windowWidth / 16 * 9;
    glfwGetMonitorPos(monitors[0], &monitorX, &monitorY);
    // Set the visibility window hint to false for subsequent window creation
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (!window)
    {
        ELOG("glfwCreateWindow() failed\n");
        return -1;
    }

    glfwDefaultWindowHints();

    app.displayPos.x = monitorX + (videoMode->width - windowWidth) / 2;
    app.displayPos.y = monitorY + (videoMode->height - windowHeight) / 2;

    glfwSetWindowPos(window, app.displayPos.x, app.displayPos.y);
    glfwShowWindow(window);

    glfwSetWindowUserPointer(window, &app);

    glfwSetMouseButtonCallback(window, OnGlfwMouseEvent);
    glfwSetCursorPosCallback(window, OnGlfwMouseMoveEvent);
    glfwSetScrollCallback(window, OnGlfwScrollEvent);
    glfwSetKeyCallback(window, OnGlfwKeyboardEvent);
    glfwSetCharCallback(window, OnGlfwCharEvent);
    glfwSetFramebufferSizeCallback(window, OnGlfwResizeFramebuffer);
    glfwSetWindowCloseCallback(window, OnGlfwCloseWindow);
    glfwSetWindowPosCallback(window, OnGlfwSetWindowPos);

    glfwMakeContextCurrent(window);

    // Load all OpenGL functions using the glfw loader function
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        ELOG("Failed to initialize OpenGL context\n");
        return -1;
    }

    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( OnGlError, &app );
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        ELOG("ImGui_ImplGlfw_InitForOpenGL() failed\n");
        return -1;
    }

    if (!ImGui_ImplOpenGL3_Init())
    {
        ELOG("Failed to initialize ImGui OpenGL wrapper\n");
        return -1;
    }

    f64 lastFrameTime = glfwGetTime();

    GlobalFrameArenaMemory = (u8*)malloc(GLOBAL_FRAME_ARENA_SIZE);

    RetrieveOpenGLContext();

    Init(&app);

    while (app.isRunning)
    {
        // Tell GLFW to call platform callbacks
        glfwPollEvents();

        // ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        
        Gui(&app);

        // Clear input state if required by ImGui
        if (ImGui::GetIO().WantCaptureKeyboard)
            for (u32 i = 0; i < KEY_COUNT; ++i)
                app.input.keys[i] = BUTTON_IDLE;

        if (ImGui::GetIO().WantCaptureMouse)
            for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
                app.input.mouseButtons[i] = BUTTON_IDLE;

        // Update
        Update(&app);

        // Transition input key/button states
        if (!ImGui::GetIO().WantCaptureKeyboard)
            for (u32 i = 0; i < KEY_COUNT; ++i)
                if      (app.input.keys[i] == BUTTON_PRESS)   app.input.keys[i] = BUTTON_PRESSED;
                else if (app.input.keys[i] == BUTTON_RELEASE) app.input.keys[i] = BUTTON_IDLE;

        if (!ImGui::GetIO().WantCaptureMouse)
            for (u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i)
                if      (app.input.mouseButtons[i] == BUTTON_PRESS)   app.input.mouseButtons[i] = BUTTON_PRESSED;
                else if (app.input.mouseButtons[i] == BUTTON_RELEASE) app.input.mouseButtons[i] = BUTTON_IDLE;

        app.input.mouseDelta = glm::vec2(0.0f, 0.0f);


        // Render
        Render(&app);
        ImGui::Render();

        // ImGui Render
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        ImGui::EndFrame();
        // Present image on screen
        glfwSwapBuffers(window);

        // Frame time
        const f64 currentFrameTime = glfwGetTime();
        app.deltaTime = (f32)(currentFrameTime - lastFrameTime);
        lastFrameTime = currentFrameTime;

        // Reset frame allocator
        GlobalFrameArenaHead = 0;
    }

    free(GlobalFrameArenaMemory);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

u32 Strlen(const char* string)
{
    u32 len = 0;
    while (*string++) len++;
    return len;
}

void* PushSize(u32 byteCount)
{
    ASSERT(GlobalFrameArenaHead + byteCount <= GLOBAL_FRAME_ARENA_SIZE,
           "Trying to allocate more temp memory than available");

    u8* curPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead += byteCount;
    return curPtr;
}

void* PushBytes(const void* bytes, u32 byteCount)
{
    ASSERT(GlobalFrameArenaHead + byteCount <= GLOBAL_FRAME_ARENA_SIZE,
            "Trying to allocate more temp memory than available");

    u8* srcPtr = (u8*)bytes;
    u8* curPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    u8* dstPtr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead += byteCount;
    while (byteCount--) *dstPtr++ = *srcPtr++;
    return curPtr;
}

u8* PushChar(u8 c)
{
    ASSERT(GlobalFrameArenaHead + 1 <= GLOBAL_FRAME_ARENA_SIZE,
            "Trying to allocate more temp memory than available");
    u8* ptr = GlobalFrameArenaMemory + GlobalFrameArenaHead;
    GlobalFrameArenaHead++;
    *ptr = c;
    return ptr;
}

String MakeString(const char *cstr)
{
    String str = {};
    str.len = Strlen(cstr);
    str.str = (char*)PushBytes(cstr, str.len);
              PushChar(0);
    return str;
}

String MakePath(String dir, String filename)
{
    String str = {};
    str.len = dir.len + filename.len + 1;
    str.str = (char*)PushBytes(dir.str, dir.len);
              PushChar('/');
              PushBytes(filename.str, filename.len);
              PushChar(0);
    return str;
}

String GetDirectoryPart(String path)
{
    String str = {};
    i32 len = (i32)path.len;
    while (len >= 0) {
        len--;
        if (path.str[len] == '/' || path.str[len] == '\\')
            break;
    }
    str.len = (u32)len;
    str.str = (char*)PushBytes(path.str, str.len);
              PushChar(0);
    return str;
}

String ReadTextFile(const char* filepath)
{
    String fileText = {};

    FILE* file = fopen(filepath, "rb");

    if (file)
    {
        fseek(file, 0, SEEK_END);
        fileText.len = ftell(file);
        fseek(file, 0, SEEK_SET);

        fileText.str = (char*)PushSize(fileText.len + 1);
        fread(fileText.str, sizeof(char), fileText.len, file);
        fileText.str[fileText.len] = '\0';

        fclose(file);
    }
    else
    {
        ELOG("fopen() failed reading file %s", filepath);
    }

    return fileText;
}

u64 GetFileLastWriteTimestamp(const char* filepath)
{
#ifdef _WIN32
    union Filetime2u64 {
        FILETIME filetime;
        u64      u64time;
    } conversor;

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesExA(filepath, GetFileExInfoStandard, &Data)) {
        conversor.filetime = Data.ftLastWriteTime;
        return(conversor.u64time);
    }
#else
    // NOTE: This has not been tested in unix-like systems
    struct stat attrib;
    if (stat(filepath, &attrib) == 0) {
        return attrib.st_mtime;
    }
#endif

    return 0;
}

void LogString(const char* str)
{
#ifdef _WIN32
    OutputDebugStringA(str);
    OutputDebugStringA("\n");
#else
    fprintf(stderr, "%s\n", str);
#endif
}


OpenGlContext RetrieveOpenGLContext()
{
    OpenGlContext ctx;
    ctx.version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
   // std::cout << "OpenGL version:" << '\n';
   // std::cout << ctx.version << '\n' << '\n';

    ctx.renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
   // std::cout << "OpenGL renderer:" << '\n';
   // std::cout <<  ctx.renderer << '\n' << '\n';

    ctx.vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
   // std::cout << "OpenGL vendor:" << '\n';
   // std::cout << ctx.vendor << '\n' << '\n';

    ctx.glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
  //  std::cout << "OpenGL GLSL version:" << '\n';
  //  std::cout << ctx.glslVersion << '\n' << '\n';

 /*   std::cout << "OpenGL extensions:" << '\n';
    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (int i = 0; i < num_extensions; i++)
    {
        const u8* str = glGetStringi(GL_EXTENSIONS, GLuint(i));
        std::cout << str << " ";
    }
    std::cout << '\n' << '\n' << '\n';
*/
    return ctx;
}
