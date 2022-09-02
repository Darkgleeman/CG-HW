#ifndef COEFFICIENT_SPECTRUM_H
#define COEFFICIENT_SPECTRUM_H

#include <utils.h>

template <int nSpectrumSamples>
class CoefficientSpectrum
{
public:
    // Constructor
    CoefficientSpectrum(Float value = 0)
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            c[i] = value;
    }
    CoefficientSpectrum(Float* values)
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            c[i] = values[i];
    }
    CoefficientSpectrum(const std::initializer_list<Float>& values)
    {
        int i = 0;
        for (auto it = values.begin(); it != values.end(); ++it)
            c[i++] = *it;
        Assert(i == nSpectrumSamples, 
               "CoefficientSpectrum::CoefficientSpectrum(): Initializer_list length not matched");
    }

    // Methods
    static CoefficientSpectrum Lerp(const CoefficientSpectrum& a, const CoefficientSpectrum& b, Float t)
    {
        return a + (b - a) * t;
    }

    static CoefficientSpectrum Lerp(const CoefficientSpectrum& v00, const CoefficientSpectrum& v01, const CoefficientSpectrum& v10, const CoefficientSpectrum& v11, Float u, Float v)
    {
        Float uv = u * v;
        Float u_sub_uv = u - uv;
        return (1 - u_sub_uv - v) * v00 + u_sub_uv * v01 + (v - uv) * v10 + uv * v11;
    }
    static CoefficientSpectrum Max(const CoefficientSpectrum& a, const CoefficientSpectrum& b)
    {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret[i] = std::max(a[i], b[i]);
        return ret;
    }

    CoefficientSpectrum Clamp(Float inf = 0, Float sup = Infinity) const
    {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] = std::clamp(c[i], inf, sup);
        return ret;
    }
    CoefficientSpectrum Sqrted() const
    {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] = std::sqrt(c[i]);
        return ret;
    }
    bool IsBlack() const
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            if (c[i] != 0) return false;
        return true;
    }
    bool HasNans() const
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            if (std::isnan(c[i])) return true;
        return false;
    }
    CoefficientSpectrum Absed() const
    {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] = std::abs(c[i]);
        return ret;
    }
    Float Average() const
    {
        const Float nSpectrumSamplesInv = Float(1) / Float(nSpectrumSamples);
        Float sum = 0;
        for (int i = 0; i < nSpectrumSamples; ++i)
            sum += c[i];
        return sum * nSpectrumSamplesInv;
    }
    Float Maximum() const
    {
        Float c_max = c[0];
        for (int i = 1; i < nSpectrumSamples; ++i)
            c_max = std::max(c_max, c[i]);
        return c_max;
    }
    Float Minimum() const
    {
        Float c_max = c[0];
        for (int i = 1; i < nSpectrumSamples; ++i)
            c_max = std::min(c_max, c[i]);
        return c_max;
    }
    int MaximumExtent() const
    {
        Float c_max = c[0];
        int i_max = 0;
        for (int i = 1; i < nSpectrumSamples; ++i)
            if (c[i] > c_max)
            {
                c_max = c[i];
                i_max = i;
            }
        return i_max;
    }
    CoefficientSpectrum Exp() const
    {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] = std::exp(c[i]);
        return ret;
    }

    // Operators
    friend std::ostream& operator<<(std::ostream& out, const CoefficientSpectrum<nSpectrumSamples> spectrum)
    {
        out << '(';
        for (int i = 0; i < nSpectrumSamples - 1; ++i)
            out << spectrum[i] << ',' << ' ';
        out << spectrum[nSpectrumSamples - 1] << ')';
        return out;
    }
    Float operator[] (int i) const { return c[i]; }
    Float& operator[] (int i) { return c[i]; }
    CoefficientSpectrum operator+(const CoefficientSpectrum& that) const
    {
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] += that.c[i];
        return ret;
    }
    CoefficientSpectrum& operator+=(const CoefficientSpectrum& that)
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            c[i] += that.c[i];
        return *this;
    }
    CoefficientSpectrum operator-() const
    {
        CoefficientSpectrum ret;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] = -c[i];
        return ret;
    }
    CoefficientSpectrum operator-(const CoefficientSpectrum& that) const
    {
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] -= that.c[i];
        return ret;
    }
    CoefficientSpectrum& operator-=(const CoefficientSpectrum& that)
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            c[i] -= that.c[i];
        return *this;
    }
    CoefficientSpectrum operator*(const CoefficientSpectrum& that) const
    {
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] *= that.c[i];
        return ret;
    }
    CoefficientSpectrum& operator*=(const CoefficientSpectrum& that)
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            c[i] *= that.c[i];
        return *this;
    }
    CoefficientSpectrum operator*(Float scalar) const
    {
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] *= scalar;
        return ret;
    }
    CoefficientSpectrum& operator*=(Float scalar)
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            c[i] *= scalar;
        return *this;
    }
    friend inline CoefficientSpectrum operator*(Float scalar, const CoefficientSpectrum& spectrum)
    {
        return spectrum * scalar;
    }
    CoefficientSpectrum operator/(const CoefficientSpectrum& that) const
    {
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] /= that.c[i];
        return ret;
    }
    CoefficientSpectrum& operator/=(const CoefficientSpectrum& that)
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            c[i] /= that.c[i];
        return *this;
    }
    CoefficientSpectrum operator/(Float divide) const
    {
        CoefficientSpectrum ret = *this;
        for (int i = 0; i < nSpectrumSamples; ++i)
            ret.c[i] /= divide;
        return ret;
    }
    CoefficientSpectrum& operator/=(Float divide)
    {
        for (int i = 0; i < nSpectrumSamples; ++i)
            c[i] /= divide;
        return *this;
    }
protected:
    // Fields
    Float c[nSpectrumSamples];
};

#endif