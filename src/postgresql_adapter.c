#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include "../include/connection_adapter.h"

PGconn* connection;

int ca_execute(char* query) {
    PGresult *raw_result;
    raw_result = PQexec(connection, query);

    int status = (
            PGRES_COMMAND_OK == PQresultStatus(raw_result) ||
            PGRES_TUPLES_OK == PQresultStatus(raw_result)
            ) ? 1 : 0;

    PQclear(raw_result);
    return (status);
}

DataPart ca_fetch_data_part(char* query) {
    DataPart result = malloc(sizeof (struct DataPartStructure));

    int resultFormat = 1;
    
    time_t tstart, tend;
    tstart = time(0);
    /* Execute a parameterized query. */
    PGresult *res = PQparamExec(
            connection,
            NULL,
            query,
            resultFormat
            );

    tend = time(0);

    result->raw = (void*) (res);

    result->run_time = difftime(tend, tstart);
    result->status = res ? 1 : 0;
    result->rows = PQntuples(res);
    result->cols = PQnfields(res);

    return (result);
}

void ca_free_data_part(DataPart result) {
    PQclear((PGresult*) result->raw);
    free(result);
}

void ca_connect() {
    connection = PQconnectdb("");

    if (PQstatus(connection) != CONNECTION_OK) {
        fprintf(stderr, "%s:%d (%s): %s",
                __FILE__,
                __LINE__,
                __func__,
                PQerrorMessage(connection)
                );
        exit(EXIT_FAILURE);
    }
}

char** ca_get_column_names(DataPart part) {
    char** result;
    result = malloc(part->cols * sizeof (char*));
    for (int i = 0; i < part->cols; i++) {
        result[i] = PQfname(part->raw, i);
    }

    return (result);
}

void ca_disconnect() {
    PQfinish(connection);
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