#include <medium.h>

//* Phase functions --------------------------------------------------------------------------------------------*//

// Constructors
/*virtual*/ PhaseFunction::~PhaseFunction() { }



// Constructors
HenyeyGreenstein::HenyeyGreenstein(Float g) : g(g) { }

// Supporting functions
static inline Float PhaseFunction_HG(Float cosTheta, Float g)
{
    Float denom = 1 + g * g + 2 * g * cosTheta;
    return Pix4Inv * (1 - g * g) / (denom * std::sqrt(denom));
}

// Methods
/*override*/ Float HenyeyGreenstein::P(const Vector3f& wo, const Vector3f& wi) const
{
    return PhaseFunction_HG(Vector3f::Dot(wo, wi), g);
}

/*override*/ Float HenyeyGreenstein::Sample_P(const Vector3f& wo, Vector3f* wi, const Point2f& samples) const
{
    // Compute cosθ and φ
    Float cosTheta;
    if (std::abs(g) < 1e-3)
        cosTheta = 1 - 2 * samples[0];
    else {
        Float sqrTerm = (1 - g * g) / (1 + g - 2 * g * samples[0]);
        cosTheta = -(1 + g * g - sqrTerm * sqrTerm) / (2 * g);
    }
    Float phi = 2 * Pi * samples[1];

    // Convert spherical coordinate to xyz coordinate
    Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
    Vector3f v1, v2;
    Vector3f::CoordinateSystem(wo, &v1, &v2);
    Vector3f sphereCoordinateOfHG(Vector3f::FromSpherical(sinTheta, cosTheta, phi));
    *wi = Vector3f(sphereCoordinateOfHG.x * v1 + sphereCoordinateOfHG.y * v2 + sphereCoordinateOfHG.z * wo);
    return PhaseFunction_HG(cosTheta, g);
}



//* Medium -----------------------------------------------------------------------------------------------------*//

/*virtual*/ Medium::~Medium() { }



//* MediumInterface --------------------------------------------------------------------------------------------*//

// Constructors
Surface::Surface() : 
    mediumIn(nullptr), 
    mediumOu(nullptr)
{ }

Surface::Surface(const Medium* medium) : 
    mediumIn(medium), 
    mediumOu(medium)
{ }

Surface::Surface(const Medium* mediumIn, const Medium* mediumOu) : 
    mediumIn(mediumIn), 
    mediumOu(mediumOu)
{ }

bool Surface::IsMediumTransition() const
{
    return mediumIn != mediumOu;
}