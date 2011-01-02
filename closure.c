#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include <time.h>
#include <malloc.h>

typedef unsigned long long int BitPart;
typedef unsigned long long int Int;

struct ProtoObjectStructure;
typedef struct ProtoObjectStructure* ProtoObject;
typedef ProtoObject(*ProtoObjectMethod)(ProtoObject);

#define make_method(x) NULL

struct ProtoObjectStructure {
    char* name;

    ProtoObjectMethod print;
    ProtoObject (*rename)(char* new_name);
};

ProtoObject print(ProtoObject self);

ProtoObject print(ProtoObject self) {
    printf("ProtoObject#print: %s\n", self->name);
};

ProtoObject rename_object(ProtoObject self, char* new_name){
    free(self->name);
    self->name = new_name;
}

int main(int argc, char** argv) {
    ProtoObject self = malloc(sizeof (struct ProtoObjectStructure));
    self->print = &print;
    self->rename = make_method(char* new_name);
    self->name = "TestName";
    self->print(self);

    return (EXIT_SUCCESS);
}

