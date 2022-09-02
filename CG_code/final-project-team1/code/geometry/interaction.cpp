#include <interaction.h>

/*virtual*/ Spectrum Interaction::Le(const Ray& incidentRayWorld) const
{
    return Spectrum();
}


// Methods
/*virtual*/ void SurfaceInteraction::ComputeScatteringFunctions(SurfaceInteraction* si) const
{
    primitive->ComputeScatteringFunctions(si);
}
/*override*/ Spectrum SurfaceInteraction::Le(const Ray& incidentRayWorld) const
{
    return primitive->Le(incidentRayWorld);
}




// Constructors
MediumInteraction::MediumInteraction() : 
    phase(nullptr), 
    emission(Float(0))
{ }