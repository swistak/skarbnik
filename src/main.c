/* 
 * File:   main.c
 * Author: swistak
 *
 * Created on 30 grudzień 2010, 21:07
 */

#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <time.h>
#include <malloc.h>

int main(int argc, char** argv) {
    if (argv[0] == "-h") {
        printf("usage: PGDBNAME=db_name PGUSER=username PGPASSWORD=pass PGHOST=localhost ./skarbnik \n");
    }

    return (EXIT_SUCCESS);
}

