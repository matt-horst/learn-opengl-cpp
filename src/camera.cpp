#include "camera.hpp"
#include <glm/common.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_position(position), m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
      m_world_up(up), m_yaw(yaw), m_pitch(pitch),
      m_panning_speed(DEFAULT_PANNING_SPEED),
      m_rotational_speed(DEFAULT_ROTATIONAL_SPEED), m_zoom(DEFAULT_ZOOM) {
  update_vectors();
}

glm::mat4 Camera::get_view_matrix() {
  return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::pan(PanMovement direction, float delta_time) {
  const float delta = m_panning_speed * delta_time;
  switch (direction) {
  case PanMovement::FORWARD:
    m_position += m_front * delta;
    break;
  case PanMovement::BACK:
    m_position -= m_front * delta;
    break;
  case PanMovement::RIGHT:
    m_position += m_right * delta;
    break;
  case PanMovement::LEFT:
    m_position -= m_right * delta;
    break;
  }
}

void Camera::rotate(float dx, float dy) {
  m_yaw += dx * m_rotational_speed;
  m_pitch += dy * m_rotational_speed;

  m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

  update_vectors();
}

void Camera::zoom(float delta) {
  m_zoom = glm::clamp(m_zoom - delta, 1.0f, 45.0f);
}

void Camera::update_vectors() {
  m_front = glm::normalize(glm::vec3(
      static_cast<float>(cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))),
      static_cast<float>(sin(glm::radians(m_pitch))),
      static_cast<float>(sin(glm::radians(m_yaw)) *
                         cos(glm::radians(m_pitch)))));
  m_right = glm::normalize(glm::cross(m_front, m_world_up));
  m_up = glm::normalize(glm::cross(m_right, m_front));
}
