#include "Rendering.h"

#include "Shading/Manager.h"
#include "../Componets/Componets.h"
#include "../GameObject/GameObject.h"

Render::RenderMesh::RenderMesh() : buffers(), material(), Component::ComponetBase()
{
	/*Materials::Forward* fwd = DBG_NEW Materials::Forward();
	fwd->getDiffuse()(1); // set to the texture id
	// fwd.getDiffuse()({ 1, 0, 0 });
	fwd->getSpecular()(2);
	fwd->getNormals()(3);
	fwd->shininess = 32;
	this->material = fwd;

	Materials::PBR* prb = DBG_NEW Materials::PBR();
	prb->getAlbedo()(1);
	prb->getAO()({ 1, 1, 1 });
	prb->getNormal()(3);
	prb->getMetalic()(2); // { 1.0f/7.0f, 0, 0 }
	prb->getRoughness()({ 0.025, 0.1, 0.1 });
	this->material = prb;*/
}


void Render::RenderMesh::update()
{
	glm::mat4 m(1);
	if(parent)
		m = Component::ComponetBase::parent->getTransform()->getModel();
	bool succ = Shading::Manager::setValue("model", m);
	if (material) {
		if (this->material->getType() == Materials::Type::Forward)
			succ = Shading::Manager::setValue("material", *dynamic_cast<Materials::Forward*>(this->material));
		else
			succ = Shading::Manager::setValue("material", *dynamic_cast<Materials::PBR*>(this->material));
		this->material->activateTextures();
	}
	for (const unsigned& index : buffers) {
		const Primative::VertexBuffer* buffer = ResourceLoader::getBuffer(index);
		buffer->bind();
		buffer->draw();
		buffer->unBind();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
void Render::RenderMesh::addBuffers(std::vector<unsigned>& buffers, const GLenum draw_type)
{
	this->buffers = buffers;
	/*for (unsigned& index : buffers) {
		auto buffer = ResourceLoader::getBuffer(index);
		buffer->setDrawType(draw_type);
	}*/
}
void Render::RenderMesh::cleanUp() {
	parent = nullptr;
	for (auto itt = buffers.begin(); itt != buffers.end();) {
		itt = buffers.erase(itt);
	}
	if(material)
		material->cleanUp();
	// delete material;
	material = nullptr;
}

