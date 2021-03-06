#include "tp_image_utils/ColorMap.h"

#include <vector>
#include <sstream>
#include <cstring>
#include <atomic>

namespace tp_image_utils
{

//##################################################################################################
struct ColorMap::Private
{
  std::vector<TPPixel> data;
  size_t width;
  size_t height;

  std::atomic_int refCount{1};

  //################################################################################################
  void detach(ColorMap* q)
  {
    if(refCount==1)
      return;

    auto newSD = new Private();
    newSD->data = data;
    newSD->width = width;
    newSD->height = height;

    if(refCount.fetch_sub(1)==1)
      delete this;

    q->sd = newSD;
  }
};

//##################################################################################################
ColorMap::ColorMap(const ColorMap& other):
  sd(other.sd)
{
  sd->refCount++;
}

//##################################################################################################
ColorMap::ColorMap(size_t w, size_t h, const TPPixel* data):
  sd(new Private())
{  
  sd->width = w;
  sd->height = h;
  sd->data.resize(w*h, TPPixel());

  if(data)
    memcpy(sd->data.data(), data, sd->data.size()*sizeof(TPPixel));
}

//##################################################################################################
ColorMap::~ColorMap()
{
  if(sd->refCount.fetch_sub(1)==1)
    delete sd;
}

//##################################################################################################
ColorMap& ColorMap::operator=(const ColorMap& other)
{
  if(sd == other.sd)
    return *this;

  if(sd->refCount.fetch_sub(1)==1)
    delete sd;

  sd = other.sd;
  sd->refCount++;

  return *this;
}

//##################################################################################################
void ColorMap::fill(TPPixel value)
{
  sd->detach(this);
  std::fill(sd->data.begin(), sd->data.end(), value);
}

//##################################################################################################
const TPPixel* ColorMap::constData() const
{
  return sd->data.data();
}

//##################################################################################################
TPPixel* ColorMap::data()
{
  sd->detach(this);
  return sd->data.data();
}

//##################################################################################################
const std::vector<TPPixel>& ColorMap::constDataVector() const
{
  return sd->data;
}

//##################################################################################################
size_t ColorMap::width() const
{
  return sd->width;
}

//##################################################################################################
size_t ColorMap::height() const
{
  return sd->height;
}

//##################################################################################################
size_t ColorMap::size() const
{
  return sd->data.size();
}

//##################################################################################################
void ColorMap::setPixel(size_t x, size_t y, TPPixel value)
{
  sd->detach(this);
  if(x<sd->width && y<sd->height)
    sd->data[(y*sd->width) + x] = value;
}

//##################################################################################################
TPPixel ColorMap::pixel(size_t x, size_t y) const
{
  return (x<sd->width && y<sd->height)?sd->data.at((y*sd->width) + x):TPPixel();
}

//##################################################################################################
TPPixel& ColorMap::pixelRef(size_t x, size_t y)
{
  sd->detach(this);
  return sd->data.at((y*sd->width) + x);
}

//##################################################################################################
ColorMap ColorMap::subImage(size_t left, size_t top, size_t right, size_t bottom) const
{
  left   = tpBound(size_t(0), left,   sd->width -1);
  top    = tpBound(size_t(0), top,    sd->height-1);
  right  = tpBound(size_t(0), right,  sd->width );
  bottom = tpBound(size_t(0), bottom, sd->height);

  size_t width  = tpMax(size_t(1), right - left);
  size_t height = tpMax(size_t(1), bottom - top);

  ColorMap dst(width, height);

  for(size_t y=0; y<height; y++)
    for(size_t x=0; x<width; x++)
      dst.setPixel(x, y, pixel(x+left, y+top));

  return dst;
}

//##################################################################################################
ColorMap ColorMap::rotate90CW() const
{
  ColorMap dst(sd->height, sd->width);

  size_t sh = sd->height-1;

  for(size_t y=0; y<sd->height; y++)
    for(size_t x=0; x<sd->width; x++)
      dst.setPixel(sh-y, x, pixel(x, y));

  return dst;
}

//##################################################################################################
ColorMap ColorMap::rotate90CCW() const
{
  ColorMap dst(sd->height, sd->width);

  size_t sw = sd->width-1;

  for(size_t y=0; y<sd->height; y++)
    for(size_t x=0; x<sd->width; x++)
      dst.setPixel(y, sw-x, pixel(x, y));

  return dst;
}

//##################################################################################################
ColorMap ColorMap::flipped() const
{
  ColorMap dst(sd->width, sd->height);

  size_t sh = sd->height-1;

  for(size_t y=0; y<sd->height; y++)
    for(size_t x=0; x<sd->width; x++)
      dst.setPixel(x, sh-y, pixel(x, y));

  return dst;
}

//##################################################################################################
std::vector<TPPixel> ColorMap::extractRow(size_t y) const
{
  std::vector<TPPixel> result;
  if(sd->width>0 && sd->height>0 && y<sd->height)
  {
    result.resize(sd->width);
    const TPPixel* s = sd->data.data()+(y*sd->width);
    TPPixel* d = result.data();
    std::memcpy(d, s, sd->width);
  }
  return result;
}

//##################################################################################################
std::vector<TPPixel> ColorMap::extractColumn(size_t x) const
{
  std::vector<TPPixel> result;
  if(sd->width>0 && sd->height>0 && x<sd->width)
  {
    result.resize(sd->height);
    const TPPixel* s = sd->data.data()+x;
    TPPixel* d = result.data();
    TPPixel* dMax = d + sd->height;

    for(; d<dMax; d++, s+=sd->width)
      *d = *s;
  }
  return result;
}

//##################################################################################################
void ColorMap::setRow(size_t y, const std::vector<TPPixel>& values)
{
  sd->detach(this);
  if(sd->width>0 && sd->height>0 && y<sd->height && values.size() == sd->width)
  {
    const TPPixel* s = values.data();
    TPPixel* d = sd->data.data()+(y*sd->width);
    std::memcpy(d, s, sd->width);
  }
}

//##################################################################################################
void ColorMap::setColumn(size_t x, const std::vector<TPPixel>& values)
{
  sd->detach(this);
  if(sd->width>0 && sd->height>0 && x<sd->width && values.size() == sd->height)
  {
    TPPixel* d = sd->data.data()+x;
    const TPPixel* s = values.data();
    const TPPixel* sMax = s + sd->height;

    for(; s<sMax; s++, d+=sd->width)
      *d = *s;
  }
}

//##################################################################################################
void ColorMap::setSize(size_t width, size_t height)
{
  sd->detach(this);
  sd->width  = width;
  sd->height = height;
  sd->data.resize(width*height);
}


}
