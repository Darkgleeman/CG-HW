#include <samplerIntegrator.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include <stb_image_write.h>

// Constructors
SamplerIntegrator::SamplerIntegrator(shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
                                     int imageWidth, int imageHeigh, int samplesPerPixel, int depth) : 
    camera(camera), sampler(sampler), 
    imageWidth(imageWidth), imageHeigh(imageHeigh), samplesPerPixel(samplesPerPixel), depth(depth) { }

// Methods
/*override*/ void SamplerIntegrator::Render(const Scene& scene)
{
    // OpenMP configure
    omp_set_num_threads(omp_get_max_threads());

    // Find CWD
    string workDir = GetFilePath("build");

    // Different rendering strategy based on SPP
    if (samplesPerPixel < 128) // TODO: Magic number
    {
        // Integrate
        uint8_t* image255 = new uint8_t[imageHeigh * imageHeigh * 3];
        int scanlines_remaining = imageHeigh;
        auto chronoStart = std::chrono::system_clock::now();
        #pragma omp parallel for schedule(dynamic, 4)
        for (int j = imageHeigh - 1; j >= 0; --j)
        {
            #pragma omp critical
            {
                --scanlines_remaining;
                std::cerr << "\rScanlines remaining: " << scanlines_remaining << ' ' << std::flush;
            }
            for (int i = 0; i < imageWidth; ++i) {
                Spectrum pixelSpectrum;
                for (int s = 0; s < samplesPerPixel; ++s) {
                    Float u = (i + sampler->Get1D()) / (imageWidth - 1);
                    Float v = (j + sampler->Get1D()) / (imageHeigh - 1);
                    Ray r = camera->GenerateRay(u, v);
                    Spectrum integration = Integrate(r, scene, 0);
                    if (integration.HasNans())
                        integration = Spectrum();
                    pixelSpectrum += integration;
                }
                pixelSpectrum *= Float(1) / samplesPerPixel;
                Float rgb[3];
                pixelSpectrum.ToRGB(rgb);
                Point3i rgb256 = RGBFloatsToRGB256(rgb);
                int imageI = j * imageWidth * 3 + i * 3;
                image255[imageI] = (uint8_t)rgb256.x;
                image255[imageI + 1] = (uint8_t)rgb256.y;
                image255[imageI + 2] = (uint8_t)rgb256.z;
            }
        }
        auto chronoEnd = std::chrono::system_clock::now();
        // Write image
        string imageDir = workDir + "/../";
        const string imageName = "image.png";
        stbi_flip_vertically_on_write(true);
        stbi_write_png((imageDir + imageName).c_str(), imageWidth, imageHeigh, 3, image255, 0);
        std::chrono::duration<double> chronoDuration(chronoEnd - chronoStart);
        std::cerr << std::endl << "Done after "<< chronoDuration.count() << " seconds" << std::endl;
        delete[] image255;
    }
    else
    {
        // Variables
        std::stringstream ss;
        // Create temp directory if not exists
        string tempDir = workDir + "/../temp/";
        if (std::filesystem::exists(tempDir) == false && std::filesystem::create_directory(tempDir) == false)
        {
            std::cerr << "ERROR: SamplerIntegrator::Render(): Failed to create temp directory" << std::endl;
            exit(-1);
        }

        // Use real random numbers if RandomSampler is used
        if (sampler->IsRandomSampler())
        {
            #pragma omp parallel for
            for (int i = 0; i < omp_get_num_threads(); ++i)
                std::dynamic_pointer_cast<RandomSampler>(sampler)->UseRealRandomSeed();
        }

        // Get start filename
        int iFile;
        for (iFile = 0; ; ++iFile)
        {
            ss.clear();
            ss.str("");
            ss << tempDir << iFile << ".temp";
            string filepath = ss.str();
            if (!std::filesystem::exists(filepath)) // iFile.temp not exists
                break;
            else // iFile.temp exists
            {
                std::ifstream if_spectrum(filepath, std::ios::binary);
                uint32_t fileHeader[4];
                if_spectrum.read((char*)fileHeader, sizeof(fileHeader));
                if (uint32_t(imageWidth) != fileHeader[0] || uint32_t(imageHeigh) != fileHeader[1] || uint32_t(depth) != fileHeader[2] || uint32_t(sizeof(Float)) != fileHeader[3])
                {
                    std::cerr << "ERROR: SamplerIntegrator::Render(): Temporary file mismatch" << std::endl;
                    exit(-1);
                }
                if_spectrum.close();
            }
        }

        // Integrate
        int numTempFiles = (samplesPerPixel - 1) / 128 + 1; // TODO: Magic number
        int actualSamplesPerPixel = numTempFiles * 128; // TODO: Magic number
        for (; iFile < numTempFiles; ++iFile)
        {
            // Check existance
            ss.clear();
            ss.str("");
            ss << tempDir << iFile << ".temp";
            string filepath = ss.str();
            if (std::filesystem::exists(filepath))
            {
                std::cerr << "WARNING: SamplerIntegrator::Render(): Temporary file \"" << iFile << ".temp\" already exists" << std::endl;
                continue;
            }

            // Integrate
            Spectrum* image_temp = new Spectrum[imageHeigh * imageWidth];
            int scanlines_remaining = imageHeigh;
            auto chronoStart = std::chrono::system_clock::now();
            #pragma omp parallel for schedule(dynamic, 4)
            for (int j = imageHeigh - 1; j >= 0; --j)
            {
                #pragma omp critical
                {
                    --scanlines_remaining;
                    std::cerr << "\rScanlines remaining: " << scanlines_remaining << ' ' << std::flush;
                }
                for (int i = 0; i < imageWidth; ++i) {
                    Spectrum pixelSpectrum;
                    for (int s = 0; s < 128; ++s) { // TODO: Magic number
                        Float u = (i + sampler->Get1D()) / (imageWidth - 1);
                        Float v = (j + sampler->Get1D()) / (imageHeigh - 1);
                        Ray r = camera->GenerateRay(u, v);
                        Spectrum integration = Integrate(r, scene, 0);
                        if (integration.HasNans())
                            integration = Spectrum();
                        pixelSpectrum += integration;
                    }
                    image_temp[j * imageWidth + i] = pixelSpectrum;
                }
            }
            auto chronoEnd = std::chrono::system_clock::now();

            // Write spectrum
            uint32_t fileHeader[4] = { (uint32_t)imageWidth, (uint32_t)imageHeigh, (uint32_t)depth, (uint32_t)sizeof(Float) };
            std::ofstream of_spectrum(filepath, std::ios::binary);
            of_spectrum.write((const char*)fileHeader, sizeof(fileHeader));
            of_spectrum.write((const char*)image_temp, sizeof(Spectrum) * imageHeigh * imageWidth);
            of_spectrum.close();
            std::chrono::duration<double> chronoDuration(chronoEnd - chronoStart);
            ss.clear();
            ss.str("");
            ss << iFile << ".temp";
            string filename = ss.str();
            std::cerr << std::endl << "Temporary file \"" << filename << "\" done after "<< chronoDuration.count() << " seconds" << std::endl;
            delete[] image_temp;
        }

        // Merge
        Spectrum* image = new Spectrum[imageHeigh * imageWidth];
        for (int i = 0; i < imageHeigh * imageWidth; ++i)
            image[i] = Spectrum();
        Spectrum* image_temp = new Spectrum[imageHeigh * imageWidth];
        for (iFile = 0; iFile < numTempFiles; ++iFile)
        {
            // Check file
            ss.clear();
            ss.str("");
            ss << tempDir << iFile << ".temp";
            string filepath = ss.str();
            if (!std::filesystem::exists(filepath))
            {
                std::cerr << "ERROR: SamplerIntegrator::Render(): Temporary file \"" << iFile << ".temp\" not exists" << std::endl;
                exit(-1);
            }
            std::ifstream if_spectrum(filepath, std::ios::binary);
            uint32_t fileHeader[4];
            if_spectrum.read((char*)fileHeader, sizeof(fileHeader));
            if (uint32_t(imageWidth) != fileHeader[0] || uint32_t(imageHeigh) != fileHeader[1] || uint32_t(depth) != fileHeader[2] || uint32_t(sizeof(Float)) != fileHeader[3])
            {
                std::cerr << "ERROR: SamplerIntegrator::Render(): Temporary file \"" << iFile << ".temp\" to merge has wrong format" << std::endl;
                exit(-1);
            }
            // Merge
            if_spectrum.read((char*)image_temp, sizeof(Spectrum) * imageHeigh * imageWidth);
            for (int j = imageHeigh - 1; j >= 0; --j)
                for (int i = 0; i < imageWidth; ++i)
                    image[j * imageWidth + i] += image_temp[j * imageWidth + i];
            if_spectrum.close();
        }
        delete[] image_temp;

        // Write image
        uint8_t* image255 = new uint8_t[imageHeigh * imageHeigh * 3];
        for (int j = imageHeigh - 1; j >= 0; --j)
            for (int i = 0; i < imageWidth; ++i)
            {
                Spectrum pixelSpectrum = image[j * imageWidth + i];
                pixelSpectrum *= Float(1) / actualSamplesPerPixel;
                Float rgbSpectrum[3];
                pixelSpectrum.ToRGB(rgbSpectrum);
                Point3i rgb256 = RGBFloatsToRGB256(rgbSpectrum);
                int imageI = j * imageWidth * 3 + i * 3;
                image255[imageI] = (uint8_t)rgb256.x;
                image255[imageI + 1] = (uint8_t)rgb256.y;
                image255[imageI + 2] = (uint8_t)rgb256.z;
            }
        string imageDir = workDir + "/../";
        const string imageName = "image.png";
        stbi_flip_vertically_on_write(true);
        stbi_write_png((imageDir + imageName).c_str(), imageWidth, imageHeigh, 3, image255, 0);
        delete[] image;
        delete[] image255;
        std::cerr << "Merging of temporary file done" << std::endl;
    }
}

void SamplerIntegrator::ComputeDifferentialRays(const Ray& ray, SurfaceInteraction* si) const
{
    Float t = (camera->nearPlane.w - (camera->nearPlane.x * ray.o.x + camera->nearPlane.y * ray.o.y + camera->nearPlane.z * ray.o.z)) / (camera->nearPlane.x * ray.d.x + camera->nearPlane.y * ray.d.y + camera->nearPlane.z * ray.d.z);
    Assert(t >= 0 && !std::isinf(t), 
           "SamplerIntegrator::ComputeDifferentialRays(): T out of bound");
    Point3f pNear = ray(t);
    Vector3f cameraToPNear(pNear - camera->position);
    si->rayONextPixelX = camera->position;
    si->rayONextPixelY = camera->position;
    si->rayDNextPixelX = Vector3f(cameraToPNear + pixelPosOffset_X);
    si->rayDNextPixelY = Vector3f(cameraToPNear + pixelPosOffset_Y);
}

pair<Ray, Ray> SamplerIntegrator::GenerateRayAtNextPixel(Ray ray) const
{
    Float t = (camera->nearPlane.w - (camera->nearPlane.x * ray.o.x + camera->nearPlane.y * ray.o.y + camera->nearPlane.z * ray.o.z)) / (camera->nearPlane.x * ray.d.x + camera->nearPlane.y * ray.d.y + camera->nearPlane.z * ray.d.z);
    Assert(t >= 0 && !std::isinf(t), 
           "SamplerIntegrator::GenerateRayAtNextPixel(): T out of bound");
    Point3f pNear = ray(t);
    Vector3f cameraToPNear(pNear - camera->position);
    return pair<Ray, Ray>(Ray(camera->position, Vector3f(cameraToPNear + pixelPosOffset_X)), 
                          Ray(camera->position, Vector3f(cameraToPNear + pixelPosOffset_Y)));
}

// Supporting methods
Ray SamplerIntegrator::GenerateRayAtNextPixelX(Ray ray) const
{
    Float t = (camera->nearPlane.w - (camera->nearPlane.x * ray.o.x + camera->nearPlane.y * ray.o.y + camera->nearPlane.z * ray.o.z)) / (camera->nearPlane.x * ray.d.x + camera->nearPlane.y * ray.d.y + camera->nearPlane.z * ray.d.z);
    Assert(t >= 0 && !std::isinf(t), 
           "SamplerIntegrator::GenerateRayAtNextPixelX(): T out of bound");
    Point3f pNear = ray(t);
    return Ray(camera->position, Vector3f(pNear + pixelPosOffset_X - camera->position));
}

Ray SamplerIntegrator::GenerateRayAtNextPixelY(Ray ray) const
{
    Float t = (camera->nearPlane.w - (camera->nearPlane.x * ray.o.x + camera->nearPlane.y * ray.o.y + camera->nearPlane.z * ray.o.z)) / (camera->nearPlane.x * ray.d.x + camera->nearPlane.y * ray.d.y + camera->nearPlane.z * ray.d.z);
    Assert(t >= 0 && !std::isinf(t), 
           "SamplerIntegrator::GenerateRayAtNextPixelY(): T out of bound");
    Point3f pNear = ray(t);
    return Ray(camera->position, Vector3f(pNear + pixelPosOffset_Y - camera->position));
}
