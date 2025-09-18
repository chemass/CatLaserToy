#include "Geometry.h"

namespace Geometry {
    Point constrainToBounds(const Point& point, int xMin, int xMax, int yMin, int yMax) {
        Point result;
        result.x = (point.x < xMin) ? xMin : (point.x > xMax) ? xMax : point.x;
        result.y = (point.y < yMin) ? yMin : (point.y > yMax) ? yMax : point.y;
        return result;
    }
    
    Point lerp(const Point& start, const Point& end, float t) {
        Point result;
        result.x = start.x + (int)((end.x - start.x) * t);
        result.y = start.y + (int)((end.y - start.y) * t);
        return result;
    }
    
    bool isWithinBounds(const Point& point, int xMin, int xMax, int yMin, int yMax) {
        return point.x >= xMin && point.x <= xMax && point.y >= yMin && point.y <= yMax;
    }
    
    Point mapToRectangle(const NormalizedPoint& normalized, const Rectangle& rect) {
        Point result;
        result.x = rect.topLeft.x + (int)(normalized.u * rect.width());
        result.y = rect.topLeft.y + (int)(normalized.v * rect.height());
        return result;
    }
    
    Point mapToQuadrilateral(const NormalizedPoint& normalized, const Quadrilateral& quad) {
        return bilinearInterpolate(normalized.u, normalized.v, quad);
    }
    
    NormalizedPoint mapFromRectangle(const Point& point, const Rectangle& rect) {
        NormalizedPoint result;
        result.u = (float)(point.x - rect.topLeft.x) / (float)rect.width();
        result.v = (float)(point.y - rect.topLeft.y) / (float)rect.height();
        result.clamp();
        return result;
    }
    
    Point bilinearInterpolate(float u, float v, const Quadrilateral& quad) {
        float invU = 1.0f - u;
        float invV = 1.0f - v;
        
        Point result;
        
        // Bilinear interpolation formula
        // corners[0]=TL, [1]=TR, [2]=BR, [3]=BL
        result.x = (int)(quad.corners[0].x * invU * invV +
                         quad.corners[1].x * u * invV +
                         quad.corners[2].x * u * v +
                         quad.corners[3].x * invU * v);
                    
        result.y = (int)(quad.corners[0].y * invU * invV +
                         quad.corners[1].y * u * invV +
                         quad.corners[2].y * u * v +
                         quad.corners[3].y * invU * v);
        
        return result;
    }
    
    float distance(const Point& a, const Point& b) {
        float dx = (float)(b.x - a.x);
        float dy = (float)(b.y - a.y);
        return sqrt(dx * dx + dy * dy);
    }
    
    float angle(const Point& from, const Point& to) {
        float dx = (float)(to.x - from.x);
        float dy = (float)(to.y - from.y);
        return atan2(dy, dx);
    }
    
    NormalizedPoint polarToCartesian(float radius, float angle, const NormalizedPoint& center) {
        NormalizedPoint result;
        result.u = center.u + radius * cos(angle);
        result.v = center.v + radius * sin(angle);
        result.clamp();
        return result;
    }
    
    void rotatePoint(NormalizedPoint& point, float angle, const NormalizedPoint& pivot) {
        // Translate to origin
        float x = point.u - pivot.u;
        float y = point.v - pivot.v;
        
        // Rotate
        float cosA = cos(angle);
        float sinA = sin(angle);
        float newX = x * cosA - y * sinA;
        float newY = x * sinA + y * cosA;
        
        // Translate back
        point.u = newX + pivot.u;
        point.v = newY + pivot.v;
        point.clamp();
    }
}