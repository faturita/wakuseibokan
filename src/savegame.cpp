#include "savegame.h"

#include <vector>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <regex>


#include "container.h"
#include "profiling.h"

#include "units/Vehicle.h"
#include "terrain/Terrain.h"
#include "messages.h"

#include "engine.h"

#include "units/BoxVehicle.h"
#include "units/Walrus.h"
#include "units/Manta.h"
#include "units/SimplifiedDynamicManta.h"
#include "units/Buggy.h"
#include "units/MultiBodyVehicle.h"
#include "units/Balaenidae.h"
#include "units/Beluga.h"
#include "units/AdvancedWalrus.h"
#include "units/Medusa.h"
#include "units/AdvancedManta.h"
#include "units/Stingray.h"

#include "structures/Structure.h"
#include "structures/Runway.h"
#include "structures/Hangar.h"
#include "structures/Turret.h"
#include "structures/Warehouse.h"
#include "structures/Laserturret.h"
#include "structures/CommandCenter.h"
#include "structures/Launcher.h"

#include "actions/Gunshot.h"
#include "actions/Missile.h"

#include "usercontrols.h"

extern  Controller controller;

extern int gamemode;

extern int aiplayer;

extern unsigned long timer;

extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern std::ofstream msgboardfile;
extern std::vector<Message> messages;

extern dWorldID world;
extern dSpaceID space;


void savegame()
{
    std::ofstream ss("savegame.w", std::ios_base::binary);

    // Version
    ss << 0x01 << std::endl;

    ss << aiplayer << std::endl;

    ss << controller.faction << std::endl;

    ss << gamemode << std::endl;

    ss << timer << std::endl;

    int entitiessize = 0;
    // Get flying entities.
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        if (entities[i]->getType() == CARRIER || entities[i]->getType() == MANTA || entities[i]->getType() == WALRUS)
        {
            entitiessize++;
        }
    }

    ss << entitiessize << std::endl;
    for(size_t i=entities.first();entities.hasMore(i);i=entities.next(i))
    {
        if (entities[i]->getType() == CARRIER || entities[i]->getType() == MANTA || entities[i]->getType() == WALRUS)
        {
            ss << entities[i]->getFaction() << std::endl;
            ss << entities[i]->getType() << std::endl;

            int subtype = 0;


            if (AdvancedWalrus *lb = dynamic_cast<AdvancedWalrus*>(entities[i]))
                subtype = 1;
            else if (Walrus *lb = dynamic_cast<Walrus*>(entities[i]))
                subtype = 2;
            else if (Medusa *lb = dynamic_cast<Medusa*>(entities[i]))
                subtype = 6;
            else if (AdvancedManta *lb = dynamic_cast<AdvancedManta*>(entities[i]))
                subtype = 3;
            else if (Stingray *lb = dynamic_cast<Stingray*>(entities[i]))
                subtype = 7;
            else if (Beluga *lb = dynamic_cast<Beluga*>(entities[i]))
                subtype = 4;
            else if(Balaenidae* lb = dynamic_cast<Balaenidae*>(entities[i]))
                subtype = 5;

            ss << subtype << std::endl;

            dout << "Subtype saving:" << subtype << std::endl;

            Vec3f p= entities[i]->getPos();
            ss << p[0] << std::endl << p[1] << std::endl << p[2] << std::endl;
            ss << entities[i]->getHealth() << std::endl;
            ss << entities[i]->getPower() << std::endl;


            float R[12];
            entities[i]->getR(R);
            for(int j=0;j<12;j++) ss << R[j] << std::endl;

            ss << entities[i]->isAuto() << std::endl;
            p = entities[i]->getDestination();
            ss << p[0] << std::endl << p[1] << std::endl << p[2] << std::endl;


        }
    }


    ss << islands.size() << std::endl;
    for (int j=0;j<islands.size();j++)
    {
        std::string s;
        s = islands[j]->getName();

        s = std::regex_replace(s, std::regex(" "), "-");
        ss << s << std::endl;
        ss << islands[j]->getX() << std::endl;
        ss << islands[j]->getZ() << std::endl;
        ss << islands[j]->getModelName() << std::endl;
        dout << "Name:" << islands[j]->getName() << std::endl;

        Structure *c =  islands[j]->getCommandCenter();

        if (c)
        {
            std::vector<size_t> strs = islands[j]->getStructures();
            ss << 0x3f << std::endl;
            ss << strs.size() << std::endl;
            for(int i=0;i<strs.size();i++)
            {
                ss << entities[strs[i]]->getFaction() << std::endl;
                ss << entities[strs[i]]->getType() << std::endl;
                ss << entities[strs[i]]->getSubType() << std::endl;

                int typeofisland = 0x4f;

                if (entities[strs[i]]->getSubType() == VehicleSubTypes::COMMANDCENTER)
                    typeofisland = ((CommandCenter*)entities[strs[i]])->getIslandType();

                ss << typeofisland << std::endl;

                Vec3f p= entities[strs[i]]->getPos();
                ss << p[0] << std::endl << p[1] << std::endl << p[2] << std::endl;
                float orientation = getAzimuthRadians(entities[strs[i]]->getForward());
                ss << orientation << std::endl;
                ss << entities[strs[i]]->getHealth() << std::endl;
                ss << entities[strs[i]]->getPower() << std::endl;

            }
        } else {
            ss << 0x4f << std::endl;
        }


    }

    ss.flush();
    ss.close();


}



void loadgame()
{
    /**std::ifstream ss("savegame.w", std::ios_base::binary);

    Vec3f f(0,0,0);
    ss >> f[0] >> f[1] >> f[2] ;

    dout << f << std::endl;

    ss.close();**/

    std::ifstream ss("savegame.w", std::ios_base::binary);

    int version;
    ss >> version;

    ss >> aiplayer;

    ss >> controller.faction;

    ss >> gamemode;

    ss >> timer;

    int size;
    ss >> size;
    dout << "Size:" << size << std::endl;
    for(int i=0;i<size;i++)
    {
        Vehicle *v = NULL;
        int faction;
        ss >> faction;
        int type, subtype;
        ss >> type;
        ss >> subtype;
        dout << "Type:" << type << " subtype:" << subtype << std::endl;

        if (type == CARRIER || type == MANTA || type == WALRUS)
        {
            switch (type) {
            case CARRIER:
            {
                Balaenidae *b = NULL;
                if (subtype==5)
                    b = new Balaenidae(faction);
                else if (subtype==4)
                    b = new Beluga(faction);

                b->init();
                b->embody(world,space);
                v = b;
                break;
            }
            case MANTA:
            {
                Manta *_manta1 = NULL;

                if (subtype == 6)
                    _manta1 = new Medusa(faction);
                else if (subtype == 7)
                    _manta1 = new Stingray(faction);
                else
                    _manta1 = new AdvancedManta(faction);

                _manta1->init();
                _manta1->setNumber(findNextNumber(MANTA));
                _manta1->embody(world, space);
                _manta1->setStatus(FlyingStatus::FLYING);              // @FIXME, status should be stored.
                _manta1->inert = true;
                v = _manta1;
                break;
            }
            case WALRUS:
                Walrus *_walrus = NULL;
                if (subtype == 1)
                    _walrus = new AdvancedWalrus(faction);
                else if (subtype == 2)
                    _walrus = new Walrus(faction);
                _walrus->init();
                _walrus->setNumber(findNextNumber(WALRUS));
                _walrus->embody(world, space);
                _walrus->setStatus(Walrus::SAILING);
                //_walrus->inert = true;
                v = _walrus;

            }
            Vec3f f(0,0,0);
            ss >> f[0] >> f[1] >> f[2] ;
            v->setPos(f);
            float health;ss >> health ;v->damage(1000-health);
            float power; ss >> power ;v->setPower(power);

            float R[12];
            for(int j=0;j<12;j++) ss >> R[j];
            v->setRotation(R);


            if (type == MANTA)
            {
                Manta *m = (Manta*) v;
                m->release(v->getForward());
            }

            // Destination and auto
            bool isauto;
            ss >> isauto;
            ( isauto ? v->enableAuto() : v->disableAuto());

            ss >> f[0] >> f[1] >> f[2] ;
            v->setDestination(f);

            entities.push_back(v, v->getGeom());
        }
    }

    ss >> size;
    dout << "Size:" << size << std::endl;
    for (int j=0;j<size;j++)
    {
        BoxIsland *is = new BoxIsland(&entities);
        std::string name,modelname;
        Vec3f loc;
        ss >> name;is->setName(name);
        ss >> loc[0];
        ss >> loc[2];
        is->setLocation(loc[0],-1,loc[2]);
        ss >> modelname;
        dout << "Reading Island:" << name << "\t" << modelname << std::endl;
        is->buildTerrainModel(space,modelname.c_str());

        islands.push_back(is);

        int moredata;

        ss >> moredata;

        if (moredata == 0x3f)
        {
            int sze;
            ss >> sze;
            for (int i=0;i<sze;i++)
            {
                Structure *v = NULL;
                int faction;
                ss >> faction;
                int type, subtype;
                ss >> type;
                ss >> subtype;
                dout << "Type:" << type << " subtype:" << subtype << std::endl;
                int typeofisland = 0x4f;
                ss >> typeofisland;

                switch (subtype) {
                case 10:
                    v = new Artillery(faction);
                    break;
                case 11:
                    v = new CommandCenter(faction,typeofisland);
                    break;
                case 12:
                    v = new Hangar(faction);
                    break;
                case 13:
                    v = new Warehouse(faction);
                    break;
                case 14:
                    v = new Runway(faction);
                    break;
                case 15:
                    v = new LaserTurret(faction);
                    break;
                case 16:
                    v = new Turret(faction);
                    break;
                case 17:
                    v = new Launcher(faction);
                    break;
                case 18:
                    v = new Factory(faction);
                    break;
                case 19:
                    v = new Dock(faction);
                    break;
                case 20:
                    v = new Antenna(faction);
                    break;
                case 21:
                    v = new Radar(faction);
                    break;
                case 22:default:
                    v = new Structure(faction);
                    break;
                }

                Vec3f f(0,0,0);
                ss >> f[0] >> f[1] >> f[2] ;
                float orientation; ss >> orientation;
                float health;ss >> health ;
                float power; ss >> power ;

                is->addStructure(v   ,       -is->getX()+f[0],    -is->getZ()+f[2],orientation,world);

            }
        }

    }


    ss.close();

}

