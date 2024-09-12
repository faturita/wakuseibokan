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



#ifndef VEC3F_H_INCLUDED
#define VEC3F_H_INCLUDED

#include <iostream>

#define kmf *1000.0f

class Vec3f {
	private:
		float v[3];
	public:
		Vec3f();
		Vec3f(float x, float y, float z);
		
		float &operator[](int index);
		float operator[](int index) const;
		
		Vec3f operator*(float scale) const;
		Vec3f operator/(float scale) const;
		Vec3f operator+(const Vec3f &other) const;
		Vec3f operator-(const Vec3f &other) const;
		Vec3f operator-() const;
		
		const Vec3f &operator*=(float scale);
		const Vec3f &operator/=(float scale);
		const Vec3f &operator+=(const Vec3f &other);
		const Vec3f &operator-=(const Vec3f &other);
        bool isEquals(const Vec3f other);
        bool isZero();
		bool isCloseTo(const Vec3f &other, float epsilon);
		
		float magnitude() const;
		float magnitudeSquared() const;
		Vec3f normalize() const;
		float dot(const Vec3f &other) const;
		Vec3f cross(const Vec3f &other) const;

		Vec3f rotateOnX(float alpha);
		Vec3f rotateOnY(float alpha);
		Vec3f rotateOnZ(float alpha);

        Vec3f rotateOn(Vec3f u, float alpha);

        Vec3f rotateTo(Vec3f originalDirection, Vec3f newDirection);

        char * toString(char *) const;

};

Vec3f operator*(float scale, const Vec3f &v);
std::ostream &operator<<(std::ostream &output, const Vec3f &v);
std::istream &operator>>(std::istream &input, Vec3f &v);










#endif
