#ifndef PROPERTYSTORE_H
#define PROPERTYSTORE_H

//////////////////////////////////////////////////////////////////////
//
// PropertyStore.  Useme to read info from property files.
//
// Date:		25 Agosto 2001
// Author:		Rodrigo Ramele (ramele@gmail.com)
// Versión:		1.0
//
// File format:
//		<file>                  ::==  <file>  |  <property definition>
//		<property definition>   ::==  <property name> '=' <value> 'CRLF'
//		<property name>         ::==  'digits and characters'
//		<value>                 ::==  'digits and characters'
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Deshabilita los mensajes de warning generados en la STL.
// Ubiquese antes de algún include de una STL.
#pragma warning(disable:4786)

#ifdef __APPLE__
#define DIRSEPARATOR "//"
#elif __linux
#define DIRSEPARATOR "//"
#else
#define DIRSEPARATOR "\\"
#endif

// Librerias de Estructura de Datos
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>

//#include "util.h"

using namespace std;

typedef std::map<std::string,char *> MAPACHTOACH;


// Constantes para los Archivos de Propiedades
const int KEYSIZE=256;
const int ELEMENTSIZE=256;

#define	EMPTYVALUE	"EMPTY"
#define OK		1
#define NOOK	0


class PropertyStore
{
private:
    /*
     * Tabla donde se almacenan las propiedades
     */
    MAPACHTOACH tableMap;
    /*
     * Archivo donde se leen y graban las propiedades
     */
    FILE * pFile;
    /*
     * Nombre del archivo de propiedades
     */
    char *sPropertyFileName;

public:
    template <class T> T Get(string stAreaKey,string stKey, T TDefaultValue);
    /*
     * @deprecated
     */
    void Flush();
    /*
     * Lee una linea completa de registro y devuelve los valores de la
     * clave y el elemento según la gramática del archivo de propiedades.
     *
     * @param	sAux			Buffer con los datos del registro.
     * @param	sKey OUT		Buffer de salida con el nombre de la propiedad.
     * @param	sElement OUT	Buffer de salida con el valor de la propiedad.
     * @return					OK si es un registro con formato válido.
     */
    int SplitKeyElement(const char *sAux, char *sKey, char *sElement);
    /*
     * Graba la tabla de propiedades en el archivo de propiedades.
     */
    void Save();
    /*
     * Carga la tabla de propiedades del archivo de propiedades.
     */
    void Load();
    /*
     * Limpia la tabla de propiedades.
     */
    void Clear();
    /*
     * Setea una propiedad.
     *
     * @param	stKey			string object con el nombre de la propiedad
     * @param	pGenericObject	Buffer con el vaor de la propiedad.
     */
    void Set(std::string stKey,char *pGenericObject);
    /*
     * Devuelve el valor de una propiedad
     *
     * @param	stKey			string object con el nombre de la propiedad.
     * @return					Devuelve un puntero a un buffer con el valor de la propiedad.
     */
    char * Get(std::string stKey);
    /*
     * Devuelve el valor de una propiedad que sea un entero, y si no se encuentra
     * definida devuelve el valor especificado en iDefaultValue.
     *
     * @param	stKey			string object con el nombre de la propiedad.
     * @param	iDefaultValue	Valor por defecto que devuelve el método si no encuentra la prop.
     * @return					Devuelve el valor de la propiedad en un entero.
     */
    int Get(std::string stKey,int iDefaultValue);
    /*
     * Construye un objeto especificando el nombre del archivo de propiedades.
     *
     * @param	sFileName		Nombre del archivo de propiedades
     */
    PropertyStore(char * sFileName);
    virtual ~PropertyStore();


};

#endif // PROPERTYSTORE_H
