#ifndef MODEL_H
#define MODEL_H

#include "math/yamathutil.h"
#include "openglutils.h"

class Model
{
public:
    void virtual draw() = 0;
    void virtual draw(GLuint texture) = 0;
    void virtual setAnimation(const char *name) = 0;
    void virtual setTexture(GLuint texture) = 0;
    virtual ~Model() {  };
};

#endif // MODEL_H
