#ifndef CAMERA_H
#define CAMERA_H

#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class PanMovement { FORWARD, BACK, LEFT, RIGHT };

constexpr float DEFAULT_YAW = -90.0f;
constexpr float DEFAULT_PITCH = 0.0f;
constexpr float DEFAULT_PANNING_SPEED = 2.5f;
constexpr float DEFAULT_ROTATIONAL_SPEED = 0.1f;
constexpr float DEFAULT_ZOOM = 45.0f;

class Camera {
public:
  glm::vec3 m_position;
  glm::vec3 m_front;
  glm::vec3 m_up;
  glm::vec3 m_right;
  glm::vec3 m_world_up;

  float m_yaw;
  float m_pitch;

  float m_panning_speed;
  float m_rotational_speed;
  float m_zoom;

  Camera(glm::vec3 position = glm::vec3(),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = DEFAULT_YAW,
         float pitch = DEFAULT_PITCH);

  glm::mat4 get_view_matrix();

  void pan(PanMovement direction, float delta_time);
  void rotate(float dx, float dy);
  void zoom(float delta);

private:
  void update_vectors();
};

#endif
