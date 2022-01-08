#include "lab_m1/tema2/tema2.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;

static int** maze;
static int rows, cols;

Tema2::Tema2()
{
}

Tema2::~Tema2()
{
}

// check if bullets or hero collision kills enemies
bool Tema2::check_dead(float x_enemy, float z_enemy, float len_enemy, float x_bullet, float z_bullet, float len_bullet) {

    if (x_enemy < x_bullet + len_enemy && x_enemy + len_enemy > x_bullet && z_enemy < z_bullet + len_bullet && len_enemy + z_enemy > z_bullet) {
        return true;
    }
    else {
        return false;
    }
}

// check if bullets or hero collide with obstacles
bool Tema2::check_obstacle_collision(bool test_hero, float x_hero, float y_hero, float z_hero, float len_hero, float x, float y, float z, float len) {
    if (test_hero) {
        float offset_hero = 0.3f;
        float offset_hands = offset_hero * 5.0f;

        if ((x_hero <= x + len - offset_hero && x_hero + len_hero - offset_hero >= x) && (y_hero <= y + len && y_hero + len_hero >= y) && (z_hero <= z + len - offset_hero / offset_hands && z_hero + len_hero - offset_hero / offset_hands >= z)) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        z_hero -= 0.5f; // this is on obstacles coordinates, acts as an offset
        if ((x_hero <= x + len && x_hero + len_hero >= x) && (y_hero <= y + len && y_hero + len_hero >= y) && (z_hero <= z + len && z_hero + len_hero  >= z)) {
            return true;
        }
        else {
            return false;
        }
    }
}

void Tema2::generate_maze(int height, int width) {

    // Aldous - Broder algorithm
    int dx[4] = { 1, -1, 0, 0 };
    int dy[4] = { 0, 0, -1, 1 };

    int x = rand() % width;
    int y = rand() % height;

    int remaining_cells = width * height - 1;

    while (remaining_cells > 0) {
        for (int i = 0; i < 4; i++) {
            int direction = rand() % 4;
            int new_x = x + dx[direction];
            int new_y = y + dy[direction];

            if (new_x >= 0 && new_y >= 0 && new_x < width && new_y < height) {
                if (maze[new_y][new_x] == 0) {
                    maze[y][x] = 99;
                    maze[new_y][new_x] = 1;
                    remaining_cells -= 1;
                }
                x = new_x;
                y = new_y;
                break;
            }
        }
    }
}

void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color) {
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // object color
    GLint loc_object_color = glGetUniformLocation(shader->program, "object_color");
    glUniform3fv(loc_object_color, 1, glm::value_ptr(color));

    // Bind model matrix
    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Bind view matrix
    glm::mat4 viewMatrix = camera->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Tema2::setupHUD() {

    projectionMatrix = glm::ortho(0.1f, right, -0.1f, top, 0.01f, 200.0f);
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, camera->GetTargetPosition() + camera->forward);
    modelMatrix = glm::rotate(modelMatrix, -angleX, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, -angleY, glm::vec3(0, 1, 0));

    // healthbar
    glm::mat4 healthBar = modelMatrix;
    healthBar = glm::scale(healthBar, glm::vec3(0.2f, 0.2f, health * 5));
    RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], healthBar, glm::vec3(0, 1, 0));
 
    // timebar
    projectionMatrix = glm::ortho(-right, 0.1f, 0.1f, top, 0.01f, 200.0f);
    glm::mat4 timeBar = modelMatrix;
    timeBar = glm::scale(timeBar, glm::vec3(0.2f, 0.5f, timeTotal * 5));
    RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], timeBar, glm::vec3(1, 1, 1));

    // switch to normal
    projectionMatrix = glm::perspective(RADIANS(default_fov), window->props.aspectRatio, 0.01f, 200.0f);
}


void Tema2::Init()
{
    obstacle_count = 0;
    last_id = 0;
    health = 1.0f;
    score = 0;
    right = 5;  top = 5;
    one_second = std::chrono::steady_clock::now();
    timeTotal = 1.0f;

    rows = 12; cols = 12;
    maze = (int**) calloc(rows, sizeof(int*));
    for (int i = 0; i < rows; i++) {
        maze[i] = (int*) calloc(cols, sizeof(int));
    }

    generate_maze(rows, cols);

    // walls left and right to improve the maze
    for (int i = 0; i < rows; i++) {
        maze[i][0] = 1;
        maze[i][cols - 1] = 1;
    }

    int count = 0;
    // wall at start each 2 cells to improve the maze
    for (int i = 1; i < cols; i += 2) {
        count = 0;
        if (maze[0][i] == 99) {
            count++;
            if (count > 2) {
                maze[0][i] = 1;
            }
        }
        count = 0;
        if (maze[rows - 1][i] == 99) {
            count++;
            if (count > 2) {
                maze[rows - 1][i] = 1;
            }
        }
    }

    // make the final grid of maze
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (maze[i][j] == 99) {
                maze[i][j] = 0;
            }
            if (maze[i][j] == 1) {
                // coords of obstacles
                obstacles[obstacle_count].x = i;
                obstacles[obstacle_count].y = 0;
                obstacles[obstacle_count].z = j;
                obstacles[obstacle_count].length = 1;
                obstacle_count++;
            }
        }
    }

    // invisible wall behind the player so it cannot cheat
    for (int i = 0; i < cols; i++) {
        obstacles[obstacle_count].x = -1;
        obstacles[obstacle_count].y = 0;
        obstacles[obstacle_count].z = i;
        obstacles[obstacle_count].length = 1;
        obstacle_count++;
    }

    // place enemies
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (maze[i][j] == 0) {
                int place_enemy = rand() % 6;
                if (place_enemy == 1) {
                    maze[i][j] = 2;

                    // create an enemy
                    enemies[last_id].follow_x = 0;
                    enemies[last_id].follow_z = 0;
                    enemies[last_id].isDead = false;
                    enemies[last_id].length = 0.3f; // length of enemy object
                    enemies[last_id].x = i;
                    enemies[last_id].z = j;
                    enemies[last_id].speed = 0.3f;
                    enemies[last_id].steps = 0;
                    last_id++;
                }
            }
        }
    }

    // place the hero at maze entrance
    int found_start = 0;
    x_start = 0, z_start = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (maze[i][j] == 0 && maze[i + 1][j] != 1 && maze[i + 2][j] != 1) {
                x_start = i;
                z_start = j;
                found_start = 1;
                break;
            }
        }
        if (found_start == 1) {
            break;
        }
    }

    // bullets creation
    for (int i = 0; i < AMMO; i++) {
        bullets[i].follow_x = 0; bullets[i].follow_z = 0; bullets[i].follow_y = 0; bullets[i].length = 0.1f; bullets[i].isShot = false; bullets[i].max_distance = 0.8f;
    }
    bullet_count = 0;

    // misc variables added
    default_fov = 50;
    move_x = 0; move_z = 0;
    offset_box = 0.3f;

    camera = new implemented::Tema2Camera();
    offset_third_person = glm::vec3(-1.0f, 0.2f, 0);
    offset_first_person = glm::vec3(0.3f, 0.1f, 0);

    // set third person by default at start of game
    camera->Set(glm::vec3(x_start, 1, z_start) + offset_third_person, glm::vec3(x_start, 1, z_start), glm::vec3(0, 1, 0));

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("alien");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "characters/alien"), "Alien Animal.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Shader* shader = new Shader("Tema2Shader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    {
        Shader* shader = new Shader("Tema2ShaderDeformed");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexShaderDeformed.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragmentShaderDeformed.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    projectionMatrix = glm::perspective(RADIANS(default_fov), window->props.aspectRatio, 0.01f, 200.0f);
}


void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema2::Update(float deltaTimeSeconds)
{
    end = std::chrono::steady_clock::now();
    // exit game after time elapsed -> 30s
    if (std::chrono::duration_cast<std::chrono::seconds>(end - one_second).count() > 0.5f) {
        one_second = std::chrono::steady_clock::now();
        timeTotal -= 0.025f;
        if (timeTotal <= 0) {
            cout << "Time elapsed!\n" << "Score: " << score << "\nHealth: " << health << '\n';
            exit(1);
        }
    }
    setupHUD();

    if (health <= 0) {
        cout << "Game Over!\n" << "Final score: " << score << '\n';
        exit(1);
    }

    // put 3rd person camera
    if (set_third) {
        camera->Set(glm::vec3(x_start + move_x, 1, z_start + move_z) + offset_third_person, glm::vec3(x_start + move_x, 1, z_start + move_z), glm::vec3(0, 1, 0));
        set_third = false;
    }

    // put 1st person camera
    if (third_person == false) {
        if (set_first) {
            camera->Set(glm::vec3(x_start + move_x, 1, z_start + move_z) - offset_first_person, glm::vec3(x_start + move_x, 1, z_start + move_z), glm::vec3(0, 1, 0));
            set_first = false;
        }
    }

    // render ground
    {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.01f, 0.5f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f));
        RenderSimpleMesh(meshes["plane"], shaders["Tema2Shader"], modelMatrix, glm::vec3(0.4f, 0.4f, 0.4f)); // gray
    }

    // render maze walls
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (maze[i][j] == 1) {
                glm::mat4 modelMatrix = glm::mat4(1);
                // 1 -> so the maze will stay on top of ground
                modelMatrix = glm::translate(modelMatrix, glm::vec3(i, 1, j));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 2, 1));
                RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], modelMatrix, glm::vec3(0.4f, 0.27f, 0)); // yellow
            }
        }
    }

    // render enemies
    for (int i = 0; i < last_id; i++) {
        // if it s not dead then compute
        if (enemies[i].isDead == false) {
            enemies[i].modelMatrix = glm::mat4(1);
            // enemy movement
            enemies[i].modelMatrix *= glm::translate(enemies[i].modelMatrix, glm::vec3(enemies[i].follow_x, 0, enemies[i].follow_z)); 
            enemies[i].modelMatrix = glm::translate(enemies[i].modelMatrix, glm::vec3(enemies[i].x + offset_box, 0, enemies[i].z));
            enemies[i].modelMatrix = glm::scale(enemies[i].modelMatrix, glm::vec3(0.012f, 0.012f, 0.012f));
            // rotate so the enemy will look at the hero
            enemies[i].modelMatrix = glm::rotate(enemies[i].modelMatrix, RADIANS(-90), glm::vec3(0, 1, 0));

            RenderMesh(meshes["alien"], shaders["VertexColor"], enemies[i].modelMatrix);
        }
        // if it s dead, then show the death animation -> explosion
        else {
            if (std::chrono::duration_cast<std::chrono::seconds>(end - enemies[i].explosion).count() < 0.8f) {
                RenderMesh(meshes["alien"], shaders["Tema2ShaderDeformed"], enemies[i].modelMatrix);
            }
        }
    }
    int distance = 30;
    for (int i = 0; i < last_id; i++) {

        /* compute follow_x and follow_z
        
        the enemy is in the middle of the cell
        1. go front
        2. go in the middle again
        3. go back
        4. go in the middle again
        5. repeat for left-right movement
        */

        // front
        if (enemies[i].steps < distance && enemies[i].steps >= 0) {
            enemies[i].follow_x -= enemies[i].speed * deltaTimeSeconds;
            enemies[i].steps++;
        }
        // back
        else if (enemies[i].steps < distance * 3 && enemies[i].steps >= distance) {
            enemies[i].follow_x += enemies[i].speed * deltaTimeSeconds;
            enemies[i].steps++;
        }
        else if (enemies[i].steps < distance * 4 && enemies[i].steps >= distance * 3) {
            enemies[i].follow_x -= enemies[i].speed * deltaTimeSeconds;
            enemies[i].steps++;
        }
        // left
        else if (enemies[i].steps < distance * 5 && enemies[i].steps >= distance * 4) {
            enemies[i].follow_z -= enemies[i].speed * deltaTimeSeconds;
            enemies[i].steps++;
        }
        // right
        else if (enemies[i].steps < distance * 7 && enemies[i].steps >= distance * 5) {
            enemies[i].follow_z += enemies[i].speed * deltaTimeSeconds;
            enemies[i].steps++;
        }
        else if (enemies[i].steps < distance * 8 && enemies[i].steps >= distance * 7) {
            enemies[i].follow_z -= enemies[i].speed * deltaTimeSeconds;
            enemies[i].steps++;
        }
        // reset
        else if (enemies[i].steps == distance * 8) {
            enemies[i].steps = 0;
        }
    }

    for (int i = 0; i < last_id; i++) {
        // test bullet collision
        for (int j = 0; j < AMMO; j++) {
            if (bullets[j].isShot == false) {
                continue;
            }
            if (enemies[i].isDead == true) {
                continue;
            }
            enemies[i].isDead = check_obstacle_collision(false, enemies[i].x + enemies[i].follow_x + offset_box, 0.5f, enemies[i].z + enemies[i].follow_z, enemies[i].length + 0.2f, bullets[j].forward.x * bullets[j].follow_x + x_start + move_x + 0.5f, bullets[j].forward.y * bullets[j].follow_y + 0.6f, bullets[j].forward.z * bullets[j].follow_z + z_start + move_z, bullets[j].length);
            if (enemies[i].isDead) {
                bullets[j].isShot = false;
                bullets[j].max_distance = 0;
                enemies[i].explosion = std::chrono::steady_clock::now();
                score += 10;
                cout << "Score: " << score << "\n";
                break;
            }
        }
        // test enemy - hero collision
        // hero kills the enemy but gets 0.5 damage
        if (enemies[i].isDead == false) {
            enemies[i].isDead = check_dead(enemies[i].x + enemies[i].follow_x + offset_box, enemies[i].z + enemies[i].follow_z, enemies[i].length, x_start + move_x, z_start + move_z, 1);
            if (enemies[i].isDead) {
                health -= 0.5f;
                score -= 20;
                cout << "Score: " << score << "\n";
                enemies[i].explosion = std::chrono::steady_clock::now();
                cout << "Health: " << health << '\n';
                break;
            }
        }
    }

    for (int i = 0; i < NO_OBSTACLES; i++) {
        // test bullet collision
        for (int j = 0; j < AMMO; j++) {
            if (bullets[j].isShot == false) {
                continue;
            }
            bool hitObstacle = check_obstacle_collision(false, obstacles[i].x, obstacles[i].y, obstacles[i].z, obstacles[i].length, bullets[j].forward.x * bullets[j].follow_x + x_start + move_x + 0.5f, bullets[j].forward.y * bullets[j].follow_y + 0.6f, bullets[j].forward.z * bullets[j].follow_z + z_start + move_z + 0.1f, bullets[j].length);
            if (hitObstacle) {
                bullets[j].isShot = false;
                bullets[j].max_distance = 0;
            }
        }
    }

   // render bullets
    for (int i = 0; i < AMMO; i++) {

        bullets[i].modelMatrix = glm::mat4(1);
        // movement
        bullets[i].modelMatrix *= glm::translate(bullets[i].modelMatrix, bullets[i].forward * glm::vec3(bullets[i].follow_x, bullets[i].follow_y, bullets[i].follow_z));
        bullets[i].modelMatrix = glm::translate(bullets[i].modelMatrix, glm::vec3(x_start + move_x + 0.5f, 0.6f, z_start + move_z + 0.1f));
        bullets[i].modelMatrix = glm::scale(bullets[i].modelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
        bullets[i].modelMatrix = glm::translate(bullets[i].modelMatrix, glm::vec3(0, 0, 1));

        // if bullet is shot
        if (bullets[i].isShot && bullets[i].max_distance > 0) {
            bullets[i].follow_x += 0.4f;
            bullets[i].follow_y += 0.4f;
            bullets[i].follow_z += 0.4f;
            bullets[i].max_distance -= 0.05f;
        }
        // render the weapon only in first person
        if (third_person == false) {
            RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], bullets[i].modelMatrix, glm::vec3(0.4f, 1, 0.7f)); // turquoise
        }

        // reset bullets after distance expires
        if (bullets[i].max_distance <= 0) {
            bullets[i].isShot = false;
            bullets[i].follow_x = 0;
            bullets[i].follow_y = 0;
            bullets[i].follow_z = 0;
            bullets[i].max_distance = 0.8f;
        }
    }

    // render hero at x_start, z_start
    // legs
    // left
    glm::mat4 modelMatrix = glm::mat4(1);
    // movement
    float scale_factor = 0.2f;
    modelMatrix *= glm::translate(modelMatrix, glm::vec3(move_x, 0, move_z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_start, scale_factor, z_start));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale_factor, scale_factor * 2, scale_factor));
    // 0.55 = 0.4 + 0.15 (offset for space between the legs)
    float legs_space = 0.15f;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -(scale_factor * 2 + legs_space)));
    if (third_person == true) {
        RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], modelMatrix, glm::vec3(0.15f, 0, 0.9f)); // purple
    }

    // right
    modelMatrix = glm::mat4(1);
    // movement
    modelMatrix *= glm::translate(modelMatrix, glm::vec3(move_x, 0, move_z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_start, scale_factor, z_start));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale_factor, scale_factor * 2, scale_factor));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, scale_factor * 2 + legs_space));
    if (third_person == true) {
        RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], modelMatrix, glm::vec3(0.15f, 0, 0.9f)); // purple
    }

    // body
    modelMatrix = glm::mat4(1);
    // movement
    modelMatrix *= glm::translate(modelMatrix, glm::vec3(move_x, 0, move_z));
    scale_factor = 0.4f;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_start, scale_factor + scale_factor/2, z_start));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale_factor, scale_factor, scale_factor));
    if (third_person == true) {
        RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], modelMatrix, glm::vec3(0.55f, 0.1f, 1)); // violet
    }
    // head
    modelMatrix = glm::mat4(1);
    // movement
    modelMatrix *= glm::translate(modelMatrix, glm::vec3(move_x, 0, move_z));
    scale_factor = 0.2f;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_start, scale_factor * 5 - scale_factor/2, z_start));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale_factor, scale_factor, scale_factor));
    if (third_person == true) {
        RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], modelMatrix, glm::vec3(1, 0.92f, 0.5f)); // skin
    }

    // hands
    // left
    modelMatrix = glm::mat4(1);
    // movement
    modelMatrix *= glm::translate(modelMatrix, glm::vec3(move_x, 0, move_z));
    scale_factor = 0.2f;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_start, scale_factor * 3 - 0.025f, z_start));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale_factor, scale_factor * 2, scale_factor));
    float hands_space = 1;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -hands_space));
    if (third_person == true) {
        RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], modelMatrix, glm::vec3(1, 0.92f, 0.5f)); // skin
    }

    // right
    modelMatrix = glm::mat4(1);
    // movement
    modelMatrix *= glm::translate(modelMatrix, glm::vec3(move_x, 0, move_z));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x_start, scale_factor * 3 - 0.025f, z_start));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale_factor, scale_factor * 2, scale_factor));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, hands_space));
    if (third_person == true) {
        RenderSimpleMesh(meshes["box"], shaders["Tema2Shader"], modelMatrix, glm::vec3(1, 0.92f, 0.5f)); // skin
    }
}


void Tema2::FrameEnd()
{
    DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void Tema2::RenderMesh(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    float cameraSpeed = 1.5f;

    // for collisions
    float test_pos = 0;
    bool collision_obstacle = false;

    // move hero
    if (window->KeyHold(GLFW_KEY_W)) {

        test_pos = 1.5 * deltaTime;

        for (int i = 0; i < NO_OBSTACLES; i++) {
            if (check_obstacle_collision(true, x_start + move_x + test_pos, 0, z_start + move_z, 1, obstacles[i].x, obstacles[i].y, obstacles[i].z, obstacles[i].length)) {
                collision_obstacle = true;
            }
        }
        if (collision_obstacle == false) {
            move_x += 1.5 * deltaTime;
            camera->MoveForward(deltaTime * cameraSpeed);
        }
    }

    if (window->KeyHold(GLFW_KEY_A)) {

        test_pos = -1.5 * deltaTime;

        for (int i = 0; i < NO_OBSTACLES; i++) {
            if (check_obstacle_collision(true, x_start + move_x, 0, z_start + move_z + test_pos, 1, obstacles[i].x, obstacles[i].y, obstacles[i].z, obstacles[i].length)) {
                collision_obstacle = true;
            }
        }
        if (collision_obstacle == false) {
            move_z -= 1.5 * deltaTime;
            camera->TranslateRight(-deltaTime * cameraSpeed);
        }
    }

    if (window->KeyHold(GLFW_KEY_S)) {

        test_pos = -1.5 * deltaTime;

        for (int i = 0; i < NO_OBSTACLES; i++) {
            if (check_obstacle_collision(true, x_start + move_x + test_pos, 0, z_start + move_z, 1, obstacles[i].x, obstacles[i].y, obstacles[i].z, obstacles[i].length)) {
                collision_obstacle = true;
            }
        }
        if (collision_obstacle == false) {
            move_x -= 1.5 * deltaTime;
            camera->MoveForward(-deltaTime * cameraSpeed);
        }
    }

    if (window->KeyHold(GLFW_KEY_D)) {
        
        test_pos = 1.5 * deltaTime;

        for (int i = 0; i < NO_OBSTACLES; i++) {
            if (check_obstacle_collision(true, x_start + move_x, 0, z_start + move_z + test_pos, 1, obstacles[i].x, obstacles[i].y, obstacles[i].z, obstacles[i].length)) {
                collision_obstacle = true;
            }
        }
        if (collision_obstacle == false) {
            move_z += 1.5 * deltaTime;
            camera->TranslateRight(deltaTime * cameraSpeed);
        }
    }
    // move the camera only if MOUSE_RIGHT button is pressed
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        if (window->KeyHold(GLFW_KEY_Q)) {
            camera->TranslateUpward(deltaTime * cameraSpeed);
        }

        if (window->KeyHold(GLFW_KEY_E)) {
            camera->TranslateUpward(-deltaTime * cameraSpeed);
        }
    } 
}


void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
    // switch the cameras
    if (key == GLFW_KEY_LEFT_CONTROL) {
        if (third_person == true) {
            third_person = false;
            set_first = true;
        }
        else {
            third_person = true;
            set_third = true;
        }
    }
    // reset the third_person camera
    if (key == GLFW_KEY_R) {
        third_person = true;
        camera->Set(glm::vec3(x_start + move_x, 1, z_start + move_z) + offset_third_person, glm::vec3(x_start + move_x, 1, z_start + move_z), glm::vec3(0, 1, 0));
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event

    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float sensivityOX = 0.001f;
        float sensivityOY = 0.001f;

        angleY = -2 * sensivityOY * deltaY;
        angleX = -2 * sensivityOX * deltaX;

        if (third_person == false) {
            camera->RotateFirstPerson_OX(-2 * sensivityOX * deltaY);
            camera->RotateFirstPerson_OY(-2 * sensivityOY * deltaX);
        }

        if (third_person == true) {
            camera->RotateThirdPerson_OX(-2 * sensivityOX * deltaY);
            camera->RotateThirdPerson_OY(-2 * sensivityOY * deltaX);
        }
    }
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
    // fire bullets
    if (button == GLFW_MOUSE_BUTTON_RIGHT && third_person == false) {

        bullets[bullet_count].forward = camera->forward;
        bullets[bullet_count].initial_position = camera->position;
        bullets[bullet_count].isShot = true;
        bullet_count++;

        if (bullet_count == AMMO) {
            bullet_count = 0;
        }
    }
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}
