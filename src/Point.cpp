#include "tp_image_utils/Point.h"

#include <math.h>

namespace tp_image_utils
{

//##################################################################################################
std::string pointTypeToString(PointType pointType)
{
  switch (pointType) {
  case PointTypeGeneral:    return "General";
  case PointTypeRectCorner: return "RectCorner";
  case PointTypeRectSide:   return "RectSide";
  }

  return "General";
}

//##################################################################################################
PointType pointTypeFromString(const std::string &data)
{
  if(data=="RectCorner")
    return PointTypeRectCorner;
  if(data=="RectSide")
    return PointTypeRectSide;

  return PointTypeGeneral;
}

//##################################################################################################
Point::Point():
  type(PointTypeRectSide),
  x(0.0f),
  y(0.0f)
{

}

//##################################################################################################
Point::Point(float x_, float y_):
  type(PointTypeRectSide),
  x(x_),
  y(y_)
{

}

//##################################################################################################
Point::Point(PointType type_, float x_, float y_):
  type(type_),
  x(x_),
  y(y_)
{

}

//##################################################################################################
float Point::length()const
{
  return sqrt((x*x) + (y*y));
}

//##################################################################################################
bool Point::normalize()
{
  float l = length();

  if(l==0.0f)
    return false;

  x/=l;
  y/=l;

  return true;
}

//##################################################################################################
void Point::rotate(float radians)
{
  float rx = x*cos(radians) - y*sin(radians);
  y = y*cos(radians) + x*sin(radians);
  x = rx;
}

//##################################################################################################
void Point::translate(const Point& vector)
{
  x+=vector.x;
  y+=vector.y;
}

//##################################################################################################
float Point::dot(const Point& p1, const Point& p2)
{
  return (p1.x*p2.x) + (p1.y*p2.y);
}

//##################################################################################################
bool Point::positionEquals(const Point& other) const
{
  return int(x) == int(other.x) && int(y) == int(other.x);
}

//##################################################################################################
Point deserializePoint(const std::string& data)
{
  std::vector<std::string> p;
  tpSplit(p, data, ',', tp_utils::SplitBehavior::SkipEmptyParts);

  try
  {
    if(p.size()==3)
      return Point(pointTypeFromString(p[0]), std::stof(p[1]), std::stof(p[2]));
  }
  catch(...)
  {

  }

  return Point();
}

//##################################################################################################
std::string serializePoint(const Point& point)
{
  return pointTypeToString(point.type) + ',' + std::to_string(point.x) + ',' + std::to_string(point.y);
}

//##################################################################################################
Line deserializeLine(const std::string& data)
{
  Line result;

  std::vector<std::string> lines;
  tpSplit(lines, data, '|', tp_utils::SplitBehavior::SkipEmptyParts);

  for(const std::string& line : lines)
  {
    std::vector<std::string> parts;
    tpSplit(parts, line, ',', tp_utils::SplitBehavior::SkipEmptyParts);
    if(parts.size()==3)
      result.push_back(Point(pointTypeFromString(parts[0]), std::stof(parts[1]), std::stof(parts[2])));
  }

  return result;
}

//##################################################################################################
std::string serializeLine(const Line& line)
{
  std::string result;

  for(const Point& point : line)
  {
    if(!result.empty())
      result.push_back('|');
    result.append(serializePoint(point));
  }

  return result;
}

//##################################################################################################
LineCollection deserializeLineCollection(const nlohmann::json& j)
{
  LineCollection result;

  for(const nlohmann::json& line : j)
    result.push_back(deserializeLine(line));

  return result;
}

//##################################################################################################
nlohmann::json serializeLineCollection(const LineCollection& lineCollection)
{
  nlohmann::json j = nlohmann::json::array();
  for(const Line& line : lineCollection)
    j.push_back(serializeLine(line));
  return j;
}

//##################################################################################################
std::vector<LineCollection> deserializeLineCollections(const nlohmann::json& j)
{
  //const std::vector<std::string>& data
  std::vector<LineCollection> result;
  for(const nlohmann::json& lineCollection : j)
    result.push_back(deserializeLineCollection(lineCollection));
  return result;
}

//##################################################################################################
nlohmann::json serializeLineCollections(const std::vector<LineCollection>& lineCollections)
{
  std::vector<std::string> result;
  for(const LineCollection& lineCollection : lineCollections)
    result.push_back(serializeLineCollection(lineCollection));
  return result;
}


//##################################################################################################
void rotate(Line& line, float radians)
{
  for(auto& i : line)
    i.rotate(radians);
}

//##################################################################################################
void rotate(LineCollection& lines, float radians)
{
  for(auto& i : lines)
    rotate(i, radians);
}

//##################################################################################################
void translate(Line& line, const Point& vector)
{
  for(auto& i : line)
    i.translate(vector);
}

//##################################################################################################
void translate(LineCollection& lines, const Point& vector)
{
  for(auto& i : lines)
    translate(i, vector);
}

//##################################################################################################
Point meanPoint(const LineCollection& lines)
{
  int c=0;
  Point t;

  for(const Line& line : lines)
  {
    for(const Point& point : line)
    {
      c++;
      t.translate(point);
    }
  }

  t.x = t.x / float(c);
  t.y = t.y / float(c);

  return t;
}

}
