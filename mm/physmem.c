/*
  Copyright (C) 2006 Solace Systems

  This driver handles the mapping of physical memory that is reserved
  external to the linux kernel back into user space.

*/
// RCS id - DO NOT MODIFY
//
#ident "$Id: physmem.c 17 2007-03-29 21:24:20Z dgrama $"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <asm/ioctls.h>
#include <linux/compat.h>

#include <linux/physmem.h>
#include <linux/memblock.h>
#include <linux/seq_file.h>

#define dprintk(x...)	do { } while (0)

#define MODULE_NAME "physmem"


#ifdef CONFIG_PROC_FS

#include <linux/proc_fs.h>

#define PROCFS_NAME             "physmem"

static struct proc_dir_entry * proc_solace_dir;

#endif


// Memory to hold the state of the driver
static physmem_dev_t physmemDevInfo;

// Memory for the /dev/physmem file
static struct class        *physmem_class;
static int                  physmem_majorNum;

static DEFINE_SPINLOCK(physmem_lock);

static struct resource mem_resource = { 0, 0, "physmem",
                                        IORESOURCE_MEM|IORESOURCE_BUSY };


// open - called when a user app opens this driver
static int physmem_open (
    struct inode *inode,
    struct file *file)
{

    FDEBUG("Opening...");

    return 0;
    
}


// release - called when a user app closes this driver or
//           when that app finishes.
//
// It should be noted that we don't free any allocated memory
// when the driver is released.  This prevents problems where
// allocated memory could still be in use by various devices
// outside the scope of that user app and if we freed it 
// immediately, we would have two owners of the same memory.
//
// Freeing memory must be done through the init IOCTL call.
//
static int physmem_release (
    struct inode *inode,
    struct file *file)
{

    FDEBUG("Releasing...");

    return 0;
    
}


// This function will take care of the carving up of the
// large physmem area into a smaller chunk.  The pointer
// to that chunk is returned to the user, who should then
// mmap it.
//
// NOTE that if we can't allocate all the memory
// requested, then we won't allocate any.
//
// NOTE that we will always allocate memory in multiples
//      of a page.
static int physmem_ioctl_alloc_mem(
    physmemAlloc_t *allocReq_p
)
{
    unsigned int pageAlignedSize;

    if ((allocReq_p->reqSize & (~(PAGE_MASK))) != 0) {
        pageAlignedSize = ((allocReq_p->reqSize & PAGE_MASK)
                           + PAGE_SIZE);
    }
    else {
        pageAlignedSize = allocReq_p->reqSize;
    }

    printk(KERN_INFO "Allocating physmem of real size %d.  "
            "Requested size is %d, pid %i(%s)\n",
            (int)pageAlignedSize, (int)allocReq_p->reqSize, current->pid, 
	    current->comm);
        
    spin_lock(&physmem_lock);
    
    dprintk(KERN_DEBUG "before: currAlloc_p = %p, remainingMem = 0x%08x\n",
	   physmemDevInfo.currAlloc_p, physmemDevInfo.remainingMem);

    if (pageAlignedSize > physmemDevInfo.remainingMem) {
	dprintk("ERROR: pageAlignedSize(0x%08x) > remainingMem(0x%08x)\n",
		pageAlignedSize, physmemDevInfo.remainingMem);
        // Can't allocate the memory
        spin_unlock(&physmem_lock);
        return -ENOMEM;
    }

    allocReq_p->physAddr =
        (unsigned long)physmemDevInfo.currAlloc_p;

    physmemDevInfo.currAlloc_p  += pageAlignedSize;
    physmemDevInfo.remainingMem -= pageAlignedSize;

    dprintk(KERN_DEBUG "after: physAddr = 0x%08x pageAlignedSize = 0x%08x\n"
	   "currAlloc_p = %p remainingMem = 0x%08x\n",
	   allocReq_p->physAddr, pageAlignedSize, physmemDevInfo.currAlloc_p,
	   physmemDevInfo.remainingMem);

    spin_unlock(&physmem_lock);

    return 0;

}


// Reinitialize the physmem - this won't change the allocation of
// the bootmem itself, but it will consider all of that memory
// to be available for allocation again.
//
// NOTE:  when this is called, the caller must ensure that no
//        previously allocated memory is still in use ANYWHERE
static int physmem_ioctl_init(void)
{
    
    physmemDevInfo.currAlloc_p  = physmemDevInfo.physMem_p;
    physmemDevInfo.remainingMem = physmemDevInfo.totalMem;

    return 0;
}

// ioctl - generic interface from the user app into the driver
static long physmem_ioctl(
    struct file  * file,
    unsigned int   cmd,
    unsigned long  arg)
{
    int rc;
    physmemAlloc_t allocReq;

    switch (cmd) {
        
    case PHYSMEM_IOCTL_INIT:
        return physmem_ioctl_init();
        break;
        
    case PHYSMEM_IOCTL_STATUS:
       if (copy_to_user((void*)arg, &physmemDevInfo, sizeof(physmemDevInfo))) {
           return -EFAULT;
       }
       break;
        
    case PHYSMEM_IOCTL_ALLOC:
        if (copy_from_user(&allocReq, (void*)arg, sizeof(allocReq))) {
            return -EFAULT;
        }
        
        rc = physmem_ioctl_alloc_mem(&allocReq);
        
        if (rc == 0) {
            if (copy_to_user((void*)arg, &allocReq, sizeof(allocReq))) {
                return -EFAULT;
            }
        }
       
        return rc;
        
        break;

    case TCGETS:
        return  -ENOTTY;
        break;
        
    default:
        FERROR("Invalid IOCTL code: %d\n",
               cmd);
        return -EINVAL;
        break;
    }

    return 0;
}

#ifdef CONFIG_COMPAT
// ioctl - generic interface from the 32bit user app into the driver
static long physmem_ioctl_compat(
    struct file  * file,
    unsigned int   cmd,
    unsigned long  arg)
{
    int rc;
    physmemAlloc_t allocReq;
    physmem_dev_t32 status;

    switch (cmd) {
        
    case PHYSMEM_IOCTL_INIT:
        return physmem_ioctl_init();
        break;
        
    case PHYSMEM_IOCTL_STATUS:
       status.totalMem = physmemDevInfo.totalMem;
       status.remainingMem = physmemDevInfo.remainingMem;
       status.physMem_p = ptr_to_compat(physmemDevInfo.physMem_p);
       status.currAlloc_p = ptr_to_compat(physmemDevInfo.currAlloc_p);
       if (copy_to_user((__user void*)arg, &status, sizeof(status))) {
           return -EFAULT;
       }
       break;
        
    case PHYSMEM_IOCTL_ALLOC:
        if (copy_from_user(&allocReq, (__user void*)arg, sizeof(allocReq))) {
            return -EFAULT;
        }
        
        rc = physmem_ioctl_alloc_mem((physmemAlloc_t *)&allocReq);
        
        if (rc == 0) {
            if (copy_to_user((void*)arg, &allocReq, sizeof(allocReq))) {
                return -EFAULT;
            }
        }
       
        return rc;
        
        break;

    case TCGETS:
        return  -ENOTTY;
        break;
        
    default:
        FERROR("Invalid compat IOCTL code: %d\n",
               cmd);
        return -EINVAL;
        break;
    }

    return 0;
}
#endif


// mmap - allow the user app to get a virtual address to the memory
static int physmem_mmap(
    struct file           * file,
    struct vm_area_struct * vma)
{
    int rc;

    // Disallow swapping (safety - the pages should have already been
    // marked reserved during allocation)
    vma->vm_flags |= VM_SPECIAL;

    rc = remap_pfn_range(vma,
                         vma->vm_start,
                         vma->vm_pgoff,
                         vma->vm_end-vma->vm_start,
                         vma->vm_page_prot);
    
    if (rc) {
        dprintk(KERN_ERR "mmap failed with error %d\n", rc);
        return -EAGAIN;
    }
    
    return 0;
    
}

typedef unsigned long compat_ioctl_t; 

#ifdef CONFIG_PROC_FS
// physmem_procfs_show - will output the /proc info
static int physmem_procfs_show(struct seq_file *seq, void *data)
{
    spin_lock(&physmem_lock);

    seq_printf(seq, "\nSolace Physical Memory Status:\n\n");
    seq_printf(seq, "Starting Address:   %p\n",
             physmemDevInfo.physMem_p);
    seq_printf(seq, "Total Mem:          %d\n",
             (int)physmemDevInfo.totalMem);
    seq_printf(seq, "Allocated Mem:      %d\n",
             (int)(physmemDevInfo.totalMem - physmemDevInfo.remainingMem));
    seq_printf(seq, "Available Mem:      %d\n",
             (int)physmemDevInfo.remainingMem);
    seq_printf(seq, "Current Alloc Addr: %p\n",
             physmemDevInfo.currAlloc_p);
    seq_printf(seq, "\nIOCTL Codes:\n\n");
    seq_printf(seq, "PHYSMEM_IOCTL_INIT:    0x%lx\n",
             (compat_ioctl_t)PHYSMEM_IOCTL_INIT);
    seq_printf(seq, "PHYSMEM_IOCTL_STATUS:  0x%lx\n",
             (compat_ioctl_t)PHYSMEM_IOCTL_STATUS);
    seq_printf(seq, "PHYSMEM_IOCTL_ALLOC:   0x%lx\n",
             (compat_ioctl_t)PHYSMEM_IOCTL_ALLOC);
    seq_printf(seq, "\n");

    spin_unlock(&physmem_lock);

    return 0;

} /* procfile_read */ 

static int physmem_procfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, physmem_procfs_show, NULL);
}

static const struct proc_ops physmem_procfs_read_fops = {
	.proc_open		= physmem_procfs_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	        = seq_release,
};
#endif



// define which file operations are supported
struct file_operations physmem_fops = {
    .owner     = THIS_MODULE,          
    .unlocked_ioctl = physmem_ioctl,       
#ifdef CONFIG_COMPAT
    .compat_ioctl = physmem_ioctl_compat,
#endif
    .mmap      = physmem_mmap,         
    .open      = physmem_open,         
    .release   = physmem_release,   
};




/*
 * call when 'physmem=' is given on the commandline.
 *
 * Strangely, bootmem is still active during this call, but
 * during the processing of the initcalls it isn't anymore!
 * So we alloc the needed memory here instead of bigphysarea_init().
 */
static int have_initialized = 0;
static unsigned long physmem_size = 0;

int boot_physmem_init(void);
//static
int __init physmem_setup(char *str)
{
    dprintk(KERN_ALERT "physmem_setup()\n");
    physmem_size = memparse(str, &str);
    dprintk(KERN_ALERT "physmem_setup(): size=%lu\n", physmem_size);
    return 0;
}

early_param("physmem", physmem_setup);

int __init_memblock boot_physmem_init(void)
{
	unsigned long long base;
	unsigned size = physmem_size;
	int rc;

	if (have_initialized)
		return 0;

	dprintk(KERN_DEBUG "boot_physmem_init()\n");

        // Initialize our device info
	printk(KERN_DEBUG "Allocating boot mem of size: %lu\n", physmem_size);

	if (!physmem_size) {
		printk(KERN_WARNING "Not initializing physmem since no "
                                    "physmem requested on command line\n");
		return 0;
	}

	if (physmem_size & (PAGE_SIZE-1))
		physmem_size += PAGE_SIZE - (physmem_size & (PAGE_SIZE-1));


	// Align the memory to a full page so that we can make sure we can mmap it
	base = memblock_find_in_range(PAGE_SIZE, 0x7fff0000, physmem_size, PAGE_SIZE);
	if (base == 0) {
		FERROR("alloc_bootmem failed for size %u\n", (unsigned)size);
		return -ENOMEM;
	}

	dprintk(KERN_NOTICE "physmem: reserving 0x%08lx bytes at 0x%08Lx\n", 
		physmem_size, base);

	memblock_reserve(base, physmem_size);

	physmemDevInfo.physMem_p = phys_to_virt(base);
        physmemDevInfo.remainingMem      = size;
        physmemDevInfo.totalMem          = size;

        dprintk(KERN_NOTICE "Allocated boot mem at address: %p\n", physmemDevInfo.physMem_p);

	have_initialized = 1;
    
        // Just keep the physical address
        physmemDevInfo.physMem_p   = (void*)(unsigned long)virt_to_phys(physmemDevInfo.physMem_p);	// FIXME: 64bit correctness?
        physmemDevInfo.currAlloc_p = physmemDevInfo.physMem_p;

        // Register it as a resource
        mem_resource.start = base;
        mem_resource.end = mem_resource.start + physmem_size -1;
        rc = insert_resource(&iomem_resource, &mem_resource);
        if (rc < 0) {
            printk(KERN_ERR "Failed IOMEM resource request with result %d\n", 
			     rc);
        }

	return rc;
}




// initialize module
static int __init physmem_init (void) {
    int rc = 0;

    FDEBUG("initializing module\n");

    if (!physmemDevInfo.physMem_p) {
        FERROR("Boot mem was not allocated, so the physmem module"
               " is not being registered.\n");
        return -EIO;
    }

    physmem_majorNum = register_chrdev (0, MODULE_NAME, &physmem_fops);
    if (physmem_majorNum < 0) {
        FERROR("Failed to register the device.  Errno: %d\n", rc);
        rc = -EIO;
        goto out0;
    }

    physmem_class = class_create(THIS_MODULE, "phys");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    device_create(physmem_class, NULL, MKDEV(physmem_majorNum, 0), NULL, MODULE_NAME);
#else
    class_device_create(physmem_class, NULL, MKDEV(physmem_majorNum, 0), NULL, MODULE_NAME);
#endif
#ifdef CONFIG_DEVFS_FS    
    devfs_mk_cdev(MKDEV(physmem_majorNum, 0),
                  S_IFCHR | S_IRUSR | S_IWUSR,
                  MODULE_NAME);
#endif
    

#ifdef CONFIG_PROC_FS
 
    /* Create the /proc/solace/3200 file - note that proc_root is "/proc" */
    proc_create(PROCFS_NAME, S_IFREG | S_IRUGO, NULL/*proc_root*/,
                &physmem_procfs_read_fops);
    
#endif
    
    return 0;

    unregister_chrdev (0, MODULE_NAME);
    
  out0:
    return rc;
}

// close and cleanup module
static void __exit
physmem_exit(void) {
    
    FDEBUG("cleaning up module\n");

#ifdef CONFIG_PROC_FS
    // Remove the /proc file 
    remove_proc_entry(PROCFS_NAME, proc_solace_dir);
#endif

    // Remove the dev file
#ifdef CONFIG_DEVFS_FS    
    devfs_remove(MODULE_NAME, 0);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    device_destroy(physmem_class, MKDEV(physmem_majorNum, 0));
#else
    class_device_destroy(physmem_class, MKDEV(physmem_majorNum, 0));
#endif
    class_destroy(physmem_class);
    
    // Unregister the device
    unregister_chrdev (0, MODULE_NAME);
    
}


// // Dirty way of getting a call before the memory system is up and running
// console_initcall(physmem_earlyinit);

// Normal module init/exit
module_init(physmem_init);
module_exit(physmem_exit);

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Edward Funnekotter <edward.funnekotter@solacesystems.com");
MODULE_DESCRIPTION("Solace System bootmem allocator");
