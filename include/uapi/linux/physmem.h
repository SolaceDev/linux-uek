#ifndef _LINUX_PHYSMEM_H
#define _LINUX_PHYSMEM_H
/*
  Copyright (C) 2006 Solace Systems

  This driver handles the mapping of physical memory that is reserved
  external to the linux kernel back into user space.

*/
// RCS id - DO NOT MODIFY
//
#ident "$Id: physmem.h 8 2007-02-26 21:39:34Z dgrama $"


// The amount of memory (bytes) we reserve at boot time for physical buffers
#define PHYSMEM_SIZE      (500*1024*1024)  // 500 MB
//#define PHYSMEM_SIZE      (2*1024*1024)  // 2 MB

// This is the structure that holds all information about this device
typedef struct {
    unsigned int totalMem;      // How many total bytes 
    unsigned int remainingMem;  // How many bytes remain in available mem  
    char         *physMem_p;     // Pointer to the allocated memory
    char         *currAlloc_p;   // Keeps track of where we are in the block
                                 // of mem we are allocating from
} physmem_dev_t;

// This is the structure that holds all information about this device
typedef struct {
    unsigned int  totalMem;      // How many total bytes
    unsigned int  remainingMem;  // How many bytes remain in available mem
    unsigned int  physMem_p;     // Pointer to the allocated memory
    unsigned int  currAlloc_p;   // Keeps track of where we are in the block
                                 // of mem we are allocating from
} physmem_dev_t32;


#ifdef __KERNEL__

//#define DRIVER_DEBUG 1
#ifdef DRIVER_DEBUG
#define _FDEBUG(__line, args...)  \
    printk(KERN_DEBUG "\nDEBUG " __FILE__ "<" #__line ">:" args)
#define FDEBUG(args...)  _FDEBUG(__LINE__, args)
#else
#define FDEBUG(args...) 
#endif

#define _FERROR(__line, args...)  \
    printk(KERN_ERR "\nERROR " __FILE__ "<" #__line ">:" args)
#define FERROR(args...) _FERROR(__LINE__, args)

#define _FWARN(__line, args...)   \
    printk(KERN_WARNING "\nWARN " __FILE__ "<" #__line ">:" args)
#define FWARN(args...)  _WARN(__LINE__, args)

#endif


// IOCTL data types
typedef union {

    // Used for request
    unsigned int  reqSize;     // How much memory to allocated

    // Used for result
    unsigned int   physAddr;    // Address of the allocated memory
    
} physmemAlloc_t;


// IOCTL defines - these are needed by the application and kernel module
#define PHYSMEM_IOCTL            's'
#define PHYSMEM_IOCTL_INIT       _IOW(PHYSMEM_IOCTL, 1, int)
#define PHYSMEM_IOCTL_STATUS     _IOR(PHYSMEM_IOCTL, 2, physmem_dev_t32)
#define PHYSMEM_IOCTL_ALLOC      _IOWR(PHYSMEM_IOCTL, 3, physmemAlloc_t)

#endif /* _LINUX_PHYSMEM_H */
