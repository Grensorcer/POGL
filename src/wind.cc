#include "wind.hh"

Wind::Wind(float var, float max)
    : var_{ var }
    , max_{ max }
    , var_gen_{ -var, var }
{
    force_ = { var_gen_(), var_gen_(), var_gen_() };
}

const glm::vec3 &Wind::Force() const
{
    return force_;
}

glm::vec3 &Wind::Force()
{
    return force_;
}

void Wind::update()
{
    auto random_var = glm::vec3(var_gen_(), var_gen_(), var_gen_());
    force_ = glm::clamp(force_ + random_var, -max_, max_);
}