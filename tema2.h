#pragma once

#include "components/simple_scene.h"
#include "lab_m1/tema2/tema2_lab_camera.h"
#include <chrono>

#define NO_OBSTACLES 80
#define NO_ENEMIES 30
#define AMMO 10

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
     public:
        Tema2();
        ~Tema2();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix) override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        // functions added
        void generate_maze(int height, int width);
        bool check_obstacle_collision(bool test_hero, float x_hero, float y_hero, float z_hero, float len_hero, float x, float y, float z, float len);
        bool check_dead(float x_enemy, float z_enemy, float len_enemy, float x_bullet, float z_bullet, float len_bullet);
        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color);
        void setupHUD();
     
    protected:
        implemented::Tema2Camera*camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;

        // default values
        float default_fov;
        // hero variables
        float x_start, z_start;
        float move_x, move_z;
        float health;
        float score;

        struct Enemy {
            float x, z, length;
            // coord to translate the enemy
            float follow_x, follow_z;
            bool isDead = false;
            float speed;
            float steps;
            // used for explosion animation
            std::chrono::time_point<std::chrono::steady_clock> explosion;
            glm::mat4 modelMatrix;
        };
        struct Enemy enemies[NO_ENEMIES];
        int last_id = 0; // used to populate the enemies array
        float offset_box; // offset for the enemy in order to not touch the hero at spawn
        std::chrono::time_point<std::chrono::steady_clock> end; // used for explosion animation

        struct Obstacle {
            float x, y, z, length;
        };
        struct Obstacle obstacles[NO_OBSTACLES];
        int obstacle_count;

        glm::vec3 offset_third_person;
        glm::vec3 offset_first_person;

        bool set_third = true;
        bool set_first = true;
        bool third_person = true;

        struct Bullet {
            float length;
            // coord to translate the bullet
            float follow_x, follow_z, follow_y;
            bool isShot;
            float max_distance;
            float angularStepCopy;
            glm::mat4 modelMatrix;
            // for shooting at the camera's direction
            glm::vec3 forward;
            glm::vec3 initial_position;
        };
        struct Bullet bullets[AMMO];
        int bullet_count; // ammo counter
        float angularStep;

        // used for healthbar and timebar
        float right, top;
        float angleY, angleX;

        // used for time of the game
        std::chrono::time_point<std::chrono::steady_clock> one_second;
        float timeTotal;
    };
}   // namespace m1
