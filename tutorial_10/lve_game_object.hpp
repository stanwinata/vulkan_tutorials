
#pragma once

#include "lve_model.hpp"

#include <memory>

namespace lve {


struct Transform2DComponent {
    glm::vec2 translation;
    glm::vec2 scale{1.f, 1.f};
    float rotation = 0.0;
    glm::mat2 transform() {
        const float cos_theta = glm::cos(rotation);
        const float sin_theta = glm::sin(rotation);
        glm::mat2 rotation_matrix = glm::mat2{{cos_theta, -sin_theta},
                                              {sin_theta, cos_theta}};
        glm::mat2 scale_matrix = glm::mat2{{scale.x, 0.f},
                                           {0.0f, scale.y}};
        return rotation_matrix * scale_matrix;
    }
};

class LveGameObject {
    public:
        using id_t = unsigned int;
        static LveGameObject createGameObject() {
            static id_t current_id = 0;
            return LveGameObject {current_id++};
        }
        // Delete copy constructor and assignment operator
        LveGameObject(const LveGameObject &) = delete;
        LveGameObject &operator=(const LveGameObject &) = delete;
        // Enable move constructor and move assignment operator
        LveGameObject(LveGameObject &&) = default;
        LveGameObject &operator=(LveGameObject &&) = default;

        id_t getId() {return id_;}

        glm::vec3 color_{};
        std::shared_ptr<LveModel> lve_model_{};
        Transform2DComponent transform2d_{};
    private:
        LveGameObject(id_t objId) : id_(objId) {}
        id_t id_;
};

}