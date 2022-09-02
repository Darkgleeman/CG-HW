#include <texture.h>

// Constructors
BumpMap::BumpMap(const string& imagePath, BumpMapType type) : 
    SingleImageTextureSi<Normal3f>(imagePath, 3), 
    type(type)
{
    // 将 [0, 1] 的纹理映射到 [-1, 1] 的法向量
    for (int i = 0; i < texture2f->width + 1; ++i)
        for (int j = 0; j < texture2f->heigh + 1; ++j)
            texture2f->data[i][j] = Normal3f((texture2f->data[i][j] * 2 - Vector3f(1, 1, 1)).Normalized());
}

// Methods
/*override*/ Normal3f BumpMap::Evaluate(const SurfaceInteraction& evaluator) const
{
    Normal3f eval = SingleImageTextureSi<Normal3f>::Evaluate(evaluator);
    return Normal3f(eval.Normalized());
}

void BumpMap::Bump(SurfaceInteraction* si) const
{
    // Evaluate bump map
    Normal3f normal = Evaluate(*si);
    // Transform to world coordinate
    Vector3f sg(si->sg.Normalized());
    Vector3f tg(Vector3f::Cross(si->ng, sg));
    si->ng = Normal3f(
        sg.x * normal.x + tg.x * normal.y + si->ng.x * normal.z, 
        sg.y * normal.x + tg.y * normal.y + si->ng.y * normal.z, 
        sg.z * normal.x + tg.z * normal.y + si->ng.z * normal.z);
    si->ng.Normalize();
}