#pragma once
#include "Componets.h"
namespace Component {
	class AudioReciever : public ComponetBase
	{
	private:

	public:
		AudioReciever();
		~AudioReciever() = default;
		void update(float deltaTime);
		void cleanUp();
		inline Type getType() const { return Type::AudioReciever; };
	};
}

