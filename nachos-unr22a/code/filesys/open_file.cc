/// Routines to manage an open Nachos file.  As in UNIX, a file must be open
/// before we can read or write to it.  Once we are all done, we can close it
/// (in Nachos, by deleting the `OpenFile` data structure).
///
/// Also as in UNIX, for convenience, we keep the file header in memory while
/// the file is open.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "open_file.hh"
#include "file_header.hh"
#include "threads/system.hh"

#include <string.h>

#ifdef FILESYS
int findFileID(unsigned sector){
	int i = 0;
	while(opennedFilesTable->HasKey(i)){
		if (opennedFilesTable->Get(i)->sector == sector){
			return i;
        }
		i++;
	}
	return -1;
}
#endif


/// Open a Nachos file for reading and writing.  Bring the file header into
/// memory while the file is open.
///
/// * `sector` is the location on disk of the file header for this file.
#ifdef FILESYS
OpenFile::OpenFile(int sector, const char *name) {
    id = findFileID(sector);
    if(id == -1){
        OpenFileEntry *entry = new OpenFileEntry(name, (unsigned int)sector);
        id = opennedFilesTable->Add(entry);
    } else {
        opennedFilesTable->Get(id)->openFileCount++;
    }
    hdr = new FileHeader;
    hdr->FetchFrom(sector);
    seekPosition = 0;       
}
#else
OpenFile::OpenFile(int sector) {
    hdr = new FileHeader;
    hdr->FetchFrom(sector);
    seekPosition = 0;     
}
#endif


/// Close a Nachos file, de-allocating any in-memory data structures.
OpenFile::~OpenFile() {
    #ifdef FILESYS
        opennedFilesTable->Get(id)->openFileCount--;
        OpenFileEntry *entry = opennedFilesTable->Get(id);
        if (entry->openFileCount == 0 && entry->mustRemove) {
            fileSystem->removeFile(entry->name);
            opennedFilesTable->Remove(id);
        } else if (entry->openFileCount == 0 && !entry->mustRemove) {
            opennedFilesTable->Remove(id);
        }
        delete entry;
    #endif
    delete hdr;
}

/// Change the current location within the open file -- the point at which
/// the next `Read` or `Write` will start from.
///
/// * `position` is the location within the file for the next `Read`/`Write`.
void
OpenFile::Seek(unsigned position)
{
    seekPosition = position;
}

/// OpenFile::Read/Write
///
/// Read/write a portion of a file, starting from `seekPosition`.  Return the
/// number of bytes actually written or read, and as a side effect, increment
/// the current position within the file.
///
/// Implemented using the more primitive `ReadAt`/`WriteAt`.
///
/// * `into` is the buffer to contain the data to be read from disk.
/// * `from` is the buffer containing the data to be written to disk.
/// * `numBytes` is the number of bytes to transfer.

int
OpenFile::Read(char *into, unsigned numBytes)
{
    ASSERT(into != nullptr);
    ASSERT(numBytes > 0);

    int result = ReadAt(into, numBytes, seekPosition);
    seekPosition += result;
    return result;
}

int OpenFile::Write(const char *into, unsigned numBytes) {
    ASSERT(into != nullptr);
    ASSERT(numBytes > 0);

    #ifdef FILESYS
	if (seekPosition + numBytes > Length()){
        // necesita expandir
		int sizeToExpand = numBytes - (Length() - seekPosition);
		if(!fileSystem->expand(hdr, sizeToExpand, id)){
            numBytes = Length() - seekPosition; 
        }
	}
	ASSERT(seekPosition + numBytes <= Length());
    #endif


    int result = WriteAt(into, numBytes, seekPosition);
    seekPosition += result;
    return result;
}

/// OpenFile::ReadAt/WriteAt
///
/// Read/write a portion of a file, starting at `position`.  Return the
/// number of bytes actually written or read, but has no side effects (except
/// that `Write` modifies the file, of course).
///
/// There is no guarantee the request starts or ends on an even disk sector
/// boundary; however the disk only knows how to read/write a whole disk
/// sector at a time.  Thus:
///
/// For ReadAt:
///     We read in all of the full or partial sectors that are part of the
///     request, but we only copy the part we are interested in.
/// For WriteAt:
///     We must first read in any sectors that will be partially written, so
///     that we do not overwrite the unmodified portion.  We then copy in the
///     data that will be modified, and write back all the full or partial
///     sectors that are part of the request.
///
/// * `into` is the buffer to contain the data to be read from disk.
/// * `from` is the buffer containing the data to be written to disk.
/// * `numBytes` is the number of bytes to transfer.
/// * `position` is the offset within the file of the first byte to be
///   read/written.

#ifdef FILESYS
int OpenFile::ReadAt(char *into, unsigned numBytes, unsigned position, bool isGettingWritten) {
    ASSERT(into != nullptr);
    ASSERT(numBytes > 0);

	DEBUG('f',"Intentando leer en el archivo.\n");
	if (!isGettingWritten)
		opennedFilesTable->Get(id)->fileSystemLock->addReader();
	DEBUG('f',"Leyendo en el archivo.\n");

    int ret = ReadAt(into, numBytes, position);

    DEBUG('f',"Terminando de leer en el archivo.\n");
    if (!isGettingWritten)
		opennedFilesTable->Get(id)->fileSystemLock->removeReader();
    DEBUG('f',"Se termino de leer en el archivo.\n");
    return ret;
}
#endif

int OpenFile::WriteAt(const char *from, unsigned numBytes, unsigned position) {
    ASSERT(from != nullptr);
    ASSERT(numBytes > 0);

    #ifdef FILESYS
	DEBUG('f',"Intentando escribir en el archivo.\n");
	opennedFilesTable->Get(id)->fileSystemLock->addWriter();
	DEBUG('f',"Escribiendo en el archivo.\n");
    #endif

    ASSERT(from != nullptr);
    ASSERT(numBytes > 0);

    unsigned fileLength = hdr->FileLength();
    unsigned firstSector, lastSector, numSectors;
    bool firstAligned, lastAligned;
    char *buf;

    if (position >= fileLength) {
        return 0;  // Check request.
    }
    if (position + numBytes > fileLength) {
        numBytes = fileLength - position;
    }
    DEBUG('f', "Writing %u bytes at %u, from file of length %u.\n",
          numBytes, position, fileLength);

    firstSector = DivRoundDown(position, SECTOR_SIZE);
    lastSector  = DivRoundDown(position + numBytes - 1, SECTOR_SIZE);
    numSectors  = 1 + lastSector - firstSector;

    buf = new char [numSectors * SECTOR_SIZE];

    firstAligned = position == firstSector * SECTOR_SIZE;
    lastAligned  = position + numBytes == (lastSector + 1) * SECTOR_SIZE;

    // Read in first and last sector, if they are to be partially modified.
    if (!firstAligned) {
        #ifdef FILESYS
        ReadAt(buf, SECTOR_SIZE, firstSector * SECTOR_SIZE, true);
        #else
        ReadAt(buf, SECTOR_SIZE, firstSector * SECTOR_SIZE);
        #endif
    }
    if (!lastAligned && (firstSector != lastSector || firstAligned)) {
        #ifdef FILESYS
        ReadAt(&buf[(lastSector - firstSector) * SECTOR_SIZE], SECTOR_SIZE, lastSector * SECTOR_SIZE, true);
        #else
        ReadAt(&buf[(lastSector - firstSector) * SECTOR_SIZE], SECTOR_SIZE, lastSector * SECTOR_SIZE);
        #endif
    }

    // Copy in the bytes we want to change.
    memcpy(&buf[position - firstSector * SECTOR_SIZE], from, numBytes);

    // Write modified sectors back.
    for (unsigned i = firstSector; i <= lastSector; i++) {
        synchDisk->WriteSector(hdr->ByteToSector(i * SECTOR_SIZE),
                               &buf[(i - firstSector) * SECTOR_SIZE]);
    }

    #ifdef FILESYS
    DEBUG('f',"Terminando de leer en el archivo.\n");
    opennedFilesTable->Get(id)->fileSystemLock->removeReader();
    DEBUG('f',"Se termino de leer en el archivo.\n");
    #endif

    delete [] buf;
    return numBytes;
}


int OpenFile::ReadAt(char *into, unsigned numBytes, unsigned position) {
    ASSERT(into != nullptr);
    ASSERT(numBytes > 0);

    unsigned fileLength = hdr->FileLength();
    unsigned firstSector, lastSector, numSectors;
    char *buf;

    if (position >= fileLength) {
        return 0;  // Check request.
    }
    if (position + numBytes > fileLength) {
        numBytes = fileLength - position;
    }
    DEBUG('f', "Reading %u bytes at %u, from file of length %u.\n",
          numBytes, position, fileLength);

    firstSector = DivRoundDown(position, SECTOR_SIZE);
    lastSector = DivRoundDown(position + numBytes - 1, SECTOR_SIZE);
    numSectors = 1 + lastSector - firstSector;

    // Read in all the full and partial sectors that we need.
    buf = new char [numSectors * SECTOR_SIZE];
    for (unsigned i = firstSector; i <= lastSector; i++) {
        synchDisk->ReadSector(hdr->ByteToSector(i * SECTOR_SIZE),
                              &buf[(i - firstSector) * SECTOR_SIZE]);
    }

    // Copy the part we want.
    memcpy(into, &buf[position - firstSector * SECTOR_SIZE], numBytes);
    delete [] buf;
    return numBytes;
}


/// Return the number of bytes in the file.
unsigned
OpenFile::Length() const
{
    return hdr->FileLength();
}
