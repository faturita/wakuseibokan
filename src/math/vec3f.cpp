/* ============================================================================
**
** 3D Vector class - Wakuseiboukan - 16/01/2011
**
** Originally based on OpenGL tutorial from www.videotutorialsrock.com
**
** For personal, educationnal, and research purpose only, this software is
** provided under the Gnu GPL (V.3) license. To use this software in
** commercial application, please contact the author.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License V.3 for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** ========================================================================= */


#include <math.h>

#include "yamathutil.h"
#include "vec3f.h"
#include <assert.h>

using namespace std;

Vec3f::Vec3f() {
	
}

Vec3f::Vec3f(float x, float y, float z) {
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

float &Vec3f::operator[](int index) {
	return v[index];
}

float Vec3f::operator[](int index) const {
	return v[index];
}

Vec3f Vec3f::operator*(float scale) const {
	return Vec3f(v[0] * scale, v[1] * scale, v[2] * scale);
}

Vec3f Vec3f::operator/(float scale) const {
	return Vec3f(v[0] / scale, v[1] / scale, v[2] / scale);
}

Vec3f Vec3f::operator+(const Vec3f &other) const {
	return Vec3f(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]);
}

Vec3f Vec3f::operator-(const Vec3f &other) const {
	return Vec3f(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]);
}

Vec3f Vec3f::operator-() const {
	return Vec3f(-v[0], -v[1], -v[2]);
}

const Vec3f &Vec3f::operator*=(float scale) {
	v[0] *= scale;
	v[1] *= scale;
	v[2] *= scale;
	return *this;
}

const Vec3f &Vec3f::operator/=(float scale) {
	v[0] /= scale;
	v[1] /= scale;
	v[2] /= scale;
	return *this;
}

const Vec3f &Vec3f::operator+=(const Vec3f &other) {
	v[0] += other.v[0];
	v[1] += other.v[1];
	v[2] += other.v[2];
	return *this;
}

const Vec3f &Vec3f::operator-=(const Vec3f &other) {
	v[0] -= other.v[0];
	v[1] -= other.v[1];
	v[2] -= other.v[2];
	return *this;
}

bool Vec3f::isEquals(const Vec3f other)
{
    if (v[0] == other[0] && v[1] == other[1] && v[2] == other[2])
        return true;
    else
        return false;
}

bool Vec3f::isZero()
{
    return isEquals(Vec3f(0,0,0));
}

float Vec3f::magnitude() const {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float Vec3f::magnitudeSquared() const {
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

Vec3f Vec3f::normalize() const {
	float m = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	return Vec3f(v[0] / m, v[1] / m, v[2] / m);
}

float Vec3f::dot(const Vec3f &other) const {
	return v[0] * other.v[0] + v[1] * other.v[1] + v[2] * other.v[2];
}

Vec3f Vec3f::cross(const Vec3f &other) const {
	return Vec3f(v[1] * other.v[2] - v[2] * other.v[1],
				 v[2] * other.v[0] - v[0] * other.v[2],
				 v[0] * other.v[1] - v[1] * other.v[0]);
}

Vec3f operator*(float scale, const Vec3f &v) {
	return v * scale;
}

ostream &operator<<(ostream &output, const Vec3f &v) {
	cout << '(' << v[0] << v[1] << ", " << v[2] << ')';
	return output;
}

istream &operator>>(istream &input, Vec3f &v) {
    input >> v;
    return input;
}

char* Vec3f::toString(char *dat) const {
    sprintf(dat,"(%10.5f,%10.5f,%10.5f)", v[0],v[1],v[2]);
    return dat;
}


Vec3f Vec3f::rotateOnX(float alpha)
{
    //float alpha = alphagrad * PI / 180.0f;

    // x y z
    float x = (float)( v[0]                    + 0                       + 0             );
    float y = (float)( 0                       + v[1]*cos(alpha)         + v[2]*sin(alpha) );
    float z = (float)( 0                       - v[1]*sin(alpha)         + v[2]*cos(alpha) );

    Vec3f vec3f(x,y,z);

    return vec3f;
}
Vec3f Vec3f::rotateOnY(float alpha)
{
    //float alpha = alphagrad * PI / 180.0f;

    // x y z
    float x = (float)( v[0]*cos(alpha)         + 0                       - v[2]*sin(alpha) );
    float y = (float)( 0                       + v[1]                    + 0               );
    float z = (float)( v[0]*sin(alpha)         + 0                       + v[2]*cos(alpha) );

    Vec3f vec3f(x,y,z);

    return vec3f;
}
Vec3f Vec3f::rotateOnZ(float alpha)
{
    //float alpha = alphagrad * PI / 180.0f;

    // x y z
    float x = (float)( v[0]*cos(alpha)         + v[1]*sin(alpha)         + 0               );
    float y = (float)( -v[0]*sin(alpha)        + v[1]*cos(alpha)         + 0               );
    float z = (float)( 0                       + 0                       + v[2]            );

    Vec3f vec3f(x,y,z);

    return vec3f;
}
Vec3f Vec3f::rotateOn(Vec3f u, float alpha)
{
    float c[3];

    float R[3][3];

    float ca = cos(alpha);
    float sa = sin(alpha);

    u = u.normalize();

    float ux = u[0];
    float uy = u[1];
    float uz = u[2];

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

    Vec3f vec3f(c[0],c[1],c[2]);
    return vec3f;
}

Vec3f Vec3f::rotateTo(Vec3f Up, Vec3f nd)
{
    nd = nd.normalize();
    Up = Up.normalize();

    Vec3f rot = Up.cross(nd);

    float a = _acos(  Up.dot(nd)  );

    Vec3f trans = rotateOn(rot, a);

    if (isnan(v[0])) return Vec3f(v[0],v[1],v[2]);

    return trans;
}

bool Vec3f::isCloseTo(const Vec3f &other, float epsilon)
{
    Vec3f diff = *this - other;

    // @NOTE: Just equalize height.
    diff[1] = 0;
    float difference = diff.magnitude();

    return difference < epsilon;
}




