#include <stdlib.h>
#include <libpq-fe.h>

void printTuples(PGresult *result)
{
      int r, n;
      int nrows = PQntuples(result);
      int nfields = PQnfields(result);
      printf("number of rows returned = %d\n", nrows);
      printf("number of fields returned = %d\n", nfields);
      for(r = 0; r < nrows; r++) {
	for(n = 0; n < nfields; n++)
	  printf(" %s = %s(%d),\t", 
		 PQfname(result, n), 
		 PQgetvalue(result, r, n),
		 PQgetlength(result, r, n));
	printf("\n");
      }
}

void doSQL(PGconn *conn, char *command)
{
  PGresult *result;

  printf("%s\n", command);

  result = PQexec(conn, command);
  printf("status is %s\n", PQresStatus(PQresultStatus(result)));
  printf("#rows affected %s\n", PQcmdTuples(result));
  printf("result message: %s\n", PQresultErrorMessage(result));

  switch(PQresultStatus(result)) {
  case PGRES_TUPLES_OK:
    printTuples(result);
    break;
  }
  PQclear(result);
}
 
int main()
{
  PGresult *result;
  PGconn *conn;

  conn = PQconnectdb("");

  if(PQstatus(conn) == CONNECTION_OK &&
     PQsetnonblocking(conn,1) == 0) {
    printf("connection made\n");

    PQsendQuery(conn, "SELECT title, fname, lname FROM customer");
    while(result = PQgetResult(conn)) {
      printTuples(result);
      PQclear(result);
    }
  }
  else 
    printf("connection failed\n");

  PQfinish(conn);
  return EXIT_SUCCESS;
}
