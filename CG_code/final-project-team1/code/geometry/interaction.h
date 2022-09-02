#ifndef INTERACTION_H
#define INTERACTION_H

#include <vector.h>
#include <primitive.h>
#include <spectrums.h>
#include <medium.h>

class PhaseFunction;

class Interaction
{
public:
    // Fields
    /**
     * @brief 交点
     */
    Point3f p;
    /**
     * @brief Geometric normal, 仅由交点形状决定，与扰动 (如微表面模型) 无关
     */
    Normal3f ng;
    /**
     * @brief Geometric tangent, 仅由交点形状决定，与扰动 (如微表面模型) 无关
     */
    Vector3f sg;
    Vector3f tg;
    /**
     * @brief 入射射线，//! 不一定是单位向量
     */
    Vector3f woWorld;
    bool byEmbree = false;


    // Constructors

    // Methods
    virtual Spectrum Le(const Ray& incidentRayWorld) const;
};

class BSDF;

class SurfaceInteraction : public Interaction
{
public:
    // Fields
    Point2f uv;
    Point3f rayONextPixelX, rayONextPixelY;
    Vector3f rayDNextPixelX, rayDNextPixelY;
    BSDF* bsdf = nullptr;
    const Primitive* primitive = nullptr;
    Surface surface;

    // Constructors

    // Methods
    /**
     * @brief 间接地调用 primitive->material->ComputeScatteringFunctions(si)
     */
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si) const; //! 这里不得不保留这个参数，因为 this 指针是 const 的
    /**
     * @brief 间接地调用 primitive->Le(incidentRayWorld);
     */
    virtual Spectrum Le(const Ray& incidentRayWorld) const;
};

class MediumInteraction : public Interaction
{
public:
    // Fields
    const PhaseFunction* phase;
    Spectrum emission;

    // Constructors
    MediumInteraction();
};

#endif