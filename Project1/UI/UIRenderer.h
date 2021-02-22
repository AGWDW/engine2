#pragma once
#include "../Primatives/Buffers.h"
#include "Panes/Pane.h"
#include "Page.h"
namespace  Primative {
	class VertexBuffer;
	class StaticBuffer;
}
namespace UI {
	class UIRenderer
	{
		static unsigned shaderId;
		static Primative::VertexBuffer quadBuffer;
		static Primative::StaticBuffer uiBuffer;
	public:
		static void init(const unsigned& shaderId, const glm::vec2& screenDim);
		static void render(const Pane* pane);
		static void render(const Page* pane);
		static void render(const Element* element);
		static void cleanUp();
	};
};
