#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include "../include/connection_adapter.h"

PGconn* connection;

DataPart fetch_data_part() {
    PGresult *raw_result;
    DataPart result;

    char* command = "SELECT ;";

    printf("%s\n", command);

    time_t tstart, tend;
    tstart = time(0);
    raw_result = PQexec(connection, command);
    tend = time(0);

    result->run_time = difftime(tend, tstart);
    result->raw = raw_result;
    result->status = (
            PGRES_COMMAND_OK == PQresultStatus(raw_result) ||
            PGRES_TUPLES_OK == PQresultStatus(raw_result)
            ) ? 1 : 0;
    result->rows = PQntuples(raw_result);
    result->cols = PQnfields(raw_result);

    switch (PQresultStatus(raw_result)) {
        case PGRES_COMMAND_OK:
            break;
        case PGRES_TUPLES_OK:
            result->columns = malloc(result->cols * sizeof (char*));
            for (int i = 0; i < result->cols; i++) {
                result->columns[i] = PQfname(raw_result, i);
            }
            break;
        default:
            printf("error message: %s\n", PQresultErrorMessage(raw_result));
            break;
    }

    return (result);
}

void free_data_part(DataPart result) {
    for (int i = 0; i < result->cols; i++) {
        free(result->columns[i]);
    }
    free(result->columns);
    PQclear((PGresult*) (result->raw));
    free(result);
}

void connect() {
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

void disconnect() {
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