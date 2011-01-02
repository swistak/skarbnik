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

        char** columns;
    };

    typedef struct DataPartStructure* DataPart;

    void connect();
    void disconnect();



#ifdef	__cplusplus
}
#endif

#endif	/* _CONNECTION_ADAPTER_H */

