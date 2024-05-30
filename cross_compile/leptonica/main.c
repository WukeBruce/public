
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>

#include "allheaders.h"

//picture opterate test
extern int sharptest(int argc, char **argv);
extern  int rotatetest(int    argc,char **argv);
extern int rotate_it(int    argc, char **argv);
extern int scale_it(int    argc, char **argv);


int main(int argc, char **argv)
{
        //sharptest( argc,  argv);
        //rotatetest( argc,  argv);
       //rotate_it( argc,  argv);
        scale_it( argc,  argv);// ./out.elf zier.jpg 0.5 0.5 wk.jpg 0
        return 0;
}



