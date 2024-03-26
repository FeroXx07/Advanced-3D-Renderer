#ifndef CAMERA_H
#define CAMERA_H
/*
 * Camera class retrieved from LearnOpenGL.
 * https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h
 */
#include "platform.h"

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
constexpr float YAW         = -90.0f;
constexpr float PITCH       =  0.0f;
constexpr float SPEED       =  2.5f;
constexpr float SENSITIVITY =  0.1f;
constexpr float ZOOM        =  45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 position = glm::zero<vec3>();
    glm::vec3 front = glm::zero<vec3>();
    glm::vec3 up = glm::zero<vec3>();
    glm::vec3 right = glm::zero<vec3>();
    glm::vec3 worldUp = glm::zero<vec3>();
    // euler Angles, x(yaw), y(pitch), z(roll)
    glm::vec3 angles = glm::zero<vec3>();
    
    // camera options
    float movementSpeed;
    float mouseSensitivity;
    float zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
    {
        this->position = position;
        this->worldUp = up;
        this->angles.x = yaw;
        this->angles.y = pitch;
        UpdateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
    {
        this->position = glm::vec3(posX, posY, posZ);
        this->worldUp = glm::vec3(upX, upY, upZ);
        this->angles.x = yaw;
        this->angles.y = pitch;
        UpdateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(position, position + front, up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(const Camera_Movement direction, const float deltaTime)
    {
        const float velocity = movementSpeed * deltaTime;
        if (direction == Camera_Movement::FORWARD)
            position += front * velocity;
        if (direction == Camera_Movement::BACKWARD)
            position -= front * velocity;
        if (direction == Camera_Movement::LEFT)
            position -= right * velocity;
        if (direction == Camera_Movement::RIGHT)
            position += right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xOffset, float yOffset, const GLboolean constrainPitch = true)
    {
        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        angles.x += xOffset;
        angles.y += yOffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (angles.y > 89.0f)
                angles.y = 89.0f;
            if (angles.y < -89.0f)
                angles.y = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        UpdateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yOffset)
    {
        zoom -= (float)yOffset;
        if (zoom < 1.0f)
            zoom = 1.0f;
        if (zoom > 45.0f)
            zoom = 45.0f;
    }

    // calculates the front vector from the Camera's (updated) Euler Angles
    void UpdateCameraVectors()
    {
        // Calculate front vector
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(angles.x)) * cos(glm::radians(angles.y));
        newFront.y = sin(glm::radians(angles.y));
        newFront.z = sin(glm::radians(angles.x)) * cos(glm::radians(angles.y));
        front = glm::normalize(newFront);

        // Calculate right and up vectors
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));

        // Apply roll to the right vector
        glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angles.z), front);
        right = glm::vec3(rollMatrix * glm::vec4(right, 0.0f));

        // Recalculate up vector after roll
        up = glm::normalize(glm::cross(right, front));
    }
};
#endif // CAMERA_H
