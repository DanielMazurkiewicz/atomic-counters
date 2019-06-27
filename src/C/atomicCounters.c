
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "./lib/napiMacros.h"
#include "./lib/types.h"

#include "./types.h"
#include "./common.h"
#include "./base.h"

#include <stdio.h>



AtomicCounter* atomicCounters_prepare(AtomicCounters* db, char* name, size_t nameLength) {
    AtomicCounter* counter = (AtomicCounter*)calloc(1, sizeof(AtomicCounter));
    counter->context = db;
    S32 timeout = db->timeout;

    U64 offsetTest = 0;
    while (timeout > 0) {

        atomicCounters_check_if_file_resized(db);
        while (offsetTest < db->freeSpace) {
            if (!strcmp(name, (char*)(db->data + sizeof(AtomicCounterInDb) + offsetTest))) {
                counter->counterOffset = offsetTest;
                return counter;
            }
            U64 dbCounterSize = (sizeof(AtomicCounterInDb) + ((AtomicCounterInDb*)(db->data + offsetTest))->nameLength) / 8;
            dbCounterSize = (dbCounterSize + 1) * 8;
            offsetTest += dbCounterSize;
        }

        U32 previous = atomic_flag_test_and_set(&db->header->adding);
        if (previous) { // failed to set flag
            while (db->header->adding && timeout-- > 0) {
                usleep(1000); // wait 1ms
            }
        } else {
            U64 sizeOfSpace = (nameLength + sizeof(AtomicCounterInDb)) / 8;
            sizeOfSpace = (sizeOfSpace + 1) * 8;
            S64 offset = atomicCounters_get_space(db, sizeOfSpace);
            AtomicCounterInDb* atomicCounterInDb = db->data + offset;
            atomicCounterInDb->nameLength = nameLength;
            memcpy((DATA)((DATA)atomicCounterInDb + sizeof(AtomicCounterInDb)), (void*)(name), nameLength);
            atomicCounters_close_space(db);
            atomic_flag_clear(&db->header->adding);
            counter->counterOffset = offset;
            return counter;
        }
    }
    atomic_flag_clear(&db->header->adding);
    free(counter);
    return NULL;
}

AtomicCounter* atomicCounters_release(AtomicCounter* counter) {
    free(counter);
}


function(openCounters) {
    napi_status status;
    var(result);
    n_getArguments(args, argsCount, 4, status);

    size_t pathLength;
    n_getStringUtf8ZLength(pathLength, args[0]);

    char path[ pathLength ];
    n_getStringUtf8Z(path, args[0], pathLength);

    S32 fileGrowSize;
    n_getS32(fileGrowSize, args[1], status);

    S32 timeout;
    n_getS32(timeout, args[2], status);

    S32 initializationTimeout;
    n_getS32(initializationTimeout, args[3], status);

    AtomicCounters* db = atomicCounters_open(path, fileGrowSize, timeout, initializationTimeout);

    n_assignArrayBuffer(result, db, sizeof(AtomicCounters)); // little trick since we can"t determine size of ups_env_t
    return result;
}

function(closeCounters) {
    napi_status status;
    var(result);

    
    n_getArguments(args, argsCount, 1, status);

    AtomicCounters* db;
    n_getArrayBufferPointer(db, args[0], status);

    atomicCounters_close(db);

    n_setUndefined(result, status);
    return result;
}


function(prepare) {
    napi_status status;
    var(result);
    n_getArguments(args, argsCount, 2, status);

    AtomicCounters* db;
    n_getArrayBufferPointer(db, args[0], status);

    size_t nameLength;
    n_getStringUtf8ZLength(nameLength, args[1]);

    char name[ nameLength ];
    n_getStringUtf8Z(name, args[1], nameLength);

    AtomicCounter* atomicCounter = atomicCounters_prepare(db, name, nameLength);
    if (atomicCounter) {
        n_assignArrayBuffer(result, atomicCounter, sizeof(AtomicCounter));
        return result;
    }
    n_setUndefined(result, status);
    return result;
}


function(release) {
    napi_status status;
    var(result);

    n_getArguments(args, argsCount, 1, status);

    AtomicCounter* atomicCounter;
    n_getArrayBufferPointer(atomicCounter, args[0], status);

    atomicCounters_release(atomicCounter);

    n_setUndefined(result, status);
    return result;
}

function(current) {
    napi_status status;
    var(result);

    n_getArguments(args, argsCount, 1, status);

    AtomicCounter* atomicCounter;
    n_getArrayBufferPointer(atomicCounter, args[0], status);

    U64 value = *((U64*)(atomicCounter->context->data + atomicCounter->counterOffset));
    if (!value) {
        n_setUndefined(result, status);
        return result;
    }
    value--;
    n_newU64(result, value);
    return result;
}

function(next) {
    napi_status status;
    var(result);

    n_getArguments(args, argsCount, 1, status);

    AtomicCounter* atomicCounter;
    n_getArrayBufferPointer(atomicCounter, args[0], status);

    U64 value = atomic_fetch_add((U64*)(atomicCounter->context->data + atomicCounter->counterOffset), 1);
    n_newU64(result, value);
    return result;

}

function(CreateObject) {

    napi_status status;

    n_objCreate(obj, status);

    var(methodFunction);
    n_objAssignFunction(obj, methodFunction, openCounters, status);
    n_objAssignFunction(obj, methodFunction, closeCounters, status);
    n_objAssignFunction(obj, methodFunction, prepare, status);
    n_objAssignFunction(obj, methodFunction, release, status);
    n_objAssignFunction(obj, methodFunction, current, status);
    n_objAssignFunction(obj, methodFunction, next, status);

    return obj;
}

NAPI_MODULE(atomicCounters, CreateObject)
