#include <unordered_map>
#include "../profiling.h"
#include "WindTurbine.h"

extern std::unordered_map<std::string, GLuint> textures;

WindTurbine::WindTurbine(int faction)
{
    setFaction(faction);
}


void WindTurbine::init()
{
    //Load the model for the pole/tower (turretbase is just a cylinder, no baked-in blades)
    _model = (Model*)T3DSModel::loadModel(filereader("structures/turretbase.3ds"), 0.0f, 0.0f, 0.0f, 1, 5, 1, textures["metal"]);
    if (_model != NULL)
    {

    }

    Structure::height=200;
    Structure::length=50;
    Structure::width=50;

    setName("WindTurbine");

    setForward(0,0,1);
}

void WindTurbine::drawBlades()
{
    // Draw 3 blades (helices) as simple flat quads, evenly spaced at 120 degrees.
    // Each blade is a flat elongated rectangle extending outward from the hub.
    float bladeLength = 60.0f;
    float bladeWidth = 16.0f;
    float bladeThickness = 4.0f;

    // Enable texturing so blades use the metal texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures["metal"]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int i = 0; i < 3; i++)
    {
        glPushMatrix();
        glRotatef(i * 120.0f, 0.0f, 0.0f, 1.0f);  // Distribute 3 blades evenly

        // Draw one blade as a thin box extending upward from hub center
        glBegin(GL_QUADS);
        // Front face
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-bladeWidth * 0.5f, 0.0f,  bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( bladeWidth * 0.5f, 0.0f,  bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( bladeWidth * 0.5f, bladeLength,  bladeThickness * 0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-bladeWidth * 0.5f, bladeLength,  bladeThickness * 0.5f);
        // Back face
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-bladeWidth * 0.5f, 0.0f, -bladeThickness * 0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-bladeWidth * 0.5f, bladeLength, -bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( bladeWidth * 0.5f, bladeLength, -bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( bladeWidth * 0.5f, 0.0f, -bladeThickness * 0.5f);
        // Top face (blade tip)
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-bladeWidth * 0.5f, bladeLength, -bladeThickness * 0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-bladeWidth * 0.5f, bladeLength,  bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( bladeWidth * 0.5f, bladeLength,  bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( bladeWidth * 0.5f, bladeLength, -bladeThickness * 0.5f);
        // Side faces
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-bladeWidth * 0.5f, 0.0f, -bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-bladeWidth * 0.5f, 0.0f,  bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-bladeWidth * 0.5f, bladeLength,  bladeThickness * 0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-bladeWidth * 0.5f, bladeLength, -bladeThickness * 0.5f);

        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(bladeWidth * 0.5f, 0.0f, -bladeThickness * 0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(bladeWidth * 0.5f, bladeLength, -bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(bladeWidth * 0.5f, bladeLength,  bladeThickness * 0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(bladeWidth * 0.5f, 0.0f,  bladeThickness * 0.5f);
        glEnd();

        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
}

void WindTurbine::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (true || _model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        doTransform(f,R);

        _model->draw(textures["metal"]);

        // Move to the hub position at the top of the turretbase pole.
        // turretbase at scale 5: height ~140 (27.97*5), center of mass at y~-40,
        // so the top is around y=100. Tune as needed.
        glTranslatef(0.0f, 100.0f, 5.0f);

        // Rotate the blades around the Z axis (facing forward)
        glRotatef(bladeAngle, 0.0f, 0.0f, 1.0f);

        drawBlades();

        glPopMatrix();
    }
    else
    {
        CLog::Write(CLog::Debug,"Model is null.\n");
    }
}

int WindTurbine::getSubType()
{
    return WINDTURBINE;
}

EntityTypeId WindTurbine::getTypeId()
{
    return EntityTypeId::TWindTurbine;
}

void WindTurbine::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fw)
{
    position = getPos();
    fw = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,Structure::elevation);
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=10;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 90*fw + Up;
    fw = orig-position;

    setForward(fw);
}


void WindTurbine::tick()
{
    // Rotate blades continuously
    bladeAngle += 2.0f;
    if (bladeAngle >= 360.0f)
        bladeAngle -= 360.0f;

    production -= 0.01;
    if (production < 0)
    {
        production = 1;
        cargo[CargoTypes::POWERFUEL]++;
    }
}
