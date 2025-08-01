/* ============================================================================
**
** Vehicle Base Class for all Entities - Wakuseiboukan - 16/01/2011
**
** Copyright (C) 2014  Rodrigo Ramele
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

#ifndef VEHICLE_H_
#define VEHICLE_H_


#include <stdio.h>
#include <queue>
#include "../math/vec3f.h"
#include "../math/yamathutil.h"
#include "../model.h"
#include "../openglutils.h"
#include "../odeutils.h"
#include "../observable.h"
#include "../usercontrols.h"

struct ModelRecord {
    float pos1;
    float pos2;
    float pos3;
    float r1;
    float r2;
    float r3;
    float r4;
    float r5;
    float r6;
    float r7;
    float r8;
    float r9;
    float r10;
    float r11;
    float r12;

    float vel1;
    float vel2;
    float vel3;

};

struct TickRecord {
    unsigned long timerparam;
    size_t id;
    int typeId;
    int type;
    int subtype;
    int faction;
    float health;
    int power;
    int status;
    int ttl;
    int number;
    int cargo[1];
    float elevation;      // For structures and weapons
    float azimuth;          // For structures and weapons

    ModelRecord location;

    float orientation;  // For structures
    size_t islandId;    // For structures
};


// TypeId for serialization.  It recognizes each one of the C++ classes.
enum EntityTypeId {
    TBalaenidae = 1,
    TBeluga = 2,
    TWalrus = 3,
    TAdvancedWalrus = 4,
    TSimplifiedManta = 5,
    TMedusa = 6,
    TStingray = 7,
    TCephalopod = 8,
    TOtter = 9,
    TArtillery = 10,
    TCommandCenter = 11,
    THangar = 12,
    TWarehouse = 13,
    TRunway = 14,
    TLaserTurret = 15,
    TTurret = 16,
    TLauncher = 17,
    TFactory = 18,
    TDock = 19,
    TAntenna = 20,
    TRadar = 21,
    TStructure = 22,
    TAdvancedManta = 23,
    TBoxVehicle = 24,
    TBuggy = 25,
    TManta = 26,
    TMultiBodyVehicle = 27,
    TSimplifiedDynamicManta = 28,
    TWheel = 29,
    TWheeledManta = 30,
    TCarrierArtillery = 31,
    TCarrierLauncher = 32,
    TCarrierTurret = 33,
    TWeapon = 34,
    TAAM = 35,
    TArtilleryAmmo = 36,
    TBomb = 37,
    TDebris = 38,
    TExplosion = 39,
    TGunshot = 40,
    TLaserBeam = 41,
    TLaserRay = 42,
    TMissile = 43,
    TShell = 44,
    TTorpedo = 45,
    TTurtle = 46,
    TArmory = 47,
    TWindTurbine = 48,
    TCargoShip = 49
};


/**
 * @brief Structures are identified if their type number is greater than or equal to COLLISIONABLE.
 */
enum VehicleTypes { RAY=1, WALRUS=2, MANTA=3, CARRIER=4, ACTION=5, CONTROLABLEACTION = 6 , NAVALCONTROLABLEACTION = 7, EXPLOTABLEACTION = 8, WEAPON = 9, COLLISIONABLE=15, LANDINGABLE = 16, CONTROL=17};

enum VehicleSubTypes { BALAENIDAE = 1, BELUGA = 2, SIMPLEWALRUS = 3, ADVANCEDWALRUS = 4, SIMPLEMANTA = 5, MEDUSA = 6, STINGRAY = 7, CEPHALOPOD = 8, OTTER = 9, TURTLE = 10, CARGOSHIP = 11, ARTILLERY = 12, COMMANDCENTER = 13, HANGAR = 14, WAREHOUSE = 15, RUNWAY = 16,  TURRET = 17, LAUNCHER = 18, FACTORY = 19, DOCK = 20, ANTENNA = 21, RADAR = 22, STRUCTURE = 23, ARMORY = 24, WINDTURBINE = 25,LASERTURRET = 26};

enum class AutoStatus { IDLE=1, DESTINATION, WAYPOINT, LANDING, ATTACK, DOGFIGHT, DROP, GROUND, AIR, WATER, DOCKING, SENTRY };

enum ORDERS { ATTACK_ISLAND=1, DEFEND_CARRIER, DEFEND_ISLAND, CONQUEST_ISLAND };

enum FACTIONS {GREEN_FACTION = 1, BLUE_FACTION = 2, BOTH_FACTION = 3};

#define FACTION(m) ( m == GREEN_FACTION ? "Balaenidae" : "Beluga")

enum class DestinationStatus { READY=0, TRAVELLING, REACHED};

enum class WEAPONS { GUNSHOT=1, MISSILE=2, BOMB=3 };

enum CargoTypes { POWERFUEL=0 };

enum SailingStatus { SAILING=0, ROLLING, OFFSHORING, INSHORING, INDOCKING, DOCKED};

class Vehicle : Observable
{
private:
    int ttl=-1;  //Live for ever.

    float health = 1000.0;
    int power = 1000;

    int faction=-1;

    bool dotelemetry=false;

    int statuschangedelay=0;
    int finaldelayedstatus = SailingStatus::SAILING;



protected:
    Vec3f pos;
    Vec3f vel;
    Model* _model;
    Model* _topModel;
	float speed;
	float R[12];
	Vec3f forward;
    float throttle=0;
    float engine[3];
    float steering;
    Vec3f destination;
    Vec3f attitude;         // This is the set point for forward.

    std::queue<Vec3f> waypoints;

    // Signal determines if the vehicle can be remotely controlled despite of not being close the carrier or a friendly island.
    int signal=3;


    
    // ODE objects
    dBodyID me=NULL;
    dGeomID geom=NULL;
    dSpaceID body_space = NULL;

    struct controlregister registers;

    bool aienable = false;
    AutoStatus autostatus = AutoStatus::IDLE;

    int status=0;
    int order=0;

    void setTtl(int ttlvalue);

    void setFaction(int newfaction);


    // Vehicle name and number
    std::string name;
    int number;

    // Bounding box dimensions.
    float height;
    float width;
    float length;

    // Location of the vehicle on the screen coordinates.
    Vec3f onScreen;

    Vec3f offset;

    int cargo[256];
    int capacity[256];

    float fueltank=1;
    
    void doMaterial();

public:
    bool inert=false;
    float xRotAngle=0;
    float yRotAngle=0;
	Vehicle();
    ~Vehicle();
    
    int virtual getType();
    int virtual getSubType();

    void setAutoStatus(AutoStatus au);
    
    void virtual init();                                        // Init, when the entity is created.
    void virtual clean();                                       // Clean, do some cleanup when the entitiy is above to be removed. DON't USE destructor !
	void setSpeed(float speed);
	float getSpeed();
	void setXRotAngle(float xRotAngle);
	void setYRotAngle(float yRotAngle);
    void virtual setPos(const Vec3f &newpos);
    void virtual setPos(float x, float y, float z);
	Vec3f getPos();
    Vec3f getVelocity();
    void setVelocity(float vx,float vy, float vz);
	void setForward(float x, float y, float z);
    void setForward(Vec3f);
    Vec3f virtual getForward();

    dSpaceID myspace();

    void getR(float retR[12]);
    void setR(float newR[12]);
    void alignToMe(dBodyID fBodyID);
    void alignToMyBody(dBodyID fBodyID);

    void setRotation(float newR[12]);

	void virtual drawModel(float yRot, float xRot, float x, float y, float z);
    void virtual drawModel(Vec3f offset);
    
	void virtual getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward);
	void setLocation(float fPos[3], float R[12]);

	void virtual setThrottle(float thorttle);
	float virtual getThrottle();
    void virtual upThrottle(float throttle);

    float virtual getEnergyConsumption();

    void virtual stop();
    void virtual stop(dBodyID who);

	void virtual doDynamics(dBodyID);
    void virtual doDynamics();

    void virtual doControl(Controller);
    void virtual doControl(struct controlregister);
    void virtual doControl();
    
    void virtual embody(dBodyID myBodySelf);
    void antigravity(dBodyID myBodySelf);

    void wrapDynamics(dBodyID body);
    
    dBodyID virtual getBodyID();
    dGeomID virtual getGeom();

    void setVector(float* V, Vec3f v);
    void setVector(float *V, dVector3 v);

    Vec3f dBodyGetLinearVelInBody(dBodyID body);
    Vec3f dBodyGetAngularVelInBody(dBodyID body);
    Vec3f dBodyGetLinearVelVec(dBodyID body);

    struct controlregister getControlRegisters();
    void setControlRegisters(struct controlregister);
    void resetControlRegisters();

    float getBearing();
    void goTo(Vec3f target);
    Vec3f getDestination() const;

    void addWaypoint(Vec3f target);
    void clearWaypoints();

    void virtual attack(Vec3f target);
    void setDestination(Vec3f dest);

    void setAttitude(Vec3f attit);
    Vec3f getAttitude();

    virtual Vehicle* fire(int weapon, dWorldID world, dSpaceID space);

    virtual int getTtl();
    virtual void tick();

    int getCargo(int type);
    int setCargo(int type, int value);
    int removeCargo(int type, int value);
    int addCargo(int type, int value);

    bool isAuto();
    void enableAuto();
    void disableAuto();

    void goWaypoints();

    bool arrived();
    void ready();

    virtual Vehicle* spawn(dWorldID world, dSpaceID space,int type, int number);

    int getStatus() const;
    void setStatus(int value);
    void setDelayedStatus(int status, int delay, int finalstatus);
    float getHealth() const;
    virtual void damage(float d);
    void destroy();

    const Vec3f map(Vec3f);

    bool VERIFY(Vec3f newpos, dBodyID who);

    int getFaction();
    int getPower() const;
    void setPower(int value);

    Vec3f toWorld(dBodyID body,Vec3f fw);
    Vec3f toBody(dBodyID body, Vec3f fw);
    int getSignal() const;
    void setSignal(int value);
    int getOrder() const;
    void setOrder(int value);
    AutoStatus getAutoStatus() const;
    bool isAutoStatusFree();

    std::string subTypeText(int code);

    std::string getName();
    void setName(char msg[256], int number);
    void virtual setNameByNumber(int number);
    void setName(std::string);

    void setNumber(int number);
    int getNumber();

    void enableTelemetry();
    void disableTelemetry();

    Vec3f getDimensions();

    void updateScreenLocation();
    Vec3f screenLocation();

    void setTheOrientation(Vec3f orientation);

    virtual TickRecord  serialize();
    virtual void        deserialize(TickRecord);

    virtual EntityTypeId getTypeId();

    // State Machine for handling destinations.
    DestinationStatus dst_status;

    std::vector<Vec3f> getVertices();

};

#endif /* VEHICLE_H_ */
