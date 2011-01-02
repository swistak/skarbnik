#include <stdlib.h>
#include <libpq-fe.h>

int main()
{
  // próba po³±czenia
  PGconn *myconnection = PQconnectdb("");
  // sprawdzamy status po³±czenia
  if(PQstatus(myconnection) == CONNECTION_OK) {
    printf("connection made\n");
    // informacje o po³±czeniu
    printf("PGDBNAME   = %s\n",PQdb(myconnection));
    printf("PGUSER     = %s\n",PQuser(myconnection));
    printf("PGPASSWORD = %s\n",PQpass(myconnection));
    printf("PGHOST     = %s\n",PQhost(myconnection));
    printf("PGPORT     = %s\n",PQport(myconnection));
    printf("OPTIONS    = %s\n",PQoptions(myconnection));
  }
  else
    printf("connection failed: %s\n", PQerrorMessage(myconnection));
  // w razie utraty po³±czenia wywo³anie 
  // PQreset(myconnection);
  // zamyka op³±czenie i nawi±zuje je raz jeszcze  
  // z dotychczasowymi parametrami
  PQfinish(myconnection);
  return EXIT_SUCCESS;
}
