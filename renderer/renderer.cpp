#include "renderer.h"

Renderer::Renderer()
{

}

void Renderer::drawPixels(unsigned char *pixels, int width, int height)
{
  std::cout << "Renderer::drawPixels()" << std::endl;
  m_request -> print();
}
