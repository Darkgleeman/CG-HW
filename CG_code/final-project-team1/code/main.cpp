#include <utils.h>
#include <geometrics.h>
#include <shapes.h>
#include <primitives.h>
#include <spectrums.h>
#include <bxdfs.h>
#include <materials.h>
#include <integrators.h>
#include <cameras.h>
#include <lights.h>
#include <samplers.h>
#include <aggregates.h>
#include <textures.h>
#include <distributions.h>
#include <mediums.h>

shared_ptr<Scene> GetScene_CornellBox();
shared_ptr<Scene> GetScene_CornellSphere();

int main()
{
    #ifndef NDEBUG
    Error("Booting under DEBUG mode");
    #else
    Error("Booting under RELEASE mode");
    #endif
    // Image settings
    Float aspectRatio;
    int imageWidth;
    int imageHeigh;
    int samplesPerPixel;
    int maxDepth;

    // Camera settings
    Point3f cameraPosition;
    Vector3f cameraForward;
    Float vfov;
    Float aperture;
    Vector3f vup;
    Float focusDistance;

    // Scene
    shared_ptr<Scene> scene;

    switch (2)
    {
        case 1:
            // Scene
            scene = GetScene_CornellBox();

            // Image settings
            aspectRatio = 1.0f;
            imageWidth = 600; // 2048
            imageHeigh = static_cast<int>(imageWidth / aspectRatio);
            samplesPerPixel = 32; // 1024
            maxDepth = 50;

            // Camera settings
            cameraPosition = Point3f(277.5, 277.5, -800);
            cameraForward = Vector3f(0, 0, 1);
            vfov = 40;
            aperture = 0;
            vup = Vector3f(0, 1, 0);
            focusDistance = 10;
            break;
        case 2:
        default:
            // Scene
            scene = GetScene_CornellSphere();

            // Image settings
            aspectRatio = 1.0f;
            imageWidth = 600; // 2048
            imageHeigh = static_cast<int>(imageWidth / aspectRatio);
            samplesPerPixel = 32; // 1024
            maxDepth = 50;

            // Camera settings
            cameraPosition = Point3f(277.5, 277.5, -800);
            cameraForward = Vector3f(0, 0, 1);
            vfov = 40;
            aperture = 0;
            vup = Vector3f(0, 1, 0);
            focusDistance = 10;
            break;
    }

    // Camera
    shared_ptr<const Camera> camera = make_shared<const Camera>(
        cameraPosition, cameraForward, vup, 
        vfov, aspectRatio, 
        aperture, focusDistance, 
        nullptr);

    // Sampler
    shared_ptr<Sampler> sampler = make_shared<RandomSampler>();

    // Integrator
    // DirectionalSamplerIntegrator integrator(camera, sampler, imageWidth, imageHeigh, samplesPerPixel, maxDepth);
    VolumePathSamplerIntegrator integrator(camera, sampler, imageWidth, imageHeigh, samplesPerPixel, maxDepth);

    integrator.Render(*scene);

    return 0;
}

class Pool
{
public:
    // Constructors
    ~Pool()
    {
        // Delete phase functions
        for (int i = 0; i < iPhases; ++i)
            delete phases[i];
        // Delete mediums
        for (int i = 0; i < iVolumes; ++i)
            delete volumes[i];
        // Delete mediums
        for (int i = 0; i < iMediums; ++i)
            delete mediums[i];
    }

    // Methods
    void PushBack(const Transform& transform, Transform** transInPool, Transform** transInvInPool)
    {
        if (iTranses == TransCapacity) { Error("Pool::PushBack(): Too many transforms"); exit(-1); }
        transes[iTranses] = transform;
        transInvs[iTranses] = transform.Inversed();
        *transInPool = &transes[iTranses];
        *transInvInPool = &transInvs[iTranses];
        ++iTranses;
    }
    void PushBack(const Spectrum& spectrum, Spectrum** spectrumInPool)
    {
        if (iSpectrums == SpectrumCapacity) { Error("Pool::PushBack(): Too many spectrums"); exit(-1); }
        spectrums[iSpectrums] = spectrum;
        *spectrumInPool = &spectrums[iSpectrums];
        ++iSpectrums;
    }
    void PushBack(const PhaseFunction* phase)
    {
        if (iPhases == PhaseCapacity) { Error("Pool::PushBack(): Too many phase functions"); exit(-1); }
        phases[iPhases] = phase;
        ++iPhases;
    }
    void PushBack(const Volume* volume)
    {
        if (iVolumes == VolumeCapacity) { Error("Pool::PushBack(): Too many volumes"); exit(-1); }
        volumes[iVolumes] = volume;
        ++iVolumes;
    }
    void PushBack(const Medium* medium)
    {
        if (iMediums == MediumCapacity) { Error("Pool::PushBack(): Too many mediums"); exit(-1); }
        mediums[iMediums] = medium;
        ++iMediums;
    }

private:
    // Transform pool fields
    static constexpr int TransCapacity = 512;
    Transform transes[TransCapacity];
    Transform transInvs[TransCapacity];
    int iTranses = 0;
    // Spectrum pool fields
    static constexpr int SpectrumCapacity = 512;
    Spectrum spectrums[SpectrumCapacity];
    int iSpectrums = 0;
    // PhaseFunction pool fields
    static constexpr int PhaseCapacity = 32;
    const PhaseFunction* phases[PhaseCapacity];
    int iPhases = 0;
    // Volume pool fields
    static constexpr int VolumeCapacity = 32;
    const Volume* volumes[VolumeCapacity];
    int iVolumes = 0;
    // Medium pool fields
    static constexpr int MediumCapacity = 32;
    const Medium* mediums[MediumCapacity];
    int iMediums = 0;
};

Pool pool;

const Spectrum Background_Sky(const Iso2f& phiTheta)
{
    Vector3f dir_unit(Vector3f::FromSpherical(phiTheta.x, std::sin(phiTheta.y), std::cos(phiTheta.y)));
    Float t = 0.5 * (dir_unit.y + 1.0);
    const Spectrum up(0.5, 0.7, 1.0);
    const Spectrum down(1, 1, 1);
    return (1 - t) * down + t * up;
}

const Spectrum Background_Sun(const Iso2f& phiTheta)
{
    Vector3f dir_unit(Vector3f::FromSpherical(phiTheta.x, std::sin(phiTheta.y), std::cos(phiTheta.y)));
    const Vector3f toSun(Vector3f(0.5, 0.4, 1).Normalized());
    const Spectrum sun(255./255., 222./255., 173./255.);
    const Spectrum sunScaled = sun * 0.1;
    return sunScaled * std::pow(std::max(Vector3f::Dot(dir_unit, toSun), Float(0)), 8);
}

shared_ptr<Scene> GetScene_CornellBox()
{
// Transforms
    Transform* trans_wallL, * transInv_wallL;
    Transform* trans_wallR, * transInv_wallR;
    Transform* trans_wallD, * transInv_wallD;
    Transform* trans_wallU, * transInv_wallU;
    Transform* trans_wallB, * transInv_wallB;
    Transform* trans_lightU, * transInv_lightU;
    Transform* trans_lightB, * transInv_lightB;
    Transform* trans_itemL, * transInv_itemL;
    Transform* trans_itemR, * transInv_itemR;
    Transform* trans_itemL_iso, * transInv_itemL_iso;
    Transform* trans_itemR_sky, * transInv_itemR_sky;
    pool.PushBack(Transform(Point3f(0, 277.5, 555 - 800 / 2), Quaternion(Vector3f(1, 0, 0), Radians(90)) * Quaternion(Vector3f(0, 0, 1), Radians(90)), Vector3f(800, 1, 555)), &trans_wallL, &transInv_wallL);
    pool.PushBack(Transform(Point3f(555, 277.5, 555 - 800 / 2), Quaternion(Vector3f(-1, 0, 0), Radians(90)) * Quaternion(Vector3f(0, 0, -1), Radians(90)), Vector3f(800, 1, 555)), &trans_wallR, &transInv_wallR);
    pool.PushBack(Transform(Point3f(277.5, 0, 555 - 800 / 2), Quaternion::Identity(), Vector3f(555, 1, 800)), &trans_wallD, &transInv_wallD);
    pool.PushBack(Transform(Point3f(277.5, 555, 555 - 800 / 2), Quaternion::Identity(), Vector3f(555, 1, 800)), &trans_wallU, &transInv_wallU);
    pool.PushBack(Transform(Point3f(277.5, 277.5, 555), Quaternion(Vector3f(1, 0, 0), Radians(90)), Vector3f(555, 1, 555)), &trans_wallB, &transInv_wallB);
    pool.PushBack(Transform(Point3f(277.5, 554, 177.5), Quaternion(Vector3f(1, 0, 0), Radians(180)), Vector3f(130 * 2, 1, 105 * 2)), &trans_lightU, &transInv_lightU);
    pool.PushBack(Transform(Point3f(555 - 115 - 15, 140 + 15, 554), Quaternion(Vector3f(1, 0, 0), Radians(-90)), Vector3f(230, 1, 280)), &trans_lightB, &transInv_lightB);
    pool.PushBack(Transform(Point3f(555 - (82.5 + 285), 165, 82.5 + 285), Quaternion(Vector3f(0, 1, 0), Radians(-15)), Vector3f(165, 330, 165)), &trans_itemL, &transInv_itemL);
    pool.PushBack(Transform(Point3f(555 - (82.5 + 100), 82.5, 82.5 + 90), Quaternion(Vector3f(0, 1, 0), Radians(18)), Vector3f(165, 165, 165)), &trans_itemR, &transInv_itemR);
    pool.PushBack(Transform(Point3f(555 - (82.5 + 285), 82.5, 82.5 + 285), Quaternion(Vector3f(0, 1, 0), Radians(-15)), Vector3f(165, 165, 165)), &trans_itemL_iso, &transInv_itemL_iso);
    pool.PushBack(Transform(Point3f(555 - (82.5 + 100), 82.5 * 1.4 * 2.1 + 35, 82.5 + 90), Quaternion(Vector3f(0, 1, 0), Radians(18)), Vector3f(165 * 1.4, 165 * 1.4 * 2, 165 * 1.4)), &trans_itemR_sky, &transInv_itemR_sky);

// Textures
    std::function<Spectrum(const Iso2f& uv)> texFunc_green = [](const Iso2f& uv) { const Spectrum c(0.12, 0.45, 0.15); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_red   = [](const Iso2f& uv) { const Spectrum c(0.65, 0.05, 0.05); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_white = [](const Iso2f& uv) { const Spectrum c(0.73, 0.73, 0.73); return c; };
    shared_ptr<TextureSi<Spectrum>> tex_green = make_shared<FunctionTextureSi<Spectrum>>(texFunc_green);
    shared_ptr<TextureSi<Spectrum>> tex_red   = make_shared<FunctionTextureSi<Spectrum>>(texFunc_red);
    shared_ptr<TextureSi<Spectrum>> tex_white = make_shared<FunctionTextureSi<Spectrum>>(texFunc_white);

// Materials
    shared_ptr<Material> mat_medium;
    shared_ptr<Material> mat_matte_green = make_shared<MatteMaterial>(tex_green, 0);
    shared_ptr<Material> mat_matte_red   = make_shared<MatteMaterial>(tex_red, 0);
    shared_ptr<Material> mat_matte_white = make_shared<MatteMaterial>(tex_white, 0);

// PhaseFunctions
    PhaseFunction* phase_hg = new HenyeyGreenstein(0.01);
    pool.PushBack(phase_hg);

// Volumes
    // VolumeFloatGrid* volume_smoke = new VolumeFloatGrid("assets/volumes/smoke.vdb", "density");
    VolumeFloatKDTree* volume_smoke = new VolumeFloatKDTree("assets/volumes/smoke.vdb", "density", 1, false, false);
    pool.PushBack(volume_smoke);

// Mediums
    Transform trans_mediumToLocal_smoke = volume_smoke->LocalToGrid().Inversed();
    Transform trans_mediumToWorld(trans_itemR_sky->m * trans_mediumToLocal_smoke.m);
    // Medium* medium_smoke = new DensityGridDeltaMedium(volume_smoke, Spectrum(0.085, 0.03, 0.0175), Spectrum(0.01, 0.475, 0.95), phase_hg, trans_mediumToWorld.Inversed());
    // Medium* medium_smoke = new DensityGridDecompositionMedium(volume_smoke, Spectrum(0.085, 0.03, 0.0175), Spectrum(0.01, 0.475, 0.95), phase_hg, trans_mediumToWorld.Inversed());
    // Medium* medium_smoke = new DensityKDTreeDeltaMedium(volume_smoke, Spectrum(0.085, 0.03, 0.0175), Spectrum(0.01, 0.475, 0.95), phase_hg, trans_mediumToWorld.Inversed());
    Medium* medium_smoke = new DensityKDTreeDecompositionMedium(volume_smoke, Spectrum(0.085, 0.03, 0.0175), Spectrum(0.01, 0.475, 0.95), phase_hg, trans_mediumToWorld.Inversed());
    pool.PushBack(medium_smoke);

// Surfaces
    Surface surface_air(nullptr, nullptr);
    Surface surface_airToSmoke(medium_smoke, nullptr);

// Shapes
    // Walls
    shared_ptr<Shape> shape_wallL = make_shared<Plane>(trans_wallL, transInv_wallL);
    shared_ptr<Shape> shape_wallR = make_shared<Plane>(trans_wallR, transInv_wallR);
    shared_ptr<Shape> shape_wallD = make_shared<Plane>(trans_wallD, transInv_wallD);
    shared_ptr<Shape> shape_wallU = make_shared<Plane>(trans_wallU, transInv_wallU);
    shared_ptr<Shape> shape_wallB = make_shared<Plane>(trans_wallB, transInv_wallB);
    // Items
    shared_ptr<Shape> shape_itemL = make_shared<Cube>(trans_itemL, transInv_itemL);
    shared_ptr<Shape> shape_itemR = make_shared<Cube>(trans_itemR_sky, transInv_itemR_sky);
    // Lights
    shared_ptr<Shape> shape_light0 = make_shared<Plane>(trans_lightU, transInv_lightU);
    shared_ptr<Shape> shape_light1 = make_shared<Plane>(trans_lightB, transInv_lightB);

// Primitive
    // Walls
    shared_ptr<Primitive> prim_wallL = make_shared<GeometricPrimitive>(shape_wallL, mat_matte_green, surface_air);
    shared_ptr<Primitive> prim_wallR = make_shared<GeometricPrimitive>(shape_wallR, mat_matte_red, surface_air);
    shared_ptr<Primitive> prim_wallD = make_shared<GeometricPrimitive>(shape_wallD, mat_matte_white, surface_air);
    shared_ptr<Primitive> prim_wallU = make_shared<GeometricPrimitive>(shape_wallU, mat_matte_white, surface_air);
    shared_ptr<Primitive> prim_wallB = make_shared<GeometricPrimitive>(shape_wallB, mat_matte_white, surface_air);
    // Items
    shared_ptr<Primitive> prim_itemL = make_shared<GeometricPrimitive>(shape_itemL, mat_matte_white, surface_air);
    shared_ptr<Primitive> prim_itemR = make_shared<GeometricPrimitive>(shape_itemR, mat_medium, surface_airToSmoke);
    // Lights
    shared_ptr<GeometricPrimitive> prim_light0 = make_shared<GeometricPrimitive>(shape_light0, mat_matte_white, surface_air);
    shared_ptr<GeometricPrimitive> prim_light1 = make_shared<GeometricPrimitive>(shape_light1, mat_matte_white, surface_air);

// Primitives
    vector<shared_ptr<Primitive>> primitives;
    primitives.reserve(50);
    // Walls
    primitives.push_back(prim_wallL);
    primitives.push_back(prim_wallR);
    primitives.push_back(prim_wallD);
    primitives.push_back(prim_wallU);
    primitives.push_back(prim_wallB);
    // Items
    primitives.push_back(prim_itemL);
    primitives.push_back(prim_itemR);
    // Lights
    primitives.push_back(prim_light0);
    primitives.push_back(prim_light1);

// Light
    vector<shared_ptr<Light>> lights;
    lights.reserve(10);
    shared_ptr<AreaLight> light_light0 = make_shared<DiffuseAreaLight>(Spectrum(1, 0.75, 0.5) * 8, shape_light0);
    shared_ptr<AreaLight> light_light1 = make_shared<DiffuseAreaLight>(Spectrum(1, 0.75, 0.5) * 2, shape_light1);
    prim_light0->areaLight = light_light0;
    prim_light1->areaLight = light_light1;

// Lights
    lights.push_back(light_light0);
    lights.push_back(light_light1);

// Aggregate
    #ifdef INTEL
    shared_ptr<EmbreeAccel> aggregate = make_shared<EmbreeAccel>(primitives);
    #else
    // shared_ptr<BVH> aggregate = make_shared<BVH>(primitives);
    shared_ptr<PrimitiveList> aggregate = make_shared<PrimitiveList>(primitives);
    #endif

// Scene
    return make_shared<Scene>(aggregate, lights);
}

shared_ptr<Scene> GetScene_CornellSphere()
{
// Transform
    Transform* trans_wallL, * transInv_wallL;
    Transform* trans_wallR, * transInv_wallR;
    Transform* trans_wallD, * transInv_wallD;
    Transform* trans_wallU, * transInv_wallU;
    Transform* trans_wallB, * transInv_wallB;
    Transform* trans_lightU, * transInv_lightU;
    Transform* trans_lightB, * transInv_lightB;
    Transform* trans_sphereL, * transInv_sphereL;
    Transform* trans_sphereR, * transInv_sphereR;
    Transform* trans_sphereL_sky, * transInv_sphereL_sky;
    Transform* trans_sphereR_sky, * transInv_sphereR_sky;
    Transform* trans_midVol, * transInv_midVol;
    Transform* trans_sphereM_vol, * transInv_sphereM_vol;
    Transform* trans_sphereL_vol, * transInv_sphereL_vol;
    Transform* trans_sphereR_vol, * transInv_sphereR_vol;
    pool.PushBack(Transform(Point3f(0, 277.5, 555 - 800 / 2), Quaternion(Vector3f(1, 0, 0), Radians(90)) * Quaternion(Vector3f(0, 0, 1), Radians(90)), Vector3f(800, 1, 555)), &trans_wallL, &transInv_wallL);
    pool.PushBack(Transform(Point3f(555, 277.5, 555 - 800 / 2), Quaternion(Vector3f(-1, 0, 0), Radians(90)) * Quaternion(Vector3f(0, 0, -1), Radians(90)), Vector3f(800, 1, 555)), &trans_wallR, &transInv_wallR);
    pool.PushBack(Transform(Point3f(277.5, 0, 555 - 800 / 2), Quaternion::Identity(), Vector3f(555, 1, 800)), &trans_wallD, &transInv_wallD);
    pool.PushBack(Transform(Point3f(277.5, 555, 555 - 800 / 2), Quaternion::Identity(), Vector3f(555, 1, 800)), &trans_wallU, &transInv_wallU);
    pool.PushBack(Transform(Point3f(277.5, 277.5, 555), Quaternion(Vector3f(1, 0, 0), Radians(90)), Vector3f(555, 1, 555)), &trans_wallB, &transInv_wallB);
    pool.PushBack(Transform(Point3f(277.5, 554, 177.5), Quaternion(Vector3f(1, 0, 0), Radians(180)), Vector3f(130 * 2, 1, 105 * 2)), &trans_lightU, &transInv_lightU);
    pool.PushBack(Transform(Point3f(277.5, 277.5, 554), Quaternion(Vector3f(-1, 0, 0), Radians(90)), Vector3f(500, 1, 400)), &trans_lightB, &transInv_lightB);
    pool.PushBack(Transform(Point3f(555 - (82.5 + 320), 82.5, 82.5 + 285), Quaternion(Vector3f(0, 1, 0), Radians(-15)), Vector3f(165, 165, 165)), &trans_sphereL, &transInv_sphereL);
    pool.PushBack(Transform(Point3f(555 - (82.5 + 65), 82.5, 82.5 + 90), Quaternion(Vector3f(0, 1, 0), Radians(18)), Vector3f(165, 165, 165)), &trans_sphereR, &transInv_sphereR);
    pool.PushBack(Transform(Point3f(555 - (82.5 + 320), 82.5 + 350, 82.5 + 285), Quaternion(Vector3f(0, 1, 0), Radians(-15)), Vector3f(165, 165, 165)), &trans_sphereL_sky, &transInv_sphereL_sky);
    pool.PushBack(Transform(Point3f(555 - (82.5 + 65), 82.5 + 100, 82.5 + 90), Quaternion(Vector3f(0, 1, 0), Radians(18)), Vector3f(165, 165, 165)), &trans_sphereR_sky, &transInv_sphereR_sky);
    pool.PushBack(Transform(Point3f(277.5, 277.5, 277.5), Quaternion(Vector3f(0, 1, 0), Radians(90)), Vector3f(301.5, 226.5, 504.75)), &trans_midVol, &transInv_midVol);
    pool.PushBack(Transform(Point3f(277.5, 277.5, 277.5), Quaternion::Identity(), Vector3f(150, 150, 150)), &trans_sphereM_vol, &transInv_sphereM_vol);
    pool.PushBack(Transform(Point3f(165.125 - 25, 277.5, 277.5), Quaternion::Identity(), Vector3f(150, 150, 150)), &trans_sphereL_vol, &transInv_sphereL_vol);
    pool.PushBack(Transform(Point3f(389.875 + 25, 277.5, 277.5), Quaternion::Identity(), Vector3f(150, 150, 150)), &trans_sphereR_vol, &transInv_sphereR_vol);

// Textures
    std::function<Spectrum(const Iso2f& uv)> texFunc_green = [](const Iso2f& uv) { const Spectrum c( 0.12,  0.45,  0.15); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_red   = [](const Iso2f& uv) { const Spectrum c( 0.65,  0.05,  0.05); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_white = [](const Iso2f& uv) { const Spectrum c( 0.73,  0.73,  0.73); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_zero  = [](const Iso2f& uv) { const Spectrum c(   0,      0,     0); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_1_4   = [](const Iso2f& uv) { const Spectrum c( 0.25,  0.25,  0.25); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_1_8   = [](const Iso2f& uv) { const Spectrum c(0.125, 0.125, 0.125); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_half  = [](const Iso2f& uv) { const Spectrum c(  0.5,   0.5,   0.5); return c; };
    std::function<Spectrum(const Iso2f& uv)> texFunc_one   = [](const Iso2f& uv) { const Spectrum c(    1,     1,     1); return c; };
    shared_ptr<TextureSi<Spectrum>> tex_green = make_shared<FunctionTextureSi<Spectrum>>(texFunc_green);
    shared_ptr<TextureSi<Spectrum>> tex_red   = make_shared<FunctionTextureSi<Spectrum>>(texFunc_red);
    shared_ptr<TextureSi<Spectrum>> tex_white = make_shared<FunctionTextureSi<Spectrum>>(texFunc_white);
    shared_ptr<TextureSi<Spectrum>> tex_zero  = make_shared<FunctionTextureSi<Spectrum>>(texFunc_zero);
    shared_ptr<TextureSi<Spectrum>> tex_one_4 = make_shared<FunctionTextureSi<Spectrum>>(texFunc_1_4);
    shared_ptr<TextureSi<Spectrum>> tex_one_8 = make_shared<FunctionTextureSi<Spectrum>>(texFunc_1_8);
    shared_ptr<TextureSi<Spectrum>> tex_half  = make_shared<FunctionTextureSi<Spectrum>>(texFunc_half);
    shared_ptr<TextureSi<Spectrum>> tex_one   = make_shared<FunctionTextureSi<Spectrum>>(texFunc_one);

// Materials
    shared_ptr<Material> mat_medium;
    shared_ptr<Material> mat_matte_green   = make_shared<MatteMaterial>(tex_green, 0);
    shared_ptr<Material> mat_matte_red     = make_shared<MatteMaterial>(tex_red, 0);
    shared_ptr<Material> mat_matte_white   = make_shared<MatteMaterial>(tex_white, 0);
    shared_ptr<Material> mat_pureMirror    = make_shared<MirrorMaterial>(tex_one, 1, 1.5);
    shared_ptr<Material> mat_pureGlass     = make_shared<GlassMaterial>(tex_one, tex_one, 1, 1.5);
    shared_ptr<Material> mat_plastic_red   = make_shared<PlasticMaterial>(tex_red, tex_one, Float(0.1), true, 1, 1.5);
    shared_ptr<Material> mat_plastic_green = make_shared<PlasticMaterial>(tex_green, tex_one, Float(0.1), true, 1, 1.5);

// PhaseFunctions
    PhaseFunction* phase_hg = new HenyeyGreenstein(0.01);
    pool.PushBack(phase_hg);

// Volumes
    // VolumeFloatGrid* volume_smoke = new VolumeFloatGrid("assets/volumes/smoke.vdb", "density");
    VolumeFloatKDTree* volume_smoke = new VolumeFloatKDTree("assets/volumes/cloud.vdb", "density", 1000, false, true);
    pool.PushBack(volume_smoke);

// Mediums
    Transform trans_mediumToLocal_smoke = volume_smoke->LocalToGrid().Inversed();
    Transform trans_mediumToWorld(trans_midVol->m * trans_mediumToLocal_smoke.m);
    // Medium* medium_smoke = new DensityGridDeltaMedium(volume_smoke, Spectrum(0.085, 0.03, 0.0175) * 65, Spectrum(0.01, 0.475, 0.95), phase_hg, trans_mediumToWorld.Inversed());
    // Medium* medium_smoke = new DensityGridDecompositionMedium(volume_smoke, Spectrum(0.085, 0.03, 0.0175) * 65, Spectrum(0.01, 0.475, 0.95), phase_hg, trans_mediumToWorld.Inversed());
    // Medium* medium_smoke = new DensityKDTreeDeltaMedium(volume_smoke, Spectrum(0.085, 0.03, 0.0175) * 65, Spectrum(0.01, 0.475, 0.95), phase_hg, trans_mediumToWorld.Inversed());
    Medium* medium_smoke = new DensityKDTreeDecompositionMedium(volume_smoke, Spectrum(0.085, 0.03, 0.0175) * 65, Spectrum(0.01, 0.475, 0.95), phase_hg, trans_mediumToWorld.Inversed());
    pool.PushBack(medium_smoke);

// Surfaces
    Surface surface_air(nullptr, nullptr);
    Surface surface_airToSmoke(medium_smoke, nullptr);
    Surface surface_smokeToAir(nullptr, medium_smoke);

// Shapes
    // Walls
    shared_ptr<Shape> shape_wallL = make_shared<Plane>(trans_wallL, transInv_wallL);
    shared_ptr<Shape> shape_wallR = make_shared<Plane>(trans_wallR, transInv_wallR);
    shared_ptr<Shape> shape_wallD = make_shared<Plane>(trans_wallD, transInv_wallD);
    shared_ptr<Shape> shape_wallU = make_shared<Plane>(trans_wallU, transInv_wallU);
    shared_ptr<Shape> shape_wallB = make_shared<Plane>(trans_wallB, transInv_wallB);
    // Items
    shared_ptr<Shape> shape_midVol = make_shared<Cube>(trans_midVol, transInv_midVol);
    shared_ptr<Shape> shape_itemM  = make_shared<Sphere>(trans_sphereM_vol, transInv_sphereM_vol);
    shared_ptr<Shape> shape_itemL  = make_shared<Sphere>(trans_sphereL_vol, transInv_sphereL_vol);
    shared_ptr<Shape> shape_itemR  = make_shared<Sphere>(trans_sphereR_vol, transInv_sphereR_vol);
    // Lights
    shared_ptr<Shape> shape_light0 = make_shared<Plane>(trans_lightU, transInv_lightU);
    shared_ptr<Shape> shape_light1 = make_shared<Plane>(trans_lightB, transInv_lightB);

// Primitive
    // Walls
    shared_ptr<Primitive> prim_wallL = make_shared<GeometricPrimitive>(shape_wallL, mat_matte_green, surface_air);
    shared_ptr<Primitive> prim_wallR = make_shared<GeometricPrimitive>(shape_wallR, mat_matte_red, surface_air);
    shared_ptr<Primitive> prim_wallD = make_shared<GeometricPrimitive>(shape_wallD, mat_matte_white, surface_air);
    shared_ptr<Primitive> prim_wallU = make_shared<GeometricPrimitive>(shape_wallU, mat_matte_white, surface_air);
    shared_ptr<Primitive> prim_wallB = make_shared<GeometricPrimitive>(shape_wallB, mat_matte_white, surface_air);
    // Items
    shared_ptr<Primitive> prim_midVol = make_shared<GeometricPrimitive>(shape_midVol, mat_medium, surface_airToSmoke);
    shared_ptr<Primitive> prim_itemM  = make_shared<GeometricPrimitive>(shape_itemM, mat_pureGlass, surface_smokeToAir);
    shared_ptr<Primitive> prim_itemL  = make_shared<GeometricPrimitive>(shape_itemL, mat_pureMirror, surface_smokeToAir);
    shared_ptr<Primitive> prim_itemR  = make_shared<GeometricPrimitive>(shape_itemR, mat_pureGlass, surface_smokeToAir);
    // shared_ptr<Primitive> prim_itemR = make_shared<GeometricPrimitive>(shape_itemR, mat_medium, surface_airToSmoke);
    // Lights
    shared_ptr<GeometricPrimitive> prim_light0 = make_shared<GeometricPrimitive>(shape_light0, mat_matte_white, surface_air);
    shared_ptr<GeometricPrimitive> prim_light1 = make_shared<GeometricPrimitive>(shape_light1, mat_matte_white, surface_air);

// Primitives
    vector<shared_ptr<Primitive>> primitives;
    primitives.reserve(50);
    // Walls
    primitives.push_back(prim_wallL);
    primitives.push_back(prim_wallR);
    primitives.push_back(prim_wallD);
    primitives.push_back(prim_wallU);
    primitives.push_back(prim_wallB);
    // Items
    primitives.push_back(prim_midVol);
    primitives.push_back(prim_itemR);
    primitives.push_back(prim_itemL);
    // primitives.push_back(prim_itemR);
    // Lights
    primitives.push_back(prim_light0);
    primitives.push_back(prim_light1);

// Light
    vector<shared_ptr<Light>> lights;
    lights.reserve(10);
    shared_ptr<AreaLight> light_light0 = make_shared<DiffuseAreaLight>(Spectrum(1, 0.75, 0.5) * 5, shape_light0);
    shared_ptr<AreaLight> light_light1 = make_shared<DiffuseAreaLight>(Spectrum(1, 0.75, 0.5) * 5, shape_light1);
    prim_light0->areaLight = light_light0;
    prim_light1->areaLight = light_light1;

// Lights
    lights.push_back(light_light0);

// Aggregate
    #ifdef INTEL
    shared_ptr<EmbreeAccel> aggregate = make_shared<EmbreeAccel>(primitives);
    #else
    // shared_ptr<BVH> aggregate = make_shared<BVH>(primitives);
    shared_ptr<PrimitiveList> aggregate = make_shared<PrimitiveList>(primitives);
    #endif

// Scene
    return make_shared<Scene>(aggregate, lights);
}
