#ifndef PDEBUG_H
#define PDEBUG_H

#ifdef KISE_DEBUG
#define DEBUG_PRINT
#endif

#ifdef DEBUG_PRINT
#		ifdef __KERNEL__                /* This one if debugging is on, and kernel space */
#			define PDEBUG(fmt, args...) printk( KERN_DEBUG "Debug: " fmt, ## args)
# 		else                                      /* This one for user space */
#			define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
# 		endif
#else
# 		define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#endif //PDEBUG_H
