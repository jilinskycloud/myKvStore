#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> 
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
//#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/mutex.h>

////        Define device Vars
static int    majorNumber;
#define  DEVICE_NAME "myKvStore"                  /// The device will appear at /dev/mykvStore using this value
#define  CLASS_NAME  "myKvS"                      /// The device class -- this is a character device driver
static struct class*  kvStoreClass  = NULL;     /// The device-driver class struct pointer
static struct device* kvStoreDevice = NULL;     /// The device-driver device struct pointer

////        Semaphore Lock
//static struct rw_semaphore rwsema;
static DEFINE_MUTEX(mlock);
////        Likedlist KV Store 
struct llStore{
	struct list_head list;
	char *name;
	int rNo;
};
struct dataStruct{
	char *name;
	int rNo;
}userData;
LIST_HEAD(llHead);

////////////////////////  CUstom Defined Functions
////        Insert a Node
static void insertNode(struct dataStruct args, struct list_head *head)
{
	struct llStore *kvData;
    // Grabe Semaphore Lock
	//down_read(&rwsema);  
    mutex_lock(&mlock);          
	kvData = kmalloc(sizeof(struct llStore), GFP_KERNEL);
	kvData->name = args.name;
	kvData->rNo = args.rNo;
    printk("Name = %s and rollNo = %d ", kvData->name ,kvData->rNo);
	INIT_LIST_HEAD(&kvData->list);
	list_add(&kvData -> list, head);
    ssleep(1);
    // Release Semaphore Lock
	//up_read(&rwsema);
    mutex_unlock(&mlock);
}

////        Print all nodes in the linked list
static void readkv(struct list_head *head)
{
    struct list_head *loopOver;
	struct llStore *kvData;
    //down_read(&rwsema);
    mutex_lock(&mlock);  
	list_for_each(loopOver, head){
	kvData = list_entry(loopOver,struct llStore, list);
		printk("Name = %s and rollNo = %d ", kvData->name ,kvData->rNo);
	}
    //up_read(&rwsema);
    mutex_unlock(&mlock);
}


////        Find a node in the linked list.
struct dataStruct searchNode(struct dataStruct searchName, struct list_head *head)
{
	struct list_head *loopOver;
	struct llStore *kvData;
    struct dataStruct retKV;
    retKV.name = "NAN";
    retKV.rNo = -1;
    //printk("Searching it :: %s", searchName.name);
	list_for_each(loopOver, head){
	    kvData = list_entry(loopOver,struct llStore, list);
		if (kvData->name == searchName.name){
            retKV.name = kvData->name;
            retKV.rNo = kvData->rNo;
		    return retKV;
        }else{
            return retKV; 
        }
	}
	return retKV;
}

////        File Open
int etx_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Inside open\n");
    return 0;
}

////        File Release */
int etx_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Inside close\n");
    return 0;
}

//  IOCTL Definition
#define readkvStore _IO('r' , 1)
#define kvRead _IOR('r', 2, char *)
#define kvWrite _IOW('r', 3, struct kvStore *)
#define searchANode _IOWR('r', 4, struct dataStruct *)
static long etx_ioctl(struct file *file, unsigned int userCalls, unsigned long arg){
    switch(userCalls){
        case kvWrite:
            printk("Writing on device...!");
            copy_from_user(&userData, (struct dataStruct *) arg, sizeof(userData));
            insertNode(userData, &llHead);
            break;
        case searchANode:
			printk("Reading from device...!");
            struct dataStruct search1;
            struct dataStruct rNo;
            copy_from_user(&search1, (struct dataStruct *) arg, sizeof(search1));
            rNo = searchNode(search1, &llHead);
		    if(copy_to_user((struct dataStruct *) arg, &rNo, sizeof(rNo)))
				printk("error copying data to user :: %s\n", rNo.name);

            break;
        case readkvStore: //Display full linked-list
            printk("This is Display!....................................................."); 
			readkv(&llHead);
        default:
            printk("Switch statement Default. ::  %d", userCalls);
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////
// File Operations
static struct file_operations fops = {
    .owner              = THIS_MODULE,
    .open               = etx_open,
    .unlocked_ioctl     = etx_ioctl,
    .release            = etx_release,
};

// Init the Driver
int rw_sem_init(void)
{
    printk(KERN_INFO "=========================== Initializing the kvStore LKM  Time :: %lld=============================\n", ktime_get());
    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if(majorNumber<0){
        printk(KERN_ALERT "kvStore failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "kvStore: registered correctly with major number %d\n", majorNumber);
    // Register the device class
    kvStoreClass = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(kvStoreClass)){                // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(kvStoreClass);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "kvStore: device class registered correctly\n");
    // Register the device driver
    kvStoreDevice = device_create(kvStoreClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(kvStoreDevice)){               // Clean up if there is an error
        class_destroy(kvStoreClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(kvStoreDevice);
    }
    printk(KERN_INFO "kvStore: device class created correctly\n"); // Made it! device was initialized
    //init_rwsem(&rwsema);

    return 0;
}
// Exit the Driver
void rw_sem_cleanup(void)
{
   device_destroy(kvStoreClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(kvStoreClass);                          // unregister the device class
   class_destroy(kvStoreClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "myKvStore: is unloaded!\n");
}

// LKM Entry Point
module_init(rw_sem_init);
module_exit(rw_sem_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Syed Umer");
MODULE_DESCRIPTION("KV Store LKM");
MODULE_VERSION("0.01");