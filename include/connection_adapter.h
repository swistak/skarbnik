/* 
 * File:   connection_adapter.h
 * Author: swistak
 *
 * Created on 2 styczeń 2011, 22:23
 */

#ifndef _CONNECTION_ADAPTER_H
#define	_CONNECTION_ADAPTER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <time.h>
#include <postgresql/libpq-fe.h>

    struct DataPartStructure {
        int rows;
        int cols;

        int status;
        float run_time;
        void* raw;
    };

    typedef struct DataPartStructure* DataPart;

    void ca_connect();
    void ca_disconnect();

    int ca_execute(char* query);
    DataPart ca_fetch_data_part(char* query, char* type_information, ...);
    void ca_free_data_part(DataPart result);

    //char** ca_get_column_names(DataPart part);
#ifdef	__cplusplus
}
#endif

#endif	/* _CONNECTION_ADAPTER_H */

