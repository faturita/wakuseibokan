/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* File for "Animation" lesson of the OpenGL tutorial on
 * www.videotutorialsrock.com
 */



#ifndef VEC3D_H_INCLUDED
#define VEC3D_H_INCLUDED

#include <iostream>

class Vec3d {
	private:
        double v[3];
	public:
        Vec3d();
        Vec3d(double x, double y, double z);
		
        double &operator[](int index);
        double operator[](int index) const;
		
        Vec3d operator*(double scale) const;
        Vec3d operator/(double scale) const;
        Vec3d operator+(const Vec3d &other) const;
        Vec3d operator-(const Vec3d &other) const;
        Vec3d operator-() const;
		
        const Vec3d &operator*=(double scale);
        const Vec3d &operator/=(double scale);
        const Vec3d &operator+=(const Vec3d &other);
        const Vec3d &operator-=(const Vec3d &other);
		
        double magnitude() const;
        double magnitudeSquared() const;
        Vec3d normalize() const;
        double dot(const Vec3d &other) const;
        Vec3d cross(const Vec3d &other) const;

        Vec3d rotateOnX(double alpha);
        Vec3d rotateOnY(double alpha);
        Vec3d rotateOnZ(double alpha);

        Vec3d rotateOn(Vec3d u, double alpha);
};

Vec3d operator*(double scale, const Vec3d &v);
std::ostream &operator<<(std::ostream &output, const Vec3d &v);










#endif
