#pragma once
#include <random>
#include <glm/vec3.hpp>
#include <glm/common.hpp>

class random_real_generator
{
public:
    random_real_generator(float a, float b)
    {
        gen = std::mt19937(rd());
        dis = std::uniform_real_distribution<float>(a, b);
    }

    float operator()()
    {
        return dis(gen);
    }

protected:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
};

class Wind
{
public:
    Wind(float var, float max);

    const glm::vec3 &Force() const;
    glm::vec3 &Force();

    void update();

protected:
    float var_;
    float max_;
    random_real_generator var_gen_;
    glm::vec3 force_;
};