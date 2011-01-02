#define _GNU_SOURCE

#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


typedef void (*ResultHandler)(PGresult*);

void sql(PGconn *conn, char *command, ResultHandler callback) {
    PGresult *result;

    printf("%s\n", command);

    time_t tstart, tend;

    tstart = time(0);
    result = PQexec(conn, command);
    tend = time(0);

    printf(
            "took: %.2f (s) status is: %s returned rows: %d fields: %d affected: %s\n",
            difftime(tend, tstart),
            PQresStatus(PQresultStatus(result)),
            PQntuples(result),
            PQnfields(result),
            PQcmdTuples(result)
            );

    switch (PQresultStatus(result)) {
        case PGRES_COMMAND_OK:
            break;
        case PGRES_TUPLES_OK:
        {
            if (callback) callback(result);
            break;
        }
        default:
            printf("error message: %s\n", PQresultErrorMessage(result));
            break;
    }

    PQclear(result);
}

void print_result(PGresult* result) {
    PQprintOpt pqp;
    pqp.header = 1;
    pqp.align = 1;
    pqp.html3 = 0;
    pqp.expanded = 0;
    pqp.pager = 0;
    pqp.fieldSep = "";
    pqp.caption = "";
    pqp.fieldName = NULL;
    PQprint(stdout, result, &pqp);
};

void establish_connection() {
    PGconn *connection = PQconnectdb("host=localhost dbname=event_testing user=keke");

    if (PQstatus(connection) == CONNECTION_OK) {
        printf("Connected...\n");
        sql(connection, "SELECT COUNT(*) FROM events;", print_result);
    } else {
        printf("connection failed: %s\n", PQerrorMessage(connection));
    }

    PQfinish(connection);
}