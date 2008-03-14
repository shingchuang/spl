#include <sys/sysmacros.h>
#include <sys/vmsystm.h>
#include <sys/vnode.h>
#include "config.h"

/*
 * Generic support
 */

int p0 = 0;
EXPORT_SYMBOL(p0);

char hw_serial[11];
EXPORT_SYMBOL(hw_serial);

vmem_t *zio_alloc_arena = NULL;
EXPORT_SYMBOL(zio_alloc_arena);

int
highbit(unsigned long i)
{
        register int h = 1;

        if (i == 0)
                return (0);
#if BITS_PER_LONG == 64
        if (i & 0xffffffff00000000ul) {
                h += 32; i >>= 32;
        }
#endif
        if (i & 0xffff0000) {
                h += 16; i >>= 16;
        }
        if (i & 0xff00) {
                h += 8; i >>= 8;
        }
        if (i & 0xf0) {
                h += 4; i >>= 4;
        }
        if (i & 0xc) {
                h += 2; i >>= 2;
        }
        if (i & 0x2) {
                h += 1;
        }
        return (h);
}
EXPORT_SYMBOL(highbit);

int
ddi_strtoul(const char *str, char **nptr, int base, unsigned long *result)
{
        char *end;
        return (*result = simple_strtoul(str, &end, base));
}
EXPORT_SYMBOL(ddi_strtoul);

static int __init spl_init(void)
{
	int rc;

	rc = vn_init();
	if (rc)
		return rc;

	strcpy(hw_serial, "007f0100"); /* loopback */
        printk(KERN_INFO "spl: Loaded Solaris Porting Layer v%s\n", VERSION);

	return 0;
}

static void spl_fini(void)
{
	vn_fini();
	return;
}

module_init(spl_init);
module_exit(spl_fini);

MODULE_AUTHOR("Lawrence Livermore National Labs");
MODULE_DESCRIPTION("Solaris Porting Layer");
MODULE_LICENSE("GPL");