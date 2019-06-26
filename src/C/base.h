
#define AtomicCountersHeader_magic 0x16a61cde3fcc0897ULL

AtomicCountersInternalError atomicCounters_isError(AtomicCounters* db) {
    AtomicCountersInternalError error = db->internalError;
    db->internalError = NO_ERROR;
    return error;
}

AtomicCounters* atomicCounters_open(char* fileName, S32 fileGrowSize, S32 timeout, S32 initializationTimeout) {
    AtomicCounters* db = (AtomicCounters*)malloc(sizeof(AtomicCounters));
    AtomicCountersHeader* header;

    __off_t fileSize;
    int pageSize;
    struct stat sb;

    if ( (pageSize = sysconf(_SC_PAGE_SIZE)) < 0) {
        db->internalError = OPEN_PAGE_SIZE;
        return db;
    }

    fileGrowSize = fileGrowSize / pageSize;
    fileGrowSize = (fileGrowSize + 1) * pageSize;

    // int fd = open(fileName, O_RDONLY, S_IRUSR | S_IWUSR);
    int fd = open(fileName, O_RDWR | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
    if (fd == -1) {
        //error - couldn't open/create file
        db->internalError = OPEN_FALLOCATE;
        return db;
    }
    db->fd = fd;

    if (fstat(fd, &sb) == -1) {
        // error - couldn't determine file properties
        db->internalError = OPEN_FSTAT;
        return db;
    }
    if (!S_ISREG(sb.st_mode)) {
        // error - is not a regular file
        db->internalError = OPEN_S_ISREG;
        return db;
    }


    if (sb.st_size == 0) {
        fileSize = fileGrowSize > sizeof(AtomicCountersHeader) ? fileGrowSize : sizeof(AtomicCountersHeader);
        if (posix_fallocate(fd, 0, fileSize) != 0) {
            // error - couldn't allocate disk space
            db->internalError = OPEN_FALLOCATE;
            return db;
        }
    } else {
        fileSize = sb.st_size;
        if (fileSize < sizeof(AtomicCountersHeader)) {
            // error - improper file or corrupted
            db->internalError = OPEN_FILE_SIZE;
            return db;
        }
    }

    if ((header = (AtomicCountersHeader*)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        // error - couldn't map a file
        db->internalError = OPEN_MMAP;
        return db;
    }

    db->header = header;

    if (sb.st_size == 0) {
        header->magic       = AtomicCountersHeader_magic;
        header->freeSpace   = 0;
        header->initialized = 2;
    } else {
        while (!header->initialized && initializationTimeout-- > 0) {
            usleep(1000); // wait 1ms
        }

        if (!header->initialized) {
            // error - initialization timeout
            header->initialized = 1;
            db->internalError = OPEN_HEADER_INITIALIZATION_TIMEOUT;
            return db;
        }
        if (header->magic != AtomicCountersHeader_magic) {
            // error - not a database file
            db->internalError = OPEN_HEADER_MAGIC;
            return db;
        }
        db->freeSpace = header->freeSpace;
    }

    db->dataSize        = fileSize - sizeof(AtomicCountersHeader);
    db->fileSize        = fileSize;
    db->fileGrowSize    = fileGrowSize;
    db->timeout         = timeout;
    db->data            = (DATA)header + sizeof(AtomicCountersHeader);
    db->internalError   = NO_ERROR;
    return db;
}

int atomicCounters_close(AtomicCounters* db) {
    int error = NO_ERROR;
    if (msync(db->header, db->fileSize, MS_ASYNC) != 0) {
        // error - couldn't flush data to a file
        error = CLOSE_MSYNC;
    }

    if (munmap(db->header, db->fileSize) != 0) {
        // error - couldn't unmap memory mapped file
        error = CLOSE_MUNMAP;
    }
    close(db->fd);
    free(db);
    return error;
}


