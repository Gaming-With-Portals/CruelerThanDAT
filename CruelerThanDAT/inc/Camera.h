#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

class CameraManager {
public:
    // Access the one and only instance
    static CameraManager& Instance();

    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;
    CameraManager(CameraManager&&) = delete;
    CameraManager& operator=(CameraManager&&) = delete;

    glm::mat4 getViewMatrix();

    void Zoom(float amount);

    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 eye = glm::vec3(0, 0, 5);
    glm::vec3 center = glm::vec3(0, 1, 0); 
    glm::vec3 up = glm::vec3(0, 1, 0);

    glm::vec3 v0, v1;
    bool dragging = false;
    int win_w = 1600, win_h = 900;
    int win_x = 0, win_y = 0;

    void SetWindowSize(int res_x, int res_y);

    void SetWindowPos(int pos_x, int pos_y);

    void Input(SDL_Event* e);

    glm::mat4 GetMatrixV();

    glm::mat4 GetMatrixP();
private:

    glm::vec3 arcballMap(float x, float y, float w, float h);

    CameraManager();

    ~CameraManager();
};