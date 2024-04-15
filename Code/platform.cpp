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
    SetupImGuiStyle();
    //ImGui::StyleColorsDark();
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
void SetupImGuiStyle()
{
    // Photoshop style by Derydoca from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();
	
	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6000000238418579f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 4.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 4.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 2.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.FrameRounding = 2.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 13.0f;
	style.ScrollbarRounding = 12.0f;
	style.GrabMinSize = 7.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 1.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
	
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1764705926179886f, 0.1764705926179886f, 0.1764705926179886f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.2627451121807098f, 0.2627451121807098f, 0.2627451121807098f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2745098173618317f, 0.2745098173618317f, 0.2745098173618317f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3910000026226044f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.2627451121807098f, 0.2627451121807098f, 0.2627451121807098f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6700000166893005f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.5843137502670288f, 0.5843137502670288f, 0.5843137502670288f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
}
