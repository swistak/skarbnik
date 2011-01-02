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

#define TREE_SIZE 2

typedef struct {
    struct Node* children;
    int dimension;
    int value;
} Node;

Node* alloc_node() {
    Node* new_node = malloc(sizeof (Node));
    new_node->children = malloc(TREE_SIZE * sizeof (Node*));

    return (new_node);
}

void build_tree() {
    int dimension;
    dimension = (dimension + 1) % TREE_SIZE;

    
}

void process_result(PGresult* result) {

}

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

int main(int argc, char** argv) {
    PGconn *connection = PQconnectdb("host=localhost dbname=event_testing user=keke");

    if (argv[0] == "-h") {
        printf("usage: PGDBNAME=db_name PGUSER=username PGPASSWORD=pass PGHOST=localhost ./skarbnik \n");
    }

    if (PQstatus(connection) == CONNECTION_OK) {
        printf("Connected...\n");
        sql(connection, "SELECT COUNT(*) FROM events;", print_result);
    } else {
        printf("connection failed: %s\n", PQerrorMessage(connection));
    }

    PQfinish(connection);
    return (EXIT_SUCCESS);
}

