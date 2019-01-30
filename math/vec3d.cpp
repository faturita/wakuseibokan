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



#include <math.h>

#include "yamathutil.h"
#include "vec3d.h"

using namespace std;

Vec3d::Vec3d() {
	
}

Vec3d::Vec3d(double x, double y, double z) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

double &Vec3d::operator[](int index) {
	return v[index];
}

double Vec3d::operator[](int index) const {
	return v[index];
}

Vec3d Vec3d::operator*(double scale) const {
    return Vec3d(v[0] * scale, v[1] * scale, v[2] * scale);
}

Vec3d Vec3d::operator/(double scale) const {
    return Vec3d(v[0] / scale, v[1] / scale, v[2] / scale);
}

Vec3d Vec3d::operator+(const Vec3d &other) const {
    return Vec3d(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]);
}

Vec3d Vec3d::operator-(const Vec3d &other) const {
    return Vec3d(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]);
}

Vec3d Vec3d::operator-() const {
    return Vec3d(-v[0], -v[1], -v[2]);
}

const Vec3d &Vec3d::operator*=(double scale) {
	v[0] *= scale;
	v[1] *= scale;
	v[2] *= scale;
	return *this;
}

const Vec3d &Vec3d::operator/=(double scale) {
	v[0] /= scale;
	v[1] /= scale;
	v[2] /= scale;
	return *this;
}

const Vec3d &Vec3d::operator+=(const Vec3d &other) {
	v[0] += other.v[0];
	v[1] += other.v[1];
	v[2] += other.v[2];
	return *this;
}

const Vec3d &Vec3d::operator-=(const Vec3d &other) {
	v[0] -= other.v[0];
	v[1] -= other.v[1];
	v[2] -= other.v[2];
	return *this;
}

double Vec3d::magnitude() const {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

double Vec3d::magnitudeSquared() const {
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

Vec3d Vec3d::normalize() const {
    double m = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return Vec3d(v[0] / m, v[1] / m, v[2] / m);
}

double Vec3d::dot(const Vec3d &other) const {
	return v[0] * other.v[0] + v[1] * other.v[1] + v[2] * other.v[2];
}

Vec3d Vec3d::cross(const Vec3d &other) const {
    return Vec3d(v[1] * other.v[2] - v[2] * other.v[1],
				 v[2] * other.v[0] - v[0] * other.v[2],
				 v[0] * other.v[1] - v[1] * other.v[0]);
}

Vec3d operator*(double scale, const Vec3d &v) {
	return v * scale;
}

ostream &operator<<(ostream &output, const Vec3d &v) {
	cout << '(' << v[0] << ", " << v[1] << ", " << v[2] << ')';
	return output;
}


Vec3d Vec3d::rotateOnX(double alpha)
{
    //double alpha = alphagrad * PI / 180.0f;

    // x y z
    double x = (double)( v[0]                    + 0                       + 0             );
    double y = (double)( 0                       + v[1]*cos(alpha)         + v[2]*sin(alpha) );
    double z = (double)( 0                       - v[1]*sin(alpha)         + v[2]*cos(alpha) );

    Vec3d Vec3d(x,y,z);

    return Vec3d;
}
Vec3d Vec3d::rotateOnY(double alpha)
{
    //double alpha = alphagrad * PI / 180.0f;

    // x y z
    double x = (double)( v[0]*cos(alpha)         + 0                       - v[2]*sin(alpha) );
    double y = (double)( 0                       + v[1]                    + 0               );
    double z = (double)( v[0]*sin(alpha)         + 0                       + v[2]*cos(alpha) );

    Vec3d Vec3d(x,y,z);

    return Vec3d;
}
Vec3d Vec3d::rotateOnZ(double alpha)
{
    //double alpha = alphagrad * PI / 180.0f;

    // x y z
    double x = (double)( v[0]*cos(alpha)         + v[1]*sin(alpha)         + 0               );
    double y = (double)( -v[0]*sin(alpha)        + v[1]*cos(alpha)         + 0               );
    double z = (double)( 0                       + 0                       + v[2]            );

    Vec3d Vec3d(x,y,z);

    return Vec3d;
}
Vec3d Vec3d::rotateOn(Vec3d u, double alpha)
{
    double c[3];

    double R[3][3];

    double ca = cos(alpha);
    double sa = sin(alpha);

    u = u.normalize();

    double ux = u[0];
    double uy = u[1];
    double uz = u[2];

    // x y z
    R[0][0] = ca + ux*ux*(1 - ca);
    R[0][1] = ux*uy*(1-ca) - uz * sa;
    R[0][2] = ux*ux*(1-ca)+uy*sa;

    R[1][0] = uy*ux*(1-ca)+uz*sa;
    R[1][1] = ca+uy*uy*(1-ca);
    R[1][2] = uy*uz*(1-ca)-ux*sa;

    R[2][0] = uz*ux*(1-ca)-uy*sa;
    R[2][1] = uz*uy*(1-ca)+ux*sa;
    R[2][2] = ca+uz*uz+(1-ca);


    for (int i=0;i<3;i++)
    {
         c[i]=0;
    }

    for (int i=0;i<3;i++)
    {
        for (int j=0;j<3;j++)
        {
            c[i]+=(v[j]*R[i][j]);
        }
    }

    Vec3d Vec3d(c[0],c[1],c[2]);
    return Vec3d;
}






