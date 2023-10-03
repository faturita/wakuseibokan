#include "sounds.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <unordered_map>

#include "soundtexture.h"

extern bool mute;

//std::unordered_map<std::string, SoundTexture*> soundtextures;
SoundTexture s;

void initSound()
{
    //SoundTexture* so = new SoundTexture();
    //so->init("sounds/takeoff.wav");
    //soundtextures["takeoff"] = so;

    //so = new SoundTexture();
    //so->init("sounds/gunshot.wav");
    //soundtextures["gunshot"] = so;

}

void clearSound()
{

}

void playsound(char *filename)
{
    // @FIXME: This is extremly risky !!!
    char temp[200];
    sprintf(temp, "%s %s &",PLAYSOUNDCOMMAND, filename);
    system(temp);
}

void bullethit()
{
    try {
        if (!mute && s.done) {
            //static SoundTexture s;
            s.init("sounds/bullethit.wav");
            s.play();
        }
    }  catch (StkError) {

    }

}

void firesound(int times)
{
    for(int i=0;i<times;i++)
        printf ("%c", 7);

}

void smallenginestart()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/boozing.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void enginestart()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/cruise.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void takeoff()
{
        try {
    if (!mute && s.done)
    {
        //static SoundTexture s;
        s.init("sounds/takeoff.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void explosion()
{
        try {
    if (!mute && s.done) {
        //static SoundTexture s;
        s.init("sounds/explosion.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void radarbeep()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/radarbeep.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void splash()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/splash.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void coast()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/coast.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void honk()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/boathonk.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void soaring()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/soaring.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void gunshot()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/gunshot.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void artilleryshot()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/artillery.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void droneflying()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/cephalopod.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}

void intro()
{
        try {
    if (!mute && s.done)  {
        //static SoundTexture s;
        s.init("sounds/intro.wav");
        s.play();
    }
    }  catch (StkError) {

    }
}
