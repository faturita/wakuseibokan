#ifndef COMMANDLINE_H
#define COMMANDLINE_H

char * getCommandLineParameter(int argc, char* argv[], char *pcOpcion);
int getDefaultedIntCommandLineParameter(int argc, char* argv[], char *pcOpcion, int iDefault);
int isPresentCommandLineParameter(int argc, char *argv[], char *pcOpcion);

#endif // COMMANDLINE_H
