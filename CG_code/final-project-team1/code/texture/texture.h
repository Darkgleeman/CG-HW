#ifndef TEXTURE_H
#define TEXTURE_H

#include <geometrics.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

template <typename T, typename Evaluator>
class Texture
{
public:
    virtual T Evaluate(const Evaluator& evaluator) const = 0;
};
template <typename T> class Texture2f : public Texture<T, Iso2f> { };
template <typename T> class TextureSi : public Texture<T, SurfaceInteraction> { };

template <typename T>
class FunctionTexture2f : public Texture2f<T>
{
public:
    // Constructors
    FunctionTexture2f(const std::function<T(const Iso2f&)>& function) : function(function) { }

    // Methods
    virtual T Evaluate(const Iso2f& evaluator) const override
    {
        return function(evaluator);
    }
private:
    // Fields
    const std::function<T(const Iso2f&)> function;
};

template <typename T>
class SingleImageTexture2f : public Texture2f<T>
{
public:
    // Constructors
    SingleImageTexture2f(const string& imagePath, int desiredChannels = 3) : 
        channels(desiredChannels)
    {
        // 从文件中加载纹理
        Assert(desiredChannels <= dimension, 
               "SingleImageTexture2f::SingleImageTexture2f(): Desired channels out of bound");
        string relativePath = GetFilePath(imagePath);
        int channelsInFile;
        unsigned char* data_255 = stbi_load(relativePath.c_str(), &width, &heigh, &channelsInFile, desiredChannels);
        Assert(data_255, 
               "SingleImageTexture2f::SingleImageTexture2f(): Failed to load image file");
        Assert(desiredChannels <= channelsInFile, 
               "SingleImageTexture2f::SingleImageTexture2f(): Desired channels out of bound");
        int bytesPerScanline = channels * width;
        // 从 255 转化为 Float
        const Float byteToFloat = (long double)1 / (long double)255;
        data = new T*[width + 2]; //! data 边缘留 1，使得 Evaluate() 更方便
        for (int i = -1; i < width + 1; ++i)
        {
            data[i + 1] = new T[heigh + 2];
            int iEval = std::clamp(i, 0, width - 1);
            for (int j = -1; j < heigh + 1; ++j)
            {
                int jEval = std::clamp(j, 0, heigh - 1);
                Float values[dimension];
                for (int dim = 0; dim < dimension; ++dim)
                    values[dim] = byteToFloat * data_255[jEval * bytesPerScanline + iEval * channels + dim];
                data[i + 1][j + 1] = T(values);
            }
        }
        // 清除数据
        delete data_255;
    }
    SingleImageTexture2f(int channels, T** data, int width, int heigh) : 
        channels(channels), 
        data(data), 
        width(width), 
        heigh(heigh)
    { }
    ~SingleImageTexture2f()
    {
        for (int i = 0; i < width + 2; ++i)
            delete[] data[i];
        delete[] data;
    }

    // Methods
    virtual T Evaluate(const Iso2f& evaluator) const override
    {
        Assert(evaluator.x >= 0 && evaluator.x <= 1 && 
               evaluator.y >= 0 && evaluator.y <= 1, 
               "SingleImageTexture2f::Evaluate(): Evaluator out of bound");
        // 将 uv 从 texture coordinate 变换到 image coordinate
        Float u = evaluator.x;
        Float v = 1 - evaluator.y;
        // 二维线性插值
        Float iF = u * width; // iF ∈ [0, width]
        Float jF = v * heigh; // jF ∈ [0, heigh]
        Float iFShift = iF + 0.5;
        Float jFShift = jF + 0.5;
        int i = static_cast<int>(iFShift); // i ∈ [0, width]
        int j = static_cast<int>(jFShift); // j ∈ [0, heigh]
        Float uTexel = iFShift - i;
        Float vTexel = jFShift - j;
        return T(T::Lerp(data[i][j], data[i + 1][j], data[i][j + 1], data[i + 1][j + 1], uTexel, vTexel)); //! 外面套一层 T() 用于处理继承关系：可能只有基类定义了 Lerp()
    }

    virtual shared_ptr<SingleImageTexture2f> Resized(int resizedWidth, int resizedHeigh) const
    {
        Assert(resizedWidth <= width && resizedHeigh <= heigh, 
               "SingleImageTexture2f::Resized(): Resized image should be smaller");
        Vector2f stride(Float(1) / resizedWidth, Float(1) / resizedHeigh);
        Point2f checkPointStart(stride * 0.5);
        T** resizedData = new T*[resizedWidth + 2];
        for (int i = -1; i < resizedWidth + 1; ++i)
        {
            resizedData[i + 1] = new T[resizedHeigh + 2]; 
            int iEval = std::clamp(i, 0, resizedWidth - 1);
            for (int j = -1; j < resizedHeigh + 1; ++j)
            {
                int jEval = std::clamp(j, 0, resizedHeigh - 1);
                resizedData[i + 1][j + 1] = Evaluate(checkPointStart + Vector2f(iEval * stride.x, jEval * stride.y));
            }
        }
        return make_shared<SingleImageTexture2f>(channels, resizedData, resizedWidth, resizedHeigh);
    }
// protected:
    // Fields
    const int channels; // Channels = bytesPerPixel
    T** data;
    int width;
    int heigh;
    // Supporting fields
    static constexpr int dimension = sizeof(T) / sizeof(Float);
};

enum class BumpMapType
{
    Override, 
    Distort
};

template <typename T>
class FunctionTextureSi : public TextureSi<T>
{
public:
    // Constructors
    FunctionTextureSi(const shared_ptr<FunctionTexture2f<T>>& texture2f) : texture2f(texture2f) { }
    FunctionTextureSi(const std::function<T(const Iso2f&)>& function) : texture2f(make_shared<FunctionTexture2f<T>>(function)) { }

    // Methods
    virtual T Evaluate(const SurfaceInteraction& evaluator) const override { return texture2f->Evaluate(evaluator.uv); }

protected:
    // Fields
    const shared_ptr<FunctionTexture2f<T>> texture2f;
};

template <typename T>
class SingleImageTextureSi : public TextureSi<T>
{
public:
    // Constructors
    SingleImageTextureSi(const shared_ptr<SingleImageTexture2f<T>>& texture2f) : texture2f(texture2f) { }
    SingleImageTextureSi(const string& imagePath, int desiredChannels = 3) : texture2f(make_shared<SingleImageTexture2f<T>>(imagePath, desiredChannels)) { }
    SingleImageTextureSi(int channels, T** data, int width, int heigh) : texture2f(make_shared<SingleImageTexture2f<T>>(channels, data, width, heigh)) { }

    // Methods
    virtual T Evaluate(const SurfaceInteraction& evaluator) const override { return texture2f->Evaluate(evaluator.uv); }
    virtual shared_ptr<SingleImageTextureSi> Resized(int resizedWidth, int resizedHeigh) const { return make_shared<SingleImageTextureSi>(texture2f->Resized(resizedWidth, resizedHeigh)); }

protected:
    // Fields
    const shared_ptr<SingleImageTexture2f<T>> texture2f;
};

template <typename T>
class MIPMap : public TextureSi<T>
{
public:
    // Constructors
    MIPMap(const shared_ptr<SingleImageTexture2f<T>>& singleTexture)
    {
        singleTextures.push_back(singleTexture);
        while (singleTextures.back()->width > 1 && singleTextures.back()->heigh > 1)
            singleTextures.push_back(singleTextures.back()->Resized(singleTextures.back()->width / 2, singleTextures.back()->heigh / 2));
    }

    // Methods
    virtual T Evaluate(const SurfaceInteraction& evaluator) const override
    {
        const SurfaceInteraction& si = evaluator;
        // 计算 differential rays 与平面的交点 //! 注：si.ng 是否已单位化无关紧要
        Float tx(Normal3f::Dot(si.ng, si.p - si.rayONextPixelX) / Normal3f::Dot(si.ng, si.rayDNextPixelX)); //! 因为射线可能碰到摄像机背后的物体，这时候 t 就会是负数，但是不要紧，照样算就行
        Float ty(Normal3f::Dot(si.ng, si.p - si.rayONextPixelY) / Normal3f::Dot(si.ng, si.rayDNextPixelY));
        Point3f px(si.rayONextPixelX + tx * si.rayDNextPixelX);
        Point3f py(si.rayONextPixelY + ty * si.rayDNextPixelY);
        // 计算 d
        Float dudx = singleTextures[0]->width * Vector3f::Dot(px - si.p, si.sg) / si.sg.NormSquared();
        Float dvdx = singleTextures[0]->heigh * Vector3f::Dot(px - si.p, si.tg) / si.tg.NormSquared();
        Float dudy = singleTextures[0]->width * Vector3f::Dot(py - si.p, si.sg) / si.sg.NormSquared();
        Float dvdy = singleTextures[0]->heigh * Vector3f::Dot(py - si.p, si.tg) / si.tg.NormSquared();
        Float lx = std::sqrt(dudx * dudx + dvdx * dvdx);
        Float ly = std::sqrt(dudy * dudy + dvdy * dvdy);
        Float l = std::max(lx, ly);
        Float d = std::clamp(std::log2(l), Float(0), Float(singleTextures.size() - 1.0001)); //! 需要保证 floor(d) <= size() - 2
        int dI = int(d);
        Float dFrac = d - dI;
        //* Debug: MIPmap level ------------------------------------------------------------------------------ //
        // vector<Spectrum> dToColor;
        // dToColor.push_back(Spectrum(0, 0, 0));
        // for (size_t i = 1; i < singleTextures.size(); i += 3)
        // {
        //     dToColor.push_back(Spectrum(1, 0, 0));
        //     dToColor.push_back(Spectrum(0, 1, 0));
        //     dToColor.push_back(Spectrum(0, 0, 1));
        // }
        // return Spectrum::Lerp(dToColor[dI], dToColor[dI + 1], dFrac);
        //* Debug: MIPmap level ------------------------------------------------------------------------------ //
        return (1 - dFrac) * singleTextures[dI]->Evaluate(si.uv) + dFrac * singleTextures[dI + 1]->Evaluate(si.uv);
    }

protected:
    // Fields
    vector<shared_ptr<SingleImageTexture2f<T>>> singleTextures;
};

class BumpMap : public SingleImageTextureSi<Normal3f>
{
public:
    // Fields
    const BumpMapType type;

    // Constructors
    BumpMap(const string& imagePath, BumpMapType type);

    // Methods
    //! 返回值必为单位向量，虽然可能慢一点，但符合一贯性更重要
    Normal3f Evaluate(const SurfaceInteraction& evaluator) const override;
    //! 默认 si->ng 已单位化
    void Bump(SurfaceInteraction* si) const;
};

#endif