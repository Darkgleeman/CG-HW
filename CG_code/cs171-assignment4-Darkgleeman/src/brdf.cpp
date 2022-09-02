#include <brdf.h>
#include <utils.h>
#include <interaction.h>

/**
 * IdealDiffusion class
 */

IdealDiffusion::IdealDiffusion(vec3 diff) : reflectivity(diff),pdf_value(0) {}

vec3 IdealDiffusion::eval(const Interaction &interact) {
	return reflectivity *INV_PI;
}

Float IdealDiffusion::pdf(const Interaction &interact) {
	return this->pdf_value;
}

Float IdealDiffusion::sample(Interaction &interact) {
	float eta1 = unif(0.0001, 0.9999, 1)[0];
	float eta2 = unif(0.0001, 0.9999, 1)[0];
	float r = sqrt(eta1);
	float theta = 2 * PI * eta2;
	vec3 local_wo = vec3(r * cos(theta), r* sin(theta), sqrt(1 - eta1));
	local_wo.normalize();
	interact.wo = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f(0,0,1), interact.normal).toRotationMatrix()*local_wo;
	interact.wo.normalize();
	this->pdf_value = sqrt(1 - eta1) / PI;
	return this->pdf_value;

}

bool IdealDiffusion::isDelta() const { return false; }

std::shared_ptr<BRDF> makeIdealDiffusion(const vec3 &diff) {
  return std::make_shared<IdealDiffusion>(diff);
}


IdealSpecular::IdealSpecular(vec3 spec) : reflectivity(spec), pdf_value(0) {}

vec3 IdealSpecular::eval(const Interaction& interact) {
	return reflectivity;
}

Float IdealSpecular::pdf(const Interaction& interact) {
	return 1.0f;
}

Float IdealSpecular::sample(Interaction& interact) {
	interact.wo = (-interact.wi + 2 * interact.normal.dot(interact.wi) * interact.normal).normalized();
	return 1.0f;
}

bool IdealSpecular::isDelta() const { return true; }

std::shared_ptr<BRDF> makeIdealSpecular(const vec3& diff) {
	return std::make_shared<IdealSpecular>(diff);
}

IdealTransmission::IdealTransmission(vec3 diff,float ratio_value) : reflectivity(diff), pdf_value(0),ratio(ratio_value) {}

vec3 IdealTransmission::eval(const Interaction& interact) {
	return reflectivity;
}

Float IdealTransmission::pdf(const Interaction& interact) {
	vec3 in = -interact.wi;
	vec3 n = interact.normal;
	float temp_ratio = ratio;
	float cos_theta_i = in.dot(interact.normal);
	if (cos_theta_i > 0)
	{
		temp_ratio = 1 / temp_ratio;
		n = -n; //如果in和normal方向一致的话，则因为假设 normal朝向为空气，所以此时是从介质射向空气
	}
	else cos_theta_i = -cos_theta_i;
	float sin_theta_t_2 = temp_ratio * temp_ratio * (1 - cos_theta_i * cos_theta_i);
	if (sin_theta_t_2 > 1) return 0.0f;
	return 1.0f;		
}



Float IdealTransmission::sample(Interaction& interact) {
	float eta = unif(0.0001, 0.9999, 1)[0];
	if (eta > 0.3)
	{	
		vec3 in = -interact.wi;  
		vec3 n = interact.normal;
		float temp_ratio = ratio;
		float cos_theta_i = in.dot(interact.normal);
		if (cos_theta_i > 0)
		{
			temp_ratio = 1 / temp_ratio;
			n = -n; //如果in和normal方向一致的话，则因为假设 normal朝向为空气，所以此时是从介质射向空气
		}
		else cos_theta_i = -cos_theta_i;
		float sin_theta_t_2 = temp_ratio * temp_ratio * (1 - cos_theta_i * cos_theta_i);
		if(sin_theta_t_2 > 1)  interact.wo = Eigen::Vector3f(0, 0, 0);
		else  interact.wo = (temp_ratio * in + (temp_ratio * cos_theta_i - sqrt(1 - sin_theta_t_2)) * n).normalized();
	}
	else
	{
		interact.wo = (-interact.wi + 2 * interact.normal.dot(interact.wi) * interact.normal).normalized();
	}
	return 1.0f;
}

bool IdealTransmission::isDelta() const { return false; }

std::shared_ptr<BRDF> makeIdealTransmission(const vec3& diff,float ratio) {
	return std::make_shared<IdealTransmission>(diff,ratio);
}

GlossyMetal::GlossyMetal(vec3 diff) : reflectivity(diff), pdf_value(0) {}

vec3 GlossyMetal::eval(const Interaction& interact) {
	return 0.1*reflectivity;
}

Float GlossyMetal::pdf(const Interaction& interact) {
	return this->pdf_value;
}

Float GlossyMetal::sample(Interaction& interact) {
	vec3 axis = (-interact.wi + 2 * interact.normal.dot(interact.wi) * interact.normal).normalized();
	float eta1 = unif(0.0001, 0.0, 1)[0];
	float eta2 = unif(0.0001, 0.9999, 1)[0];
	float r = sqrt(eta1);
	float theta = 2 * PI * eta2;
	vec3 local_wo = vec3(r * cos(theta), r * sin(theta), sqrt(1 - eta1));
	local_wo.normalize();
	interact.wo = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f(0, 0, 1), axis).toRotationMatrix() * local_wo;
	interact.wo.normalize();
	this->pdf_value = sqrt(1 - eta1) / PI;
	return this->pdf_value;

}

bool GlossyMetal::isDelta() const { return true; }

std::shared_ptr<BRDF> makeGlossyMetal(const vec3& diff) {
	return std::make_shared<GlossyMetal>(diff);
}
