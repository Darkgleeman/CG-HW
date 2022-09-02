#include <randomSampler.h>

// Static Fields
/*static*/ thread_local std::uniform_real_distribution<Float> RandomSampler::distribution(0.0, 1.0);
/*static*/ thread_local std::mt19937 RandomSampler::generator(omp_get_thread_num());

// Methods
/*virtual*/ void RandomSampler::UseRealRandomSeed()
{
    generator = std::mt19937(std::random_device{}());
}
/*override*/ Float RandomSampler::Get1D()
{
    return distribution(generator);
}
/*override*/ Point2f RandomSampler::Get2D()
{
    return Point2f(Get1D(), Get1D());
}
/*override*/ Point3f RandomSampler::Get3D()
{
    return Point3f(Get1D(), Get1D(), Get1D());
}

/*override*/ bool RandomSampler::IsRandomSampler() const
{
    return true;
}