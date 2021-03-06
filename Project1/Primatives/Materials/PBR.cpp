#include "PBR.h"
#include "MatItemBase.h"

Materials::PBR::PBR() : MaterialBase(), albedo(nullptr), normal(nullptr), metalic(nullptr), roughness(nullptr), ao(nullptr), emission(nullptr)
{
}

Materials::PBR::PBR(MatItemBase<glm::vec4>* albedo, MatItemBase<glm::vec3>* normal, MatItemBase<glm::vec3>* emission,
	MatItemBase<float>* metalic, MatItemBase<float>* roughness, MatItemBase<float>* ao) : PBR()
{
	this->albedo = albedo;
	this->normal = normal;
	this->emission = emission;
	this->metalic = metalic;
	this->roughness = roughness;
	this->ao = ao;
}

void Materials::PBR::activateTextures(Int startUnit) const
{
	unsigned unit = startUnit;

	albedo->tryBindTexture(unit);
	normal->tryBindTexture(unit);
	emission->tryBindTexture(unit);
	metalic->tryBindTexture(unit);
	roughness->tryBindTexture(unit);
	ao->tryBindTexture(unit);
}

void Materials::PBR::cleanUp()
{
	albedo->cleanUp();
	normal->cleanUp();
	emission->cleanUp();
	metalic->cleanUp();
	roughness->cleanUp();
	ao->cleanUp();
}

void Materials::PBR::update(float deltaTime)
{
	albedo->update(deltaTime);
	normal->update(deltaTime);
	emission->update(deltaTime);
	metalic->update(deltaTime);
	roughness->update(deltaTime);
	ao->update(deltaTime);
}

const Materials::MatItemBase<glm::vec4>* Materials::PBR::getAlbedo() const
{
	return albedo;
}

const Materials::MatItemBase<glm::vec3>* Materials::PBR::getNormal() const
{
	return normal;
}

const Materials::MatItemBase<glm::vec3>* Materials::PBR::getEmission() const
{
	return emission;
}

const Materials::MatItemBase<float>* Materials::PBR::getMetalic() const
{
	return metalic;
}

const Materials::MatItemBase<float>* Materials::PBR::getRoughness() const
{
	return roughness;
}

const Materials::MatItemBase<float>* Materials::PBR::getAO() const
{
	return ao;
}

void Materials::PBR::setAlbedo(MatItemBase<glm::vec4>* albedo)
{
	this->albedo = albedo;
}

void Materials::PBR::setNormal(MatItemBase<glm::vec3>* normal)
{
	this->normal = normal;
}

void Materials::PBR::setEmission(MatItemBase<glm::vec3>* emission)
{
	this->emission = emission;
}

void Materials::PBR::setMetalic(MatItemBase<float>* metalic)
{
	this->metalic = metalic;
}

void Materials::PBR::setRoughness(MatItemBase<float>* roughness)
{
	this->roughness = roughness;
}

void Materials::PBR::setAO(MatItemBase<float>* ao)
{
	this->ao = ao;
}

void Materials::PBR::setRepeatValue(Float mul)
{
	albedo->setRepeatValue(mul);
	normal->setRepeatValue(mul);
	emission->setRepeatValue(mul);
	metalic->setRepeatValue(mul);
	roughness->setRepeatValue(mul);
	ao->setRepeatValue(mul);
}
