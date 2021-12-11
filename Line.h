#pragma once
#include "Object.h"
; class Line :
    public Object,
    public std::enable_shared_from_this<Line>
{
public:
    Line(Shader& sh);

    void DrawObject(glm::mat4 mvp) override;
    void CreateMenu() override {};

    void AddPoint(glm::vec3 point);
    void Update() override;
    void DeleteFirstPoint();
    bool IsValidPath();
    int size() { return points_.size(); };
    glm::vec3 GetFirstPoint();
    glm::vec3 GetSecondPoint();
    void ClearPoints();

    void Reverse();

    static unsigned int counter;
private:

    void update_object() override;

    std::vector<float> points_on_curve;
    std::vector<unsigned int> lines;

    std::vector<glm::vec3> points_;
};

