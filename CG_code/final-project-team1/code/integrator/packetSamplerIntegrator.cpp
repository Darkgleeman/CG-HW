#include <packetSamplerIntegrator.h>

// Constructors
PacketSamplerIntegrator::PacketSamplerIntegrator(
        shared_ptr<const Camera>& camera, shared_ptr<Sampler>& sampler, 
        int imageWidth, int imageHeigh, int samplesPerPixel, int depth, 
        int packetSize) : 
    camera(camera), sampler(sampler), 
    imageWidth(imageWidth), imageHeigh(imageHeigh), samplesPerPixel(samplesPerPixel), depth(depth), 
    packetSize(packetSize)
{
    if (packetSize != 4 && packetSize != 8 && packetSize != 16)
    {
        Error("PacketSamplerIntegrator::PacketSamplerIntegrator(): Currently does not support such packet size");
        exit(-1);
    }
    if (samplesPerPixel % packetSize != 0)
    {
        Error("PacketSamplerIntegrator::PacketSamplerIntegrator(): Sampler per pixel should match packet size");
        exit(-1);
    }
}

// Methods
/*override*/ void PacketSamplerIntegrator::Render(const Scene& scene)
{
    // Create image
    string workDir = GetFilePath("build");
    string imageDir = workDir + "/../";
    const string imageName = "image.ppm";
    std::ofstream of_image;
    of_image.open(imageDir + imageName);

    // Write image header
    of_image << "P3" << std::endl;
    of_image << imageWidth << ' ' << imageHeigh << std::endl;
    of_image << 255 << std::endl;

    // Integrate
    vector<vector<Point3i>> image(imageHeigh);
    for (vector<Point3i>& scanline : image)
        scanline.resize(imageWidth);
    int scanlines_remaining = imageHeigh;
    omp_set_num_threads(omp_get_max_threads());
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
            Spectrum packetSpectrum[16];
            for (int s = 0; s < samplesPerPixel; s += packetSize) {
                Ray rays[16];
                for (int p = 0; p < packetSize; ++p)
                {
                    Float u = (i + sampler->Get1D()) / (imageWidth - 1);
                    Float v = (j + sampler->Get1D()) / (imageHeigh - 1);
                    rays[p] = camera->GenerateRay(u, v);
                }
                Spectrum integration[16];
                Integrate(rays, scene, 0, integration);
                for (int p = 0; p < packetSize; ++p)
                {
                    if (integration[p].HasNans())
                        integration[p] = Spectrum();
                    packetSpectrum[p] += integration[p];
                }
            }
            Spectrum pixelSpectrum;
            for (int p = 0; p < packetSize; ++p)
                pixelSpectrum += packetSpectrum[p];
            pixelSpectrum *= Float(1) / samplesPerPixel;
            Float rgb[3];
            pixelSpectrum.ToRGB(rgb);
            image[j][i] = RGBFloatsToRGB256(rgb);
        }
    }
    auto chronoEnd = std::chrono::system_clock::now();
    for (int j = imageHeigh - 1; j >= 0; --j)
        for (int i = 0; i < imageWidth; ++i)
            of_image << image[j][i].x << ' ' << image[j][i].y << ' ' << image[j][i].z << std::endl;
    of_image.close();
    std::chrono::duration<double> chronoDuration(chronoEnd - chronoStart);
    std::cerr << std::endl << "Done after "<< chronoDuration.count() << " seconds" << std::endl;
}
