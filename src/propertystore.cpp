#include "propertystore.h"

// PropertyStore.cpp: implementation of the PropertyStore class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PropertyStore::PropertyStore(char * sPropertyFileName)
{
    PropertyStore::sPropertyFileName=sPropertyFileName;
}

PropertyStore::~PropertyStore()
{

}

/**
 * Devuelve el valor asignado a alguna propiedad.
 *
 * Si la propiedad no fue seteada devuelve EMPTYVALUE
 *
 * @param stKey		Objeto String con la clave a buscar.
 * @return			Devuelve el valor de la propiedad o EMPTYVALUE
 */
char * PropertyStore::Get(string stKey)
{
    if (tableMap[stKey]==NULL) {
        return EMPTYVALUE;
    }
    else {
        return (tableMap[stKey]);
    }

}

int PropertyStore::Get(string stKey,int iDefaultValue) {
    int iRetValue=iDefaultValue;
    char *sRetValue;

    sRetValue=Get(stKey);

    if (strcmp(sRetValue,EMPTYVALUE)) {
        iRetValue=atoi(sRetValue);
    }

    return iRetValue;
}


/**
 * Carga del archivo especificado en el objeto PropertyStore (en
 * su constructor) se cargan las propiedades definidas en el archivo.
 *
 * Se consideran propiedades (propiedad,valor) a los pares de strings
 * separados por un =
 *
 * Las claves de valores son objetos string, en tanto que los contenidos
 * son arrays de char planos.
 *
 */
void PropertyStore::Load()
{
    char *sAux=new char[KEYSIZE+ELEMENTSIZE+1];
    char *sKey=new char[KEYSIZE];
    char *sElement;

    Clear();

    if ( (pFile=fopen(sPropertyFileName,"rw+"))==NULL) {
        char dd[]="Archivo no encontrado.";
        throw (dd);
        cerr << "Archivo " << sPropertyFileName <<" no puedo ser abierto.";
        return;
    }

    string stKey;

    while (!feof(pFile)) {
        if (fgets(sAux,KEYSIZE+ELEMENTSIZE+1,pFile)==NULL)
            break;

        sElement=new char[ELEMENTSIZE];

        if (SplitKeyElement(sAux,sKey,sElement)) {
            stKey=sKey;
            //cout << stKey.c_str() << "- " << sAux << endl;
            Set(stKey,sElement);
            //cout << stKey.c_str() << "- " << tableMap[stKey] << endl;
        }

    }
    delete [] sKey;
    delete [] sAux;
    fclose(pFile);
}


void PropertyStore::Clear()
{
    MAPACHTOACH::iterator it;

    it=tableMap.begin();

    while (it!=tableMap.end()) {
        delete [] it->second;
        it++;
    }
}

/**
 * Setea el valor de una propiedad.
 *
 * Dada una clave, que puede o no existir previamente, se setea el
 * valor para esa propiedad.
 *
 * @param	stKey			Objeto string con la clave.
 * @param	pGenericObject	Objeto char *  con el array de contenido
 */
void PropertyStore::Set(string stKey, char *pGenericObject)
{
    if (pGenericObject!=NULL) {
        tableMap[stKey]=pGenericObject;
    }
}

/**
 * Almacena los valores de las claves seteadas.
 *
 * El archivo de configuración se reescribe para actualizar el
 * estado de las claves y para agregar las que no existan.
 *
 */
void PropertyStore::Save()
{
    if ( (pFile=fopen(sPropertyFileName,"w"))==NULL) {
        cerr << "No se puede abrir el archivo.\n";
        return;
    }

    char *sAux=new char[KEYSIZE+ELEMENTSIZE+1];
    MAPACHTOACH::iterator it;

    it=tableMap.begin();

    rewind(pFile);

    while (it!=tableMap.end()) {

        strcpy(sAux,(it->first).c_str());
        strcat(sAux,"=");
        strcat(sAux,it->second);
        strcat(sAux,"\n");
        //cout << sAux;
        fputs(sAux,pFile);
        it++;
    }

    delete [] sAux;
    fclose(pFile);
}

/**
 * Separa los valores leídos de (claves,valores) del archivo.
 *
 * @param	sAux		 char* leído desde el archivo.
 * @param	OUT sKey	 char* con la representación de la clave.
 * @param	OUT sElement char* con el elemento.
 * @return				 Devuelve OK si se encontró un par, y
 *						 devuelve NOOK si no se encontró.
 */
int PropertyStore::SplitKeyElement(const char *sAux,char *sKey,char *sElement)
{
    unsigned int flag=-1;
    unsigned int iKeyIndex=0;
    unsigned int iElementIndex=0;

    // Recorre el array de auxiliar y graba la clave en una posición
    // y el elemento en otra.
    // Solo se toman aquellos caracteres que sean considerados caracteres
    // validos.
    for(int i=0;i<strlen(sAux);i++) {
        if (sAux[i]=='=') {
            flag=i+1;continue;
        }

        if (sAux[i]>=32) {
            if (flag==-1) {
                sKey[iKeyIndex++]=sAux[i];
            }
            else {
                sElement[iElementIndex++]=sAux[i];
            }
        }
    }

    if (flag==-1)
        return NOOK;

    sKey[iKeyIndex]='\0';
    sElement[iElementIndex]='\0';
    return OK;
}

/**
 * @deprecated
 */
void PropertyStore::Flush()
{
    if (pFile!=NULL)
        fflush(pFile);
}

template <class T> T PropertyStore::Get(string stAreaKey, string stKey, T iDefaultValue)
{
    T iRetValue=iDefaultValue;
    char *sRetValue;

    sRetValue=Get(stKey);

    if (strcmp(sRetValue,EMPTYVALUE)) {
        iRetValue=atoi(sRetValue);
    }

    return iRetValue;
}
