#ifndef CS171_HW4_INCLUDE_BRDF_H_
#define CS171_HW4_INCLUDE_BRDF_H_
#include <core.h>
/**
 * Base class of BRDFs
 */
class BRDF {
 public:
  BRDF() = default;

  /**
   * Evaluate the BRDF, namely, return the BRDF value at the given interation
   */
  virtual vec3 eval(const Interaction &interact) = 0;
  /**
   * Compute the PDF of the given BRDF sample at the specified interaction
   * You may need to use the Interaction.wi and Interaction.wo
   */
  virtual Float pdf(const Interaction &interact) = 0;
  /**
   * Sample a direction according to the BRDF
   * Store the sampled direction in the given interaction
   * Also, the PDF of this sample should be returned
   */
  virtual Float sample(Interaction &interact) = 0;
  /**
   * Return if the BRDF is delta (specular or transmission)
   */
  [[nodiscard]] virtual bool isDelta() const = 0;
};

/**
 * Ideal diffusion BRDFs
 */

class IdealDiffusion : public BRDF {
 protected:
  vec3 reflectivity;
  float pdf_value;
 public:
  explicit IdealDiffusion(vec3 diff);
  vec3 eval(const Interaction &interact) override;
  Float pdf(const Interaction &interact) override;
  Float sample(Interaction &interact) override;
  [[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeIdealDiffusion(const vec3 &diff);

// optional
class IdealSpecular : public BRDF {
protected:
	vec3 reflectivity;
	float pdf_value;
public:
	explicit IdealSpecular(vec3 spec);
	vec3 eval(const Interaction& interact) override;
	Float pdf(const Interaction& interact) override;
	Float sample(Interaction& interact) override;
	[[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeIdealSpecular(const vec3& diff);

class IdealTransmission : public BRDF {
protected:
	vec3 reflectivity;
	float pdf_value;
	float ratio;//eta1 / eta2 ,in:eta1, out:eta2
public:
	explicit IdealTransmission(vec3 tran,float ratio);
	vec3 eval(const Interaction& interact) override;
	Float pdf(const Interaction& interact) override;
	Float sample(Interaction& interact) override;
	[[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeIdealTransmission(const vec3& diff,float ratio);

class GlossyMetal : public BRDF {
protected:
	vec3 reflectivity;
	float pdf_value;
public:
	explicit GlossyMetal(vec3 metal);
	vec3 eval(const Interaction& interact) override;
	Float pdf(const Interaction& interact) override;
	Float sample(Interaction& interact) override;
	[[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeGlossyMetal(const vec3& diff);
#endif  // CS171_HW4_INCLUDE_BRDF_H_
