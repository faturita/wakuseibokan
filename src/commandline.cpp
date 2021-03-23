#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/***************************************************************************
 * Lee los parámetros de la línea de comando devolviendo el dato
 * para ese parámetro.
 *
 * argc				Cantidad de argumentos de la línea de comando.
 * argv				Array de cadenas con los parámetros de la línea
 *					de comando.
 * pcOpcion			Modificador a buscar en la línea de comandos.
 */
char * getCommandLineParameter(int argc, char* argv[], const char *pcOpcion)
{
    char *pcValor;

    for (int i=0;i<argc;i++) {
        pcValor = strstr(argv[i],pcOpcion);

        // Si encontró el parámetro y el mismo tiene algun valor
        if ((pcValor!=NULL) && strlen(pcValor)>strlen(pcOpcion) )
            return pcValor+strlen(pcOpcion);
        else if ((pcValor != NULL) && (i+1)<argc )
            return argv[i+1];
    }

    // No encontró nada.
    return NULL;
}

int isPresentCommandLineParameter(int argc, char *argv[], const char *pcOpcion)
{
    char *pcValor;

    for (int i=0;i<argc;i++) {
        pcValor = strstr(argv[i],pcOpcion);

        // Si encontró el parámetro y el mismo tiene algun valor
        if ((pcValor!=NULL))
            return 1;
    }

    // No encontró nada.
    return 0;
}

/******************************************************************************
 * Permite devolver un parámetro leído de la línea de comando numérico,
 * con la posibilidad de que si el mismo no es encontrado se devuelve un
 * valor por default.
 *
 * argc				Cantidad de parámetros en cmdline.
 * argv				Array de cadenas de cmdline.
 * pcOpcion			Modificador a buscar.
 * iDefault			Valor entero a devolver si no se encuentra pcOpcion.
 */
int getDefaultedIntCommandLineParameter(int argc, char* argv[], const char *pcOpcion, int iDefault)
{
    char *pcValor = getCommandLineParameter(argc,argv,pcOpcion);

    if (pcValor==NULL)
        return iDefault;
    else
        return atoi(pcValor);
}
