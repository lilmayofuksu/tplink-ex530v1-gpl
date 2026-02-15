#ifndef _XT_MARK_H
#define _XT_MARK_H

#include <linux/types.h>

struct xt_mark_tginfo2 {
	__u32 mark, mask;
};

struct xt_mark_mtinfo1 {
	__u32 mark, mask;
	__u8 invert;
};

/* add for LAN/WAN binding 20190926 --HSW */
/* Added By Shangguan Weijie, 2017-07-28. */
/* for LAN/WAN binding */
#ifdef CONFIG_TP_IMAGE
enum {
    XT_MARK_SET=0,
    XT_MARK_AND,
    XT_MARK_OR
};

struct xt_mark_target_info_v1 {
    __u32 mark;
    __u8 mode;
};
#endif
/* End Add. */

#endif /*_XT_MARK_H*/
