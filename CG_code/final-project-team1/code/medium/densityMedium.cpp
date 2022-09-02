#include <densityMedium.h>

//* DensityMedium ----------------------------------------------------------------------------------------------*//

// Constructors
DensityGridMedium::DensityGridMedium(
        const VolumeFloatGrid* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium) : 
    density(density), 
    toMut(toMut), 
    albedo(albedo), 
    phase(phase), 
    worldToMedium(worldToMedium)
{ }

// Constructors
DensityKDTreeMedium::DensityKDTreeMedium(
        const VolumeFloatKDTree* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium) : 
    density(density), 
    toMut(toMut), 
    albedo(albedo), 
    phase(phase), 
    worldToMedium(worldToMedium)
{ }



//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////*//
//* DensityGridMediums /////////////////////////////////////////////////////////////////////////////////////////*//
//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////*//

//* DensityGridDeltaMedium -------------------------------------------------------------------------------------*//

DensityGridDeltaMedium::DensityGridDeltaMedium(
        const VolumeFloatGrid* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium) : 
    DensityGridMedium(density, toMut, albedo, phase, worldToMedium)
{
    mubar = density->GridMaxValue() * toMut.Maximum();
    mubar_inv = Float(1) / mubar;
}

/*override*/ DensityGridDeltaMedium::~DensityGridDeltaMedium() { }

// Methods
/*override*/ Spectrum DensityGridDeltaMedium::Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const
{
    // Transform ray from world coordinate to medium coordinate
    Ray rayMedium = worldToMedium(Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()), rayWorld.tMax * rayWorld.d.Norm()));
    if (std::isnan(rayMedium.tMax))
        return Spectrum(Float(1));

    // Ray casting and compute tMin, tMax
    const AABound3f bound = density->GridBoundf();
    Float tMin, tMax;
    if (!bound.Intersect(rayMedium, &tMin, &tMax) || std::isnan(tMax))
        return Spectrum(Float(1));

    // Delta tracking
    Float t = tMin;
    while (true)
    {
        // Step forward
        t -= std::log(1 - sampler.Get1D()) * mubar_inv;
        if (t >= tMax || std::isnan(t))
            break;

        // Compute sampling variables
        Float dense = density->Sample(rayMedium(t));
        Float kexi = sampler.Get1D();
        Spectrum mut = dense * toMut;
        Spectrum mus = albedo * mut;
        Spectrum mua = mut - mus;
        Spectrum mun = Spectrum(mubar) - mut;
        Float weighedAbsAvga = (mua * *weigh).Absed().Average();
        Float weighedAbsAvgs = (mus * *weigh).Absed().Average();
        Float weighedAbsAvgn = (mun * *weigh).Absed().Average();
        Float c_inv = Float(1) / (weighedAbsAvga + weighedAbsAvgs + weighedAbsAvgn);
        Float pa;
        Float pa_plus_ps;
        Float pn;

        // Delta tracking
        if (kexi < (pa = weighedAbsAvga * c_inv))
        {
            // Set mi
            mi->emission = *weigh * mua * (mubar_inv / pa) * Spectrum(); // TODO: Implement emission, maybe using another grid?
            // Terminate path
            return Spectrum();
        }
        else if (kexi < (pa_plus_ps = Float(1) - (pn = weighedAbsAvgn * c_inv)))
        {
            // Update weigh
            Float ps = pa_plus_ps - pa;
            *weigh *= mus * (mubar_inv / ps);
            // Set mi
            mi->p = Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()))(t);
            mi->woWorld = Vector3f(-rayWorld.d);
            mi->phase = phase;
            // Return albedo
            return albedo;
        }
        else
        {
            // Update weigh
            *weigh *= mun * (mubar_inv / pn);
        }
    }
    return Spectrum(Float(1));
}

/*override*/ Spectrum DensityGridDeltaMedium::Tr(const Ray& rayWorld, Sampler& sampler) const
{
    // Transform ray from world coordinate to medium coordinate
    Ray rayMedium = worldToMedium(Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()), rayWorld.tMax * rayWorld.d.Norm()));

    // Ray casting and compute tMin, tMax
    const AABound3f bound = density->GridBoundf();
    Float tMin, tMax;
    if (!bound.Intersect(rayMedium, &tMin, &tMax) || std::isnan(tMax))
        return Spectrum(Float(1));

    // Ratio tracking
    Spectrum tr(Float(1));
    Float t = tMin;
    while (true)
    {
        t -= std::log(1 - sampler.Get1D()) * mubar_inv;
        if (t >= tMax || std::isnan(t))
            break;
        Float dense = density->Sample(rayMedium(t));
        tr *= Spectrum(Float(1)) - Spectrum::Max(Spectrum(), dense * mubar_inv * toMut);
        // TODO: Russian roulette
    }
    return tr;
}



//* DensityGridDecompositionMedium -----------------------------------------------------------------------------*//

// Constructors
DensityGridDecompositionMedium::DensityGridDecompositionMedium(
        const VolumeFloatGrid* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium) : 
    DensityGridMedium(density, toMut, albedo, phase, worldToMedium)
{
    // Compute mubar
    mubar = density->GridMaxValue() * toMut.Maximum();
    mubar_inv = Float(1) / mubar;
    // Compute mu
    mutC = density->GridMinValue() * toMut;
    musC = albedo * mutC;
    muaC = mutC - musC;
    // Compute p
    paC = (muaC * mubar_inv).Average();
    psC = (musC * mubar_inv).Average();
    pResidual = Float(1) - paC - psC;
    // Compute with homogeneous control component
    mutC_homoC = mutC.Minimum();
    mubarR_homoC_inv = Float(1) / (mubar - mutC_homoC);
}

/*override*/ DensityGridDecompositionMedium::~DensityGridDecompositionMedium() { }

// Methods
/*override*/ Spectrum DensityGridDecompositionMedium::Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const
{
    // Transform ray from world coordinate to medium coordinate
    Ray rayMedium = worldToMedium(Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()), rayWorld.tMax * rayWorld.d.Norm()));
    if (std::isnan(rayMedium.tMax))
        return Spectrum(Float(1));

    // Ray casting and compute tMin, tMax
    const AABound3f bound = density->GridBoundf();
    Float tMin, tMax;
    if (!bound.Intersect(rayMedium, &tMin, &tMax) || std::isnan(tMax))
        return Spectrum(Float(1));

    // Decomposition tracking
    Float t = tMin;
    while (true)
    {
        // Step forward
        t -= std::log(1 - sampler.Get1D()) * mubar_inv;
        if (t >= tMax || std::isnan(t))
            break;

        // Compute sampling variables
        Float dense = density->Sample(rayMedium(t));
        Float kexi = sampler.Get1D();
        Spectrum mut = dense * toMut;
        Spectrum mus = albedo * mut;
        Spectrum mua = mut - mus;
        Spectrum mun = Spectrum(mubar) - mut;
        Spectrum muaR = mua - muaC;
        Spectrum musR = mus - musC;
        Float weighedAbsAvga = (muaR * *weigh).Absed().Average();
        Float weighedAbsAvgs = (musR * *weigh).Absed().Average();
        Float weighedAbsAvgn = (mun  * *weigh).Absed().Average();
        Float c_inv = Float(1) / (weighedAbsAvga + weighedAbsAvgs + weighedAbsAvgn);
        Float paR;
        Float psR;

        // Delta tracking
        Float cdf = 0;
        if (kexi < (cdf += paC))
        {
            // Set mi
            mi->emission = *weigh * muaC * (mubar_inv / paC) * Spectrum(); // TODO: Implement emission, maybe using another grid?
            // Terminate path
            return Spectrum();
        }
        else if (kexi < (cdf += psC))
        {
            // Update weigh
            *weigh *= musC * (mubar_inv / psC);
            // Set mi
            mi->p = Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()))(t);
            mi->woWorld = Vector3f(-rayWorld.d);
            mi->phase = phase;
            // Return albedo
            return albedo;
        }
        else if (kexi < (cdf += (paR = pResidual * weighedAbsAvga * c_inv)))
        {
            // Set mi
            mi->emission = *weigh * muaR * (mubar_inv / paR) * Spectrum(); // TODO: Implement emission, maybe using another grid?
            // Terminate path
            return Spectrum();
        }
        else if (kexi < (cdf += (psR = pResidual * weighedAbsAvgs * c_inv)))
        {
            // Update weigh
            *weigh *= musR * (mubar_inv / psR);
            // Set mi
            mi->p = Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()))(t);
            mi->woWorld = Vector3f(-rayWorld.d);
            mi->phase = phase;
            // Return albedo
            return albedo;
        }
        else
        {
            // Update weigh
            Float pn = pResidual * weighedAbsAvgn * c_inv;
            *weigh *= mun * (mubar_inv / pn);
        }
    }
    return Spectrum(Float(1));
}

/*override*/ Spectrum DensityGridDecompositionMedium::Tr(const Ray& rayWorld, Sampler& sampler) const
{
    // Transform ray from world coordinate to medium coordinate
    Ray rayMedium = worldToMedium(Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()), rayWorld.tMax * rayWorld.d.Norm()));
    if (std::isnan(rayMedium.tMax))
        return Spectrum(Float(1));

    // Ray casting and compute tMin, tMax
    const AABound3f bound = density->GridBoundf();
    Float tMin, tMax;
    if (!bound.Intersect(rayMedium, &tMin, &tMax) || std::isnan(tMax))
        return Spectrum(Float(1));

    // Residual ratio tracking
    Spectrum tc(std::exp(-mutC_homoC * (tMax - tMin)));
    Spectrum tr(Float(1));
    Float t = tMin;
    while (true)
    {
        t -= std::log(1 - sampler.Get1D()) * mubarR_homoC_inv;
        if (t >= tMax || std::isnan(t))
            break;
        Float dense = density->Sample(rayMedium(t));
        tr *= Spectrum(Float(1)) - Spectrum::Max(Spectrum(), (dense * toMut - mutC_homoC) * mubarR_homoC_inv);
        // TODO: Russian roulette
    }
    return tc * tr;
}



//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////*//
//* DensityKDTreeMediums ///////////////////////////////////////////////////////////////////////////////////////*//
//* ////////////////////////////////////////////////////////////////////////////////////////////////////////////*//

//* DensityKDTreeDeltaMedium -----------------------------------------------------------------------------------*//

DensityKDTreeDeltaMedium::DensityKDTreeDeltaMedium(
        const VolumeFloatKDTree* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium) : 
    DensityKDTreeMedium(density, toMut, albedo, phase, worldToMedium)
{
    toMut_maximum = toMut.Maximum();
}

/*override*/ DensityKDTreeDeltaMedium::~DensityKDTreeDeltaMedium() { }

// Methods
/*override*/ Spectrum DensityKDTreeDeltaMedium::Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const
{
    // Transform ray from world coordinate to medium coordinate
    Ray rayMedium = worldToMedium(Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()), rayWorld.tMax * rayWorld.d.Norm()));
    if (std::isnan(rayMedium.tMax))
        return Spectrum(Float(1));

    // Ray casting and compute tMin, tMax
    const AABound3f bound = density->GridBoundf();
    Float tMin, tMax;
    if (!bound.Intersect(rayMedium, &tMin, &tMax))
        return Spectrum(Float(1));

    // KDTree traversal states
    Vector3f rayMediumDInv(1 / rayMedium.d.x, 1 / rayMedium.d.y, 1 / rayMedium.d.z);
    bool isNonPositiveAxis[3] = { rayMedium.d.x <= 0, rayMedium.d.y <= 0, rayMedium.d.z <= 0 };
    struct KDTreeTask
    {
        int idx;
        Float tMin;
        Float tMax;
    };
    KDTreeTask tasks[VolumeFloatKDTree::KDTreeMaxDepth];
    int iTasks = 0;
    int i = 1;

    // KDTree traversal
    while (true)
    {
        if (rayMedium.tMax < tMin || std::isnan(tMin) || std::isnan(tMax))
            break;
        int axis = density->KDTreeSplitAxis(i);
        if (axis == 3) // Leaf
        {
            // Compute medium data in current node
            const Float mubar = density->KDTreeMaxValue(i) * toMut_maximum;
            const Float mubar_inv = Float(1) / mubar;

            // Delta tracking for the current node
            Float t = tMin;
            while (true)
            {
                // Step forward
                t -= std::log(1 - sampler.Get1D()) * mubar_inv;
                if (t >= tMax || std::isnan(t))
                    break;

                // Compute sampling variables
                Float dense = density->Sample(rayMedium(t));
                Float kexi = sampler.Get1D();
                Spectrum mut = dense * toMut;
                Spectrum mus = albedo * mut;
                Spectrum mua = mut - mus;
                Spectrum mun = Spectrum(mubar) - mut;
                Float weighedAbsAvga = (mua * *weigh).Absed().Average();
                Float weighedAbsAvgs = (mus * *weigh).Absed().Average();
                Float weighedAbsAvgn = (mun * *weigh).Absed().Average();
                Float c_inv = Float(1) / (weighedAbsAvga + weighedAbsAvgs + weighedAbsAvgn);
                Float pa;
                Float pa_plus_ps;
                Float pn;

                // Delta tracking
                if (kexi < (pa = weighedAbsAvga * c_inv))
                {
                    // Set mi
                    mi->emission = *weigh * mua * (mubar_inv / pa) * Spectrum(); // TODO: Implement emission, maybe using another grid?
                    // Terminate path
                    return Spectrum();
                }
                else if (kexi < (pa_plus_ps = Float(1) - (pn = weighedAbsAvgn * c_inv)))
                {
                    // Update weigh
                    Float ps = pa_plus_ps - pa;
                    *weigh *= mus * (mubar_inv / ps);
                    // Set mi
                    mi->p = Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()))(t);
                    mi->woWorld = Vector3f(-rayWorld.d);
                    mi->phase = phase;
                    // Return albedo
                    return albedo;
                }
                else
                {
                    // Update weigh
                    *weigh *= mun * (mubar_inv / pn);
                }
            }

            // Continue if sampling failed
            if (iTasks > 0)
            {
                --iTasks;
                i = tasks[iTasks].idx;
                tMin = tasks[iTasks].tMin;
                tMax = tasks[iTasks].tMax;
            }
            else
                break;
        }
        else // Interior
        {
            Float splitPos = (Float)density->KDTreeSplitPos(i);
            Float tSplit = (splitPos - rayMedium.o[axis]) * rayMediumDInv[axis];

            int firstChild, secondChild;
            bool belowfirst = (rayMedium.o[axis]  < splitPos) || 
                              (rayMedium.o[axis] == splitPos && isNonPositiveAxis[axis]);
            int leftChild = i << 1;
            int rightChild = leftChild + 1;
            if (belowfirst)
            {
                firstChild = leftChild;
                secondChild = rightChild;
            }
            else
            {
                secondChild = leftChild;
                firstChild = rightChild;
            }

            if (tSplit > tMax || tSplit <= 0)
                i = firstChild;
            else if (tSplit < tMin)
                i = secondChild;
            else
            {
                tasks[iTasks].idx = secondChild;
                tasks[iTasks].tMin = tSplit;
                tasks[iTasks].tMax = tMax;
                ++iTasks;
                i = firstChild;
                tMax = tSplit;
            }
        }
    }
    return Spectrum(Float(1));
}

/*override*/ Spectrum DensityKDTreeDeltaMedium::Tr(const Ray& rayWorld, Sampler& sampler) const
{
    // Transform ray from world coordinate to medium coordinate
    Ray rayMedium = worldToMedium(Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()), rayWorld.tMax * rayWorld.d.Norm()));
    if (std::isnan(rayMedium.tMax))
        return Spectrum(Float(1));

    // Ray casting and compute tMin, tMax
    const AABound3f bound = density->GridBoundf();
    Float tMin, tMax;
    if (!bound.Intersect(rayMedium, &tMin, &tMax))
        return Spectrum(Float(1));

    // KDTree traversal states
    Vector3f rayMediumDInv(1 / rayMedium.d.x, 1 / rayMedium.d.y, 1 / rayMedium.d.z);
    bool isNonPositiveAxis[3] = { rayMedium.d.x <= 0, rayMedium.d.y <= 0, rayMedium.d.z <= 0 };
    struct KDTreeTask
    {
        int idx;
        Float tMin;
        Float tMax;
    };
    KDTreeTask tasks[VolumeFloatKDTree::KDTreeMaxDepth];
    int iTasks = 0;
    int i = 1;

    // KDTree traversal
    Spectrum tr(Float(1));
    while (true)
    {
        if (rayMedium.tMax < tMin || std::isnan(tMin) || std::isnan(tMax))
            break;
        int axis = density->KDTreeSplitAxis(i);
        if (axis == 3) // Leaf
        {
            // Get mubar
            const Float mubar = density->KDTreeMaxValue(i) * toMut_maximum;
            const Float mubar_inv = Float(1) / mubar;

            // Ratio tracking for the current node
            Float t = tMin;
            while (true)
            {
                t -= std::log(1 - sampler.Get1D()) * mubar_inv;
                if (t >= tMax || std::isnan(t))
                    break;
                Float dense = density->Sample(rayMedium(t));
                tr *= Spectrum(Float(1)) - Spectrum::Max(Spectrum(), dense * mubar_inv * toMut);
                // TODO: Russian roulette
            }

            // Continue if sampling failed
            if (iTasks > 0)
            {
                --iTasks;
                i = tasks[iTasks].idx;
                tMin = tasks[iTasks].tMin;
                tMax = tasks[iTasks].tMax;
            }
            else
                break;
        }
        else // Interior
        {
            Float splitPos = (Float)density->KDTreeSplitPos(i);
            Float tSplit = (splitPos - rayMedium.o[axis]) * rayMediumDInv[axis];

            int firstChild, secondChild;
            bool belowfirst = (rayMedium.o[axis]  < splitPos) || 
                              (rayMedium.o[axis] == splitPos && isNonPositiveAxis[axis]);
            int leftChild = i << 1;
            int rightChild = leftChild + 1;
            if (belowfirst)
            {
                firstChild = leftChild;
                secondChild = rightChild;
            }
            else
            {
                secondChild = leftChild;
                firstChild = rightChild;
            }

            if (tSplit > tMax || tSplit <= 0)
                i = firstChild;
            else if (tSplit < tMin)
                i = secondChild;
            else
            {
                tasks[iTasks].idx = secondChild;
                tasks[iTasks].tMin = tSplit;
                tasks[iTasks].tMax = tMax;
                ++iTasks;
                i = firstChild;
                tMax = tSplit;
            }
        }
    }
    return tr;
}



//* DensityKDTreeDecompositionMedium ---------------------------------------------------------------------------*//

// Constructors
DensityKDTreeDecompositionMedium::DensityKDTreeDecompositionMedium(
        const VolumeFloatKDTree* density, 
        const Spectrum& toMut, 
        const Spectrum& albedo, 
        const PhaseFunction* phase, 
        const Transform& worldToMedium) : 
    DensityKDTreeMedium(density, toMut, albedo, phase, worldToMedium)
{
    toMut_maximum = toMut.Maximum();
}

/*override*/ DensityKDTreeDecompositionMedium::~DensityKDTreeDecompositionMedium() { }

// Methods
/*override*/ Spectrum DensityKDTreeDecompositionMedium::Sample(const Ray& rayWorld, Sampler& sampler, Spectrum* weigh, MediumInteraction* mi) const
{
    // Transform ray from world coordinate to medium coordinate
    Ray rayMedium = worldToMedium(Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()), rayWorld.tMax * rayWorld.d.Norm()));
    if (std::isnan(rayMedium.tMax))
        return Spectrum(Float(1));

    // Ray casting and compute tMin, tMax
    const AABound3f bound = density->GridBoundf();
    Float tMin, tMax;
    if (!bound.Intersect(rayMedium, &tMin, &tMax))
        return Spectrum(Float(1));

    // KDTree traversal states
    Vector3f rayMediumDInv(1 / rayMedium.d.x, 1 / rayMedium.d.y, 1 / rayMedium.d.z);
    bool isNonPositiveAxis[3] = { rayMedium.d.x <= 0, rayMedium.d.y <= 0, rayMedium.d.z <= 0 };
    struct KDTreeTask
    {
        int idx;
        Float tMin;
        Float tMax;
    };
    KDTreeTask tasks[VolumeFloatKDTree::KDTreeMaxDepth];
    int iTasks = 0;
    int i = 1;

    // KDTree traversal
    int numIterations_outer = 0;
    while (true)
    {
        ++numIterations_outer;
        if (numIterations_outer / 500 > 0 && numIterations_outer % 500 == 0)
            printf("\n\n(Thread %d): Long loop detected in outer loop of DensityKDTreeDecompositionMedium::Sample(): i=%d, iTasks=%d, tMin=%f, tMax=%f\n\n", omp_get_thread_num(), i, iTasks, tMin, tMax);
        if (rayMedium.tMax < tMin || std::isnan(tMin) || std::isnan(tMax))
            break;
        int axis = density->KDTreeSplitAxis(i);
        if (axis == 3) // Leaf
        {
            // Compute medium data in current node
            const Float mubar = density->KDTreeMaxValue(i) * toMut_maximum; // Compute mubar
            const Float mubar_inv = Float(1) / mubar;
            const Spectrum mutC = density->GridMinValue() * toMut; // Compute mu
            const Spectrum musC = albedo * mutC;
            const Spectrum muaC = mutC - musC;
            const Float paC = (muaC * mubar_inv).Average(); // Compute p
            const Float psC = (musC * mubar_inv).Average();
            const Float pResidual = Float(1) - paC - psC;

            // Decomposition tracking for the current node
            Float t = tMin;
            int numIterations_inner = 0;
            while (true)
            {
                ++numIterations_inner;
                if (numIterations_inner / 500 > 0 && numIterations_inner % 500 == 0)
                    printf("\n\n(Thread %d): Long loop detected in inner loop of DensityKDTreeDecompositionMedium::Sample(): i=%d, iTasks=%d, tMin=%f, tMax=%f, t=%f\n\n", omp_get_thread_num(), i, iTasks, tMin, tMax, t);
                // Step forward
                t -= std::log(1 - sampler.Get1D()) * mubar_inv;
                if (t >= tMax || std::isnan(t))
                    break;

                // Compute sampling variables
                Float dense = density->Sample(rayMedium(t));
                Float kexi = sampler.Get1D();
                Spectrum mut = dense * toMut;
                Spectrum mus = albedo * mut;
                Spectrum mua = mut - mus;
                Spectrum mun = Spectrum(mubar) - mut;
                Spectrum muaR = mua - muaC;
                Spectrum musR = mus - musC;
                Float weighedAbsAvga = (muaR * *weigh).Absed().Average();
                Float weighedAbsAvgs = (musR * *weigh).Absed().Average();
                Float weighedAbsAvgn = (mun  * *weigh).Absed().Average();
                Float c_inv = Float(1) / (weighedAbsAvga + weighedAbsAvgs + weighedAbsAvgn);
                Float paR;
                Float psR;

                // Delta tracking
                Float cdf = 0;
                if (kexi < (cdf += paC))
                {
                    // Set mi
                    mi->emission = *weigh * muaC * (mubar_inv / paC) * Spectrum(); // TODO: Implement emission, maybe using another grid?
                    // Terminate path
                    return Spectrum();
                }
                else if (kexi < (cdf += psC))
                {
                    // Update weigh
                    *weigh *= musC * (mubar_inv / psC);
                    // Set mi
                    mi->p = Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()))(t);
                    mi->woWorld = Vector3f(-rayWorld.d);
                    mi->phase = phase;
                    // Return albedo
                    return albedo;
                }
                else if (kexi < (cdf += (paR = pResidual * weighedAbsAvga * c_inv)))
                {
                    // Set mi
                    mi->emission = *weigh * muaR * (mubar_inv / paR) * Spectrum(); // TODO: Implement emission, maybe using another grid?
                    // Terminate path
                    return Spectrum();
                }
                else if (kexi < (cdf += (psR = pResidual * weighedAbsAvgs * c_inv)))
                {
                    // Update weigh
                    *weigh *= musR * (mubar_inv / psR);
                    // Set mi
                    mi->p = Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()))(t);
                    mi->woWorld = Vector3f(-rayWorld.d);
                    mi->phase = phase;
                    // Return albedo
                    return albedo;
                }
                else
                {
                    // Update weigh
                    Float pn = pResidual * weighedAbsAvgn * c_inv;
                    *weigh *= mun * (mubar_inv / pn);
                }
            }

            // Continue if sampling failed
            if (iTasks > 0)
            {
                --iTasks;
                i = tasks[iTasks].idx;
                tMin = tasks[iTasks].tMin;
                tMax = tasks[iTasks].tMax;
            }
            else
                break;
        }
        else // Interior
        {
            Float splitPos = (Float)density->KDTreeSplitPos(i);
            Float tSplit = (splitPos - rayMedium.o[axis]) * rayMediumDInv[axis];

            int firstChild, secondChild;
            bool belowfirst = (rayMedium.o[axis]  < splitPos) || 
                              (rayMedium.o[axis] == splitPos && isNonPositiveAxis[axis]);
            int leftChild = i << 1;
            int rightChild = leftChild + 1;
            if (belowfirst)
            {
                firstChild = leftChild;
                secondChild = rightChild;
            }
            else
            {
                secondChild = leftChild;
                firstChild = rightChild;
            }

            if (tSplit > tMax || tSplit <= 0)
                i = firstChild;
            else if (tSplit < tMin)
                i = secondChild;
            else
            {
                tasks[iTasks].idx = secondChild;
                tasks[iTasks].tMin = tSplit;
                tasks[iTasks].tMax = tMax;
                ++iTasks;
                i = firstChild;
                tMax = tSplit;
            }
        }
    }
    return Spectrum(Float(1));
}

/*override*/ Spectrum DensityKDTreeDecompositionMedium::Tr(const Ray& rayWorld, Sampler& sampler) const
{
    // Transform ray from world coordinate to medium coordinate
    Ray rayMedium = worldToMedium(Ray(rayWorld.o, Vector3f(rayWorld.d.Normalized()), rayWorld.tMax * rayWorld.d.Norm()));
    if (std::isnan(rayMedium.tMax))
        return Spectrum(Float(1));

    // Ray casting and compute tMin, tMax
    const AABound3f bound = density->GridBoundf();
    Float tMin, tMax;
    if (!bound.Intersect(rayMedium, &tMin, &tMax))
        return Spectrum(Float(1));

    // KDTree traversal states
    Vector3f rayMediumDInv(1 / rayMedium.d.x, 1 / rayMedium.d.y, 1 / rayMedium.d.z);
    bool isNonPositiveAxis[3] = { rayMedium.d.x <= 0, rayMedium.d.y <= 0, rayMedium.d.z <= 0 };
    struct KDTreeTask
    {
        int idx;
        Float tMin;
        Float tMax;
    };
    KDTreeTask tasks[VolumeFloatKDTree::KDTreeMaxDepth];
    int iTasks = 0;
    int i = 1;

    // KDTree traversal
    Spectrum tr(Float(1));
    int numIterations_outer = 0;
    while (true)
    {
        ++numIterations_outer;
        if (numIterations_outer / 500 > 0 && numIterations_outer % 500 == 0)
            printf("\n\n(Thread %d): Long loop detected in outer loop of DensityKDTreeDecompositionMedium::Tr(): i=%d, iTasks=%d, tMin=%f, tMax=%f\n\n", omp_get_thread_num(), i, iTasks, tMin, tMax);
        if (rayMedium.tMax < tMin || std::isnan(tMin) || std::isnan(tMax))
            break;
        int axis = density->KDTreeSplitAxis(i);
        if (axis == 3) // Leaf
        {
            // Compute sampling variables
            const Float mubar = density->KDTreeMaxValue(i) * toMut_maximum;
            const Spectrum mutC = density->KDTreeMinValue(i) * toMut;
            const Float mutC_homoC = mutC.Minimum();
            const Float mubarR_homoC_inv = Float(1) / (mubar - mutC_homoC);

            // Residual ratio tracking for the current node
            Spectrum tc(std::exp(-mutC_homoC * (tMax - tMin)));
            tr *= tc;
            Float t = tMin;
            int numIterations_inner = 0;
            while (true)
            {
                ++numIterations_inner;
                if (numIterations_inner / 500 > 0 && numIterations_inner % 500 == 0)
                    printf("\n\n(Thread %d): Long loop detected in inner loop of DensityKDTreeDecompositionMedium::Tr(): i=%d, iTasks=%d, tMin=%f, tMax=%f, t=%f\n\n", omp_get_thread_num(), i, iTasks, tMin, tMax, t);
                t -= std::log(1 - sampler.Get1D()) * mubarR_homoC_inv;
                if (t >= tMax || std::isnan(t))
                    break;
                Float dense = density->Sample(rayMedium(t));
                tr *= Spectrum(Float(1)) - Spectrum::Max(Spectrum(), (dense * toMut - mutC_homoC) * mubarR_homoC_inv);
                // TODO: Russian roulette
            }

            // Continue if sampling failed
            if (iTasks > 0)
            {
                --iTasks;
                i = tasks[iTasks].idx;
                tMin = tasks[iTasks].tMin;
                tMax = tasks[iTasks].tMax;
            }
            else
                break;
        }
        else // Interior
        {
            Float splitPos = (Float)density->KDTreeSplitPos(i);
            Float tSplit = (splitPos - rayMedium.o[axis]) * rayMediumDInv[axis];

            int firstChild, secondChild;
            bool belowfirst = (rayMedium.o[axis]  < splitPos) || 
                              (rayMedium.o[axis] == splitPos && isNonPositiveAxis[axis]);
            int leftChild = i << 1;
            int rightChild = leftChild + 1;
            if (belowfirst)
            {
                firstChild = leftChild;
                secondChild = rightChild;
            }
            else
            {
                secondChild = leftChild;
                firstChild = rightChild;
            }

            if (tSplit > tMax || tSplit <= 0)
                i = firstChild;
            else if (tSplit < tMin)
                i = secondChild;
            else
            {
                tasks[iTasks].idx = secondChild;
                tasks[iTasks].tMin = tSplit;
                tasks[iTasks].tMax = tMax;
                ++iTasks;
                i = firstChild;
                tMax = tSplit;
            }
        }
    }
    return tr;
}
