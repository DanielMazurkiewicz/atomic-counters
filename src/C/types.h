
enum e_AtomicCountersInternalError {
    NO_ERROR = 0,

    GET_SPACE_MSYNC = -29,
    GET_SPACE_FALLOCATE,
    GET_SPACE_MUNMAP,
    GET_SPACE_MMAP,

    CHECK_IF_RESIZED_FSTAT = -39,
    CHECK_IF_RESIZED_MSYNC,
    CHECK_IF_RESIZED_MUNMAP,
    CHECK_IF_RESIZED_MMAP,

    CLOSE_MSYNC = -49,
    CLOSE_MUNMAP,

    OPEN_PAGE_SIZE = -69,
    OPEN_WRONG_DB_TYPE,
    OPEN_OPEN,
    OPEN_FSTAT,
    OPEN_S_ISREG,
    OPEN_FALLOCATE,
    OPEN_FILE_SIZE,
    OPEN_MMAP,
    OPEN_HEADER_MAGIC,
    OPEN_HEADER_INITIALIZATION_TIMEOUT,
};
typedef enum e_AtomicCountersInternalError AtomicCountersInternalError;



typedef struct s_AtomicCountersHeader {
    U64 magic;
    U64 freeSpace;
    U32 initialized;
    U32 adding;
} AtomicCountersHeader;

typedef struct s_AtomicCounters {
    AtomicCountersHeader*       header;
    U64                         freeSpace;
    DATA                        data;
    S64                         dataSize;
    __off_t                     fileSize;
    S32                         fileGrowSize;
    S32                         timeout;

    int                         fd;
    int                         externalError;
    AtomicCountersInternalError internalError;
} AtomicCounters;


typedef struct s_AtomicCounter {
    U64                         counterOffset;
    AtomicCounters*             context;
} AtomicCounter;

typedef struct s_AtomicCounterInDb {
    U64                         counter;
    U64                         nameLength;
} AtomicCounterInDb;
