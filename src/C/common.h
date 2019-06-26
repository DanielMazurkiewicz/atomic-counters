
S64 atomicCounters_get_space(AtomicCounters* db, U64 sizeOfSpace) {
    S64 offset = db->header->freeSpace;
    db->freeSpace = db->header->freeSpace + sizeOfSpace;
    if (db->freeSpace > db->dataSize) {
        __off_t fileSizeNew = db->freeSpace / db->fileGrowSize;
        fileSizeNew = (fileSizeNew + 1) * db->fileGrowSize;
        if (msync(db->header, db->fileSize, MS_ASYNC) != 0) {
            // error - couldn't flush data to a file
            return db->internalError = GET_SPACE_MSYNC;
        }
        if (posix_fallocate(db->fd, db->fileSize, fileSizeNew - db->fileSize) != 0) {
            // error - couldn't allocate space for database
            return db->internalError = GET_SPACE_FALLOCATE;
        }
        if (munmap(db->header, db->fileSize) != 0) {
            // error - couldn't unmap memory mapped file
            return db->internalError = GET_SPACE_MUNMAP;
        }
        if ((db->header = (AtomicCountersHeader*)mmap(NULL, fileSizeNew, PROT_READ | PROT_WRITE, MAP_SHARED, db->fd, 0)) == MAP_FAILED) {
            // error - couldn't map a file
            return db->internalError = GET_SPACE_MMAP;
        }
        db->fileSize = fileSizeNew;
        db->dataSize = fileSizeNew - sizeof(AtomicCountersHeader);
        db->data = (DATA)((DATA)db->header + sizeof(AtomicCountersHeader));
    }
    return offset;
}

S64 atomicCounters_close_space(AtomicCounters* db) {
    db->header->freeSpace = db->freeSpace;
}




S64 atomicCounters_check_if_file_resized(AtomicCounters* db) {
    db->freeSpace = db->header->freeSpace;
    if (db->freeSpace > db->dataSize) {
        struct stat sb;
        if (fstat(db->fd, &sb) == -1) {
            // error - couldn't determine file properties
            return db->internalError = CHECK_IF_RESIZED_FSTAT;
        }
        if (msync(db->header, db->fileSize, MS_ASYNC) != 0) {
            // error - couldn't flush data to a file
            return db->internalError = CHECK_IF_RESIZED_MSYNC;
        }
        if (munmap(db->header, db->fileSize) != 0) {
            // error - couldn't unmap memory mapped file
            return db->internalError = CHECK_IF_RESIZED_MUNMAP;
        }
        if ((db->header = (AtomicCountersHeader*)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, db->fd, 0)) == MAP_FAILED) {
            // error - couldn't map a file
            return db->internalError = CHECK_IF_RESIZED_MMAP;
        }
        db->fileSize = sb.st_size;
        db->dataSize = sb.st_size - sizeof(AtomicCountersHeader);
        db->data = (DATA)((DATA)db->header + sizeof(AtomicCountersHeader));

    }
    return NO_ERROR;
}



