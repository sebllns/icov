#ifndef RENDERER_H
#define RENDERER_H

#include "omnidirectional_layer/interface/IRenderer.h"

class Renderer : public IRenderer
{
public:
  Renderer();
   void drawPixels(unsigned char *pixels, int width, int height);
};

#endif // RENDERER_H
