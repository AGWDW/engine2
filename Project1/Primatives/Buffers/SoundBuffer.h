#pragma once
#include "../../Utils/General.h"
namespace Primative {
	namespace Buffers {
		class SoundBuffer {
		private:
			unsigned SBO; // sound buffer object
			std::string name;
			float bitDepth, sampleRate, dataSize;
		public:
			SoundBuffer();
			SoundBuffer(const char* soundData, Int len, Unsigned channel, Int sampleRate, Unsigned bitDepth);
			~SoundBuffer() = default;

			void cleanUp();

			Unsigned getSBO() const;
		};
	}
}
