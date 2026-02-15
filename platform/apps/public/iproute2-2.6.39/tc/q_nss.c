/*
 **************************************************************************
 * Copyright (c) 2015, 2018 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "tc_util.h"

typedef int bool;
#define true 1
#define false 0

/* ======================== NSSFIFO =======================*/

static void nssfifo_explain(void)
{
	fprintf(stderr, "Usage: ...  nsspfifo [ limit PACKETS ] [ set_default ] [ accel_mode ]\n");
}

static int nssfifo_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	struct rtattr *tail;
	struct tc_nssfifo_qopt opt;
	bool accel_mode = false;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(&opt.limit, *argv) || opt.limit == 0) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "set_default") == 0) {
			opt.set_default = 1;
		} else if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (strcmp(*argv, "help") == 0) {
			nssfifo_explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nssfifo_explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_PPE;
	} else if (opt.accel_mode >= TCA_NSS_ACCEL_MODE_MAX) {
		fprintf(stderr, "accel_mode should be < %d\n", TCA_NSS_ACCEL_MODE_MAX);
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSFIFO_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nssfifo_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSFIFO_MAX + 1];
	struct tc_nssfifo_qopt *qopt;
	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSFIFO_MAX, opt);

	if (tb[TCA_NSSFIFO_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSFIFO_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSFIFO_PARMS]);

	if (strcmp(qu->id, "nssbfifo") == 0)
		fprintf(f, "limit %s ", sprint_size(qopt->limit, b1));
	else
		fprintf(f, "limit %up ", qopt->limit);

	if (qopt->set_default)
		fprintf(f, "set_default ");

	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

struct qdisc_util nsspfifo_qdisc_util = {
	.id		= "nsspfifo",
	.parse_qopt	= nssfifo_parse_opt,
	.print_qopt	= nssfifo_print_opt,
};

struct qdisc_util nssbfifo_qdisc_util = {
	.id		= "nssbfifo",
	.parse_qopt	= nssfifo_parse_opt,
	.print_qopt	= nssfifo_print_opt,
};

/* ======================== NSSFQ_CODEL =======================*/

static void nssfq_codel_explain(void)
{
	fprintf(stderr, "Usage: ... nssfq_codel target TIME interval TIME [ flows NUMBER ] [ quantum BYTES ]"
				"[ limit PACKETS ] [ set_default ] [ accel_mode ]\n");
}

static void nssfq_codel_explain_err1(void)
{
	fprintf(stderr, "Value of target and interval should be greater than 1ms\n");
}

static int nssfq_codel_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	struct rtattr *tail;
	struct tc_nsscodel_qopt opt;
	bool accel_mode = false;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "target") == 0) {
			NEXT_ARG();
			if (get_time(&opt.target, *argv)) {
				fprintf(stderr, "Illegal \"target\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(&opt.limit, *argv) || opt.limit == 0) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "flows") == 0) {
			NEXT_ARG();
			if (get_size(&opt.flows, *argv) || opt.flows == 0) {
				fprintf(stderr, "Illegal \"flows\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (get_size(&opt.quantum, *argv) || opt.quantum == 0) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "interval") == 0) {
			NEXT_ARG();
			if (get_time(&opt.interval, *argv)) {
				fprintf(stderr, "Illegal \"interval\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "ecn") == 0) {
			fprintf(stderr, "Illegal, ECN not supported\n");
			nssfq_codel_explain();
			return -1;
		} else if (strcmp(*argv, "set_default") == 0) {
			opt.set_default = 1;
		} else if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (strcmp(*argv, "help") == 0) {
			nssfq_codel_explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nssfq_codel_explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_NSS_FW;
	} else if (opt.accel_mode != TCA_NSS_ACCEL_MODE_NSS_FW) {
		fprintf(stderr, "accel_mode should be %d\n", TCA_NSS_ACCEL_MODE_NSS_FW);
		return -1;
	}

	if (!opt.target || !opt.interval) {
		nssfq_codel_explain();
		return -1;
	}

	if (opt.target < 1000 || opt.interval < 1000) {
		nssfq_codel_explain_err1();
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSCODEL_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nssfq_codel_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSCODEL_MAX + 1];
	struct tc_nsscodel_qopt *qopt;
	SPRINT_BUF(b1);
	SPRINT_BUF(b2);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSCODEL_MAX, opt);

	if (tb[TCA_NSSCODEL_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSCODEL_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSCODEL_PARMS]);

	fprintf(f, "target %s limit %up interval %s flows %u quantum %u ",
		sprint_time(qopt->target, b1),
		qopt->limit,
		sprint_time(qopt->interval, b2),
		qopt->flows,
		qopt->quantum);

	if (qopt->ecn)
		fprintf(f, "ecn ");

	if (qopt->set_default)
		fprintf(f, "set_default ");

	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

static int nssfq_codel_print_xstats(struct qdisc_util *qu, FILE *f, struct rtattr *xstats)
{
	struct tc_nssfq_codel_xstats *st;

	if (xstats == NULL)
		return 0;

	if (RTA_PAYLOAD(xstats) < sizeof(*st))
		return -1;

	st = RTA_DATA(xstats);
	fprintf(f, " maxpacket %u drop_overlimit %u new_flow_count %u ecn_mark %u\n",
			st->maxpacket, st->drop_overlimit, st->new_flow_count, st->ecn_mark);
	fprintf(f, " new_flows_len %u old_flows_len %u", st->new_flows_len, st->old_flows_len);

	return 0;
}

struct qdisc_util nssfq_codel_qdisc_util = {
	.id		= "nssfq_codel",
	.parse_qopt	= nssfq_codel_parse_opt,
	.print_qopt	= nssfq_codel_print_opt,
	.print_xstats	= nssfq_codel_print_xstats,
};

/* ======================== NSSCODEL =======================*/

static void nsscodel_explain(void)
{
	fprintf(stderr, "Usage: ... nsscodel target TIME interval TIME [ limit PACKETS ] [ set_default ] [ accel_mode ]\n");
}

static void nsscodel_explain_err1(void)
{
	fprintf(stderr, "Value of target and interval should be greater than 1ms\n");
}

static int nsscodel_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	struct rtattr *tail;
	struct tc_nsscodel_qopt opt;
	bool accel_mode = false;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "target") == 0) {
			NEXT_ARG();
			if (get_time(&opt.target, *argv)) {
				fprintf(stderr, "Illegal \"target\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(&opt.limit, *argv) || opt.limit == 0) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "interval") == 0) {
			NEXT_ARG();
			if (get_time(&opt.interval, *argv)) {
				fprintf(stderr, "Illegal \"interval\"\n");
				return -1;
			}
		} else if (strcmp(*argv, "set_default") == 0) {
			opt.set_default = 1;
		} else if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (strcmp(*argv, "help") == 0) {
			nsscodel_explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nsscodel_explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_NSS_FW;
	} else if (opt.accel_mode != TCA_NSS_ACCEL_MODE_NSS_FW) {
		fprintf(stderr, "accel_mode should be %d\n", TCA_NSS_ACCEL_MODE_NSS_FW);
		return -1;
	}

	if (!opt.target || !opt.interval) {
		nsscodel_explain();
		return -1;
	}

	if (opt.target < 1000 || opt.interval < 1000) {
		nsscodel_explain_err1();
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSCODEL_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nsscodel_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSCODEL_MAX + 1];
	struct tc_nsscodel_qopt *qopt;
	SPRINT_BUF(b1);
	SPRINT_BUF(b2);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSCODEL_MAX, opt);

	if (tb[TCA_NSSCODEL_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSCODEL_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSCODEL_PARMS]);

	fprintf(f, "target %s limit %up interval %s ",
		sprint_time(qopt->target, b1),
		qopt->limit,
		sprint_time(qopt->interval, b2));

	if (qopt->set_default)
		fprintf(f, "set_default ");

	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

static int nsscodel_print_xstats(struct qdisc_util *qu, FILE *f, struct rtattr *xstats)
{
	struct tc_nsscodel_xstats *st;

	if (xstats == NULL)
		return 0;

	if (RTA_PAYLOAD(xstats) < sizeof(*st))
		return -1;

	st = RTA_DATA(xstats);
	fprintf(f, " peak queue delay %ums peak drop delay %ums",
			st->peak_queue_delay, st->peak_drop_delay);

	return 0;
}

struct qdisc_util nsscodel_qdisc_util = {
	.id		= "nsscodel",
	.parse_qopt	= nsscodel_parse_opt,
	.print_qopt	= nsscodel_print_opt,
	.print_xstats	= nsscodel_print_xstats,
};

/* ======================== NSSTBL =======================*/

static void nsstbl_explain(void)
{
	fprintf(stderr, "Usage: ... nsstbl burst BYTES rate BPS [ mtu BYTES ] [ accel_mode ]\n");
}

static int nsstbl_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int ok = 0;
	struct rtattr *tail;
	struct tc_nsstbl_qopt opt;
	bool accel_mode = false;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "burst") == 0 ||
			   strcmp(*argv, "buffer") == 0 ||
			   strcmp(*argv, "maxburst") == 0) {
			NEXT_ARG();
			if (opt.burst) {
				fprintf(stderr, "Double \"buffer/burst\" spec\n");
				return -1;
			}
			if (get_size(&opt.burst, *argv)) {
				fprintf(stderr, "Illegal \"burst\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "mtu") == 0 ||
			   strcmp(*argv, "minburst") == 0) {
			NEXT_ARG();
			if (opt.mtu) {
				fprintf(stderr, "Double \"mtu/minburst\" spec\n");
				return -1;
			}
			if (get_size(&opt.mtu, *argv)) {
				fprintf(stderr, "Illegal \"mtu\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "rate") == 0) {
			NEXT_ARG();
			if (opt.rate) {
				fprintf(stderr, "Double \"rate\" spec\n");
				return -1;
			}
			if (get_rate(&opt.rate, *argv)) {
				fprintf(stderr, "Illegal \"rate\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (strcmp(*argv, "help") == 0) {
			nsstbl_explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nsstbl_explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!ok) {
		nsstbl_explain();
		return -1;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_PPE;
	} else if (opt.accel_mode >= TCA_NSS_ACCEL_MODE_MAX) {
		fprintf(stderr, "accel_mode should be < %d\n", TCA_NSS_ACCEL_MODE_MAX);
		return -1;
	}

	if (!opt.rate || !opt.burst) {
		fprintf(stderr, "Both \"rate\" and \"burst\" are required.\n");
		return -1;
	}

	/*
	 * Peakrate is currently not supported, but we keep the infrastructure
	 * for future use. However, we have disabled taking input for this.
	 */
	if (opt.peakrate) {
		if (!opt.mtu) {
			fprintf(stderr, "\"mtu\" is required, if \"peakrate\" is requested.\n");
			return -1;
		}
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSTBL_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nsstbl_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSTBL_MAX + 1];
	struct tc_nsstbl_qopt *qopt;
	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSTBL_MAX, opt);

	if (tb[TCA_NSSTBL_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSTBL_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSTBL_PARMS]);

	fprintf(f, "buffer/maxburst %s ", sprint_size(qopt->burst, b1));
	fprintf(f, "rate %s ", sprint_rate(qopt->rate, b1));
	fprintf(f, "mtu %s ", sprint_size(qopt->mtu, b1));
	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

struct qdisc_util nsstbl_qdisc_util = {
	.id		= "nsstbl",
	.parse_qopt	= nsstbl_parse_opt,
	.print_qopt	= nsstbl_print_opt,
};

/* ======================== NSSPRIO =======================*/

static void nssprio_explain(void)
{
	fprintf(stderr, "Usage: ... nssprio [ bands NUMBER (default 256) ] [ accel_mode ]\n");
}

static int nssprio_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int ok = 0;
	struct rtattr *tail;
	struct tc_nssprio_qopt opt;
	bool accel_mode = false;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "bands") == 0) {
			NEXT_ARG();
			if (get_unsigned(&opt.bands, *argv, 0)) {
				fprintf(stderr, "Illegal \"limit\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (strcmp(*argv, "help") == 0) {
			nssprio_explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nssprio_explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!ok) {
		opt.bands = TCA_NSSPRIO_MAX_BANDS;
	} else if (opt.bands > TCA_NSSPRIO_MAX_BANDS) {
		nssprio_explain();
		return -1;
	}

        if (!accel_mode) {
                opt.accel_mode = TCA_NSS_ACCEL_MODE_PPE;
        } else if (opt.accel_mode >= TCA_NSS_ACCEL_MODE_MAX) {
                fprintf(stderr, "accel_mode should be < %d\n", TCA_NSS_ACCEL_MODE_MAX);
                return -1;
        }

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSPRIO_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nssprio_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSPRIO_MAX + 1];
	struct tc_nssprio_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSPRIO_MAX, opt);

	if (tb[TCA_NSSPRIO_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSPRIO_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSPRIO_PARMS]);

	fprintf(f, "bands %u ", qopt->bands);
	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

struct qdisc_util nssprio_qdisc_util = {
	.id		= "nssprio",
	.parse_qopt	= nssprio_parse_opt,
	.print_qopt	= nssprio_print_opt,
};

/* ======================== NSSBF =======================*/

static void nssbf_explain_qdisc(void)
{
	fprintf(stderr,
		"Usage: ... nssbf [ accel_mode ]\n"
	);
}

static void nssbf_explain_class(void)
{
	fprintf(stderr, "Usage: ... nssbf rate BPS burst BYTES [ mtu BYTES ]\n");
	fprintf(stderr, "                 [ quantum BYTES ]\n");
}

static void nssbf_explain1(char *arg)
{
	fprintf(stderr, "NSSBF: Illegal \"%s\"\n", arg);
}

static int nssbf_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	struct tc_nssbf_qopt opt;
	struct rtattr *tail;
	bool accel_mode = false;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (matches(*argv, "default") == 0) {
			NEXT_ARG();
			if (opt.defcls != 0) {
				fprintf(stderr, "NSSBF: Double \"default\"\n");
				return -1;
			}
			if (get_u16(&opt.defcls, *argv, 16) < 0) {
				nssbf_explain1("default");
				return -1;
			}
		} else if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (matches(*argv, "help") == 0) {
			nssbf_explain_qdisc();
			return -1;
		} else {
			fprintf(stderr, "NSSBF: What is \"%s\" ?\n", *argv);
			nssbf_explain_qdisc();
			return -1;
		}
		argc--, argv++;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_NSS_FW;
	} else if (opt.accel_mode != TCA_NSS_ACCEL_MODE_NSS_FW) {
		fprintf(stderr, "accel_mode should be %d\n", TCA_NSS_ACCEL_MODE_NSS_FW);
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSBF_QDISC_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nssbf_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSBF_MAX + 1];
	struct tc_nssbf_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSBF_MAX, opt);

	if (tb[TCA_NSSBF_QDISC_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSBF_QDISC_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSBF_QDISC_PARMS]);

	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

static int nssbf_parse_class_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int ok = 0;
	struct rtattr *tail;
	struct tc_nssbf_class_qopt opt;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "burst") == 0 ||
			   strcmp(*argv, "buffer") == 0 ||
			   strcmp(*argv, "maxburst") == 0) {
			NEXT_ARG();
			if (opt.burst) {
				fprintf(stderr, "Double \"buffer/burst\" spec\n");
				return -1;
			}
			if (get_size(&opt.burst, *argv)) {
				fprintf(stderr, "Illegal \"burst\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "mtu") == 0) {
			NEXT_ARG();
			if (opt.mtu) {
				fprintf(stderr, "Double \"mtu\" spec\n");
				return -1;
			}
			if (get_size(&opt.mtu, *argv)) {
				fprintf(stderr, "Illegal \"mtu\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (opt.quantum) {
				fprintf(stderr, "Double \"quantum\" spec\n");
				return -1;
			}
			if (get_size(&opt.quantum, *argv)) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "rate") == 0) {
			NEXT_ARG();
			if (opt.rate) {
				fprintf(stderr, "Double \"rate\" spec\n");
				return -1;
			}
			if (get_rate(&opt.rate, *argv)) {
				fprintf(stderr, "Illegal \"rate\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "help") == 0) {
			nssbf_explain_class();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nssbf_explain_class();
			return -1;
		}
		argc--; argv++;
	}

	if (!ok) {
		nssbf_explain_class();
		return -1;
	}

	if (!opt.rate || !opt.burst) {
		fprintf(stderr, "Both \"rate\" and \"burst\" are required.\n");
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSBF_CLASS_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nssbf_print_class_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSBF_MAX + 1];
	struct tc_nssbf_class_qopt *qopt;
	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSBF_MAX, opt);

	if (tb[TCA_NSSBF_CLASS_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSBF_CLASS_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSBF_CLASS_PARMS]);

	fprintf(f, "burst %s ", sprint_size(qopt->burst, b1));
	fprintf(f, "rate %s ", sprint_rate(qopt->rate, b1));
	fprintf(f, "quantum %s ", sprint_size(qopt->quantum, b1));
	fprintf(f, "mtu %s ", sprint_size(qopt->mtu, b1));

	return 0;
}

struct qdisc_util nssbf_qdisc_util = {
	.id		= "nssbf",
	.parse_qopt	= nssbf_parse_opt,
	.print_qopt	= nssbf_print_opt,
	.parse_copt	= nssbf_parse_class_opt,
	.print_copt	= nssbf_print_class_opt,
};

/* ======================== NSSWRR =======================*/

static void nsswrr_explain_qdisc(void)
{
	fprintf(stderr,	"Usage (qdisc): ... nsswrr [ accel_mode ]\n");
}

static void nsswrr_explain_class(void)
{
	fprintf(stderr, "Usage (class): ... nsswrr quantum PACKETS ]\n");
}

static int nsswrr_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	struct tc_nsswrr_qopt opt;
	bool accel_mode = false;
	struct rtattr *tail;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (matches(*argv, "help") == 0) {
			nsswrr_explain_qdisc();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\" ?\n", *argv);
			nsswrr_explain_qdisc();
			return -1;
		}
		argc--, argv++;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_PPE;
	} else if (opt.accel_mode >= TCA_NSS_ACCEL_MODE_MAX) {
		fprintf(stderr, "accel_mode should be < %d\n", TCA_NSS_ACCEL_MODE_MAX);
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSWRR_QDISC_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nsswrr_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSWRR_MAX + 1];
	struct tc_nsswrr_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSWRR_MAX, opt);

	if (tb[TCA_NSSWRR_QDISC_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSWRR_QDISC_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSWRR_QDISC_PARMS]);
	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

static int nsswrr_parse_class_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int ok = 0;
	struct rtattr *tail;
	struct tc_nsswrr_class_qopt opt;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (get_u32(&opt.quantum, *argv, 10)) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "help") == 0) {
			nsswrr_explain_class();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nsswrr_explain_class();
			return -1;
		}
		argc--; argv++;
	}

	if (!ok) {
		nsswrr_explain_class();
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSWRR_CLASS_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nsswrr_print_class_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSWRR_MAX + 1];
	struct tc_nsswrr_class_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSWRR_MAX, opt);

	if (tb[TCA_NSSWRR_CLASS_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSWRR_CLASS_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSWRR_CLASS_PARMS]);

	fprintf(f, "quantum %up ", qopt->quantum);
	return 0;
}

struct qdisc_util nsswrr_qdisc_util = {
	.id		= "nsswrr",
	.parse_qopt	= nsswrr_parse_opt,
	.print_qopt	= nsswrr_print_opt,
	.parse_copt	= nsswrr_parse_class_opt,
	.print_copt	= nsswrr_print_class_opt,
};

/* ======================== NSSWFQ =======================*/

static void nsswfq_explain_qdisc(void)
{
	fprintf(stderr, "Usage (qdisc): ... nsswfq [ accel_mode ]\n");
}

static void nsswfq_explain_class(void)
{
	fprintf(stderr, "Usage (class): ... nsswfq quantum BYTES ]\n");
}

static int nsswfq_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	struct tc_nsswfq_qopt opt;
	bool accel_mode = false;
	struct rtattr *tail;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (matches(*argv, "help") == 0) {
			nsswfq_explain_qdisc();
			return -1;
		} else {
			fprintf(stderr, "NSSWFQ: What is \"%s\" ?\n", *argv);
			nsswfq_explain_qdisc();
			return -1;
		}
		argc--, argv++;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_PPE;
	} else if (opt.accel_mode >= TCA_NSS_ACCEL_MODE_MAX) {
		fprintf(stderr, "accel_mode should be < %d\n", TCA_NSS_ACCEL_MODE_MAX);
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSWFQ_QDISC_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nsswfq_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSWFQ_MAX + 1];
	struct tc_nsswfq_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSWFQ_MAX, opt);

	if (tb[TCA_NSSWFQ_QDISC_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSWFQ_QDISC_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSWFQ_QDISC_PARMS]);
	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

static int nsswfq_parse_class_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int ok = 0;
	struct rtattr *tail;
	struct tc_nsswfq_class_qopt opt;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (get_size(&opt.quantum, *argv)) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "help") == 0) {
			nsswfq_explain_class();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nsswfq_explain_class();
			return -1;
		}
		argc--; argv++;
	}

	if (!ok) {
		nsswfq_explain_class();
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSWFQ_CLASS_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nsswfq_print_class_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSWFQ_MAX + 1];
	struct tc_nsswfq_class_qopt *qopt;
	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSWFQ_MAX, opt);

	if (tb[TCA_NSSWFQ_CLASS_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSWFQ_CLASS_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSWFQ_CLASS_PARMS]);

	fprintf(f, "quantum %s ", sprint_size(qopt->quantum, b1));

	return 0;
}

struct qdisc_util nsswfq_qdisc_util = {
	.id		= "nsswfq",
	.parse_qopt	= nsswfq_parse_opt,
	.print_qopt	= nsswfq_print_opt,
	.parse_copt	= nsswfq_parse_class_opt,
	.print_copt	= nsswfq_print_class_opt,
};

/* ======================== NSSHTB =======================*/

static void nsshtb_explain_qdisc(void)
{
	fprintf(stderr,
		"Usage: ... nsshtb [ r2q ] [ accel_mode ]\n"
	);
}

static void nsshtb_explain_class(void)
{
	fprintf(stderr, "Usage: ... nsshtb priority 0-3 [ quantum BYTES ] [ rate BPS ] [ burst BYTES ] [crate BPS ] [ cburst BYTES ]\n");
	fprintf(stderr, "                 [ overhead BYTES ] \n");
}

static void nsshtb_explain1(char *arg)
{
	fprintf(stderr, "NSSHTB: Illegal \"%s\"\n", arg);
}

static int nsshtb_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	struct tc_nsshtb_qopt opt;
	struct rtattr *tail;
	bool accel_mode = false;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "r2q") == 0) {
			NEXT_ARG();
			if (opt.r2q != 0) {
				fprintf(stderr, "NSSHTB: Double \"r2q\"\n");
				return -1;
			}
			if (get_u32(&opt.r2q, *argv, 10) < 0) {
				nsshtb_explain1("r2q");
				return -1;
			}
		} else if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (strcmp(*argv, "help") == 0) {
			nsshtb_explain_qdisc();
			return -1;
		} else {
			fprintf(stderr, "NSSHTB: What is \"%s\" ?\n", *argv);
			nsshtb_explain_qdisc();
			return -1;
		}
		argc--, argv++;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_PPE;
	} else if (opt.accel_mode >= TCA_NSS_ACCEL_MODE_MAX) {
		fprintf(stderr, "accel_mode should be < %d\n", TCA_NSS_ACCEL_MODE_MAX);
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSHTB_QDISC_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nsshtb_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSHTB_MAX + 1];
	struct tc_nsshtb_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSHTB_MAX, opt);

	if (tb[TCA_NSSHTB_QDISC_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSHTB_QDISC_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSHTB_QDISC_PARMS]);

	if (qopt->r2q != 0)
		fprintf(f, "r2q %u ", qopt->r2q);

	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

static int nsshtb_parse_class_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int ok = 0;
	struct rtattr *tail;
	struct tc_nsshtb_class_qopt opt;
	int crate = 0;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "burst") == 0) {
			NEXT_ARG();
			if (opt.burst) {
				fprintf(stderr, "Double \"burst\" spec\n");
				return -1;
			}
			if (get_size(&opt.burst, *argv)) {
				fprintf(stderr, "Illegal \"burst\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "rate") == 0) {
			NEXT_ARG();
			if (opt.rate) {
				fprintf(stderr, "Double \"rate\" spec\n");
				return -1;
			}
			if (get_rate(&opt.rate, *argv)) {
				fprintf(stderr, "Illegal \"rate\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "cburst") == 0) {
			NEXT_ARG();
			if (opt.cburst) {
				fprintf(stderr, "Double \"cburst\" spec\n");
				return -1;
			}
			if (get_size(&opt.cburst, *argv)) {
				fprintf(stderr, "Illegal \"cburst\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "crate") == 0) {
			NEXT_ARG();
			if (opt.crate) {
				fprintf(stderr, "Double \"crate\" spec\n");
				return -1;
			}
			if (get_rate(&opt.crate, *argv)) {
				fprintf(stderr, "Illegal \"crate\"\n");
				return -1;
			}
			crate++;
			ok++;
		} else if (strcmp(*argv, "priority") == 0) {
			NEXT_ARG();
			if (opt.priority) {
				fprintf(stderr, "Double \"priority\" spec\n");
				return -1;
			}
			if (get_u32(&opt.priority, *argv, 10) < 0) {
				fprintf(stderr, "Illegal \"priority\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "quantum") == 0) {
			NEXT_ARG();
			if (opt.quantum) {
				fprintf(stderr, "Double \"quantum\" spec\n");
				return -1;
			}
			if (get_size(&opt.quantum, *argv)) {
				fprintf(stderr, "Illegal \"quantum\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "overhead") == 0) {
			NEXT_ARG();
			if (opt.overhead) {
				fprintf(stderr, "Double \"overhead\" spec\n");
				return -1;
			}
			if (get_size(&opt.overhead, *argv)) {
				fprintf(stderr, "Illegal \"overhead\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "help") == 0) {
			nsshtb_explain_class();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nsshtb_explain_class();
			return -1;
		}
		argc--; argv++;
	}

	if (!ok) {
		nsshtb_explain_class();
		return -1;
	}

	if (opt.rate && !opt.burst) {
		fprintf(stderr, "\"burst\" required if \"rate\" is specified.\n");
		return -1;
	}

	if (!crate) {
		fprintf(stderr, "\"crate\" is required.\n");
		return -1;
	}

	if (opt.crate && !opt.cburst) {
		fprintf(stderr, "\"cburst\" required if \"crate\" is non-zero.\n");
		return -1;
	}

	if (opt.priority > 3) {
		fprintf(stderr, "\"priority\" should be an integer between 0 and 3.\n");
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSHTB_CLASS_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nsshtb_print_class_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSHTB_MAX + 1];
	struct tc_nsshtb_class_qopt *qopt;
	SPRINT_BUF(b1);

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSHTB_MAX, opt);

	if (tb[TCA_NSSHTB_CLASS_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSHTB_CLASS_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSHTB_CLASS_PARMS]);

	fprintf(f, "burst %s ", sprint_size(qopt->burst, b1));
	fprintf(f, "rate %s ", sprint_rate(qopt->rate, b1));
	fprintf(f, "cburst %s ", sprint_size(qopt->cburst, b1));
	fprintf(f, "crate %s ", sprint_rate(qopt->crate, b1));
	fprintf(f, "priority %u ", qopt->priority);
	fprintf(f, "quantum %s ", sprint_size(qopt->quantum, b1));
	fprintf(f, "overhead %s ", sprint_size(qopt->overhead, b1));

	return 0;
}

struct qdisc_util nsshtb_qdisc_util = {
	.id		= "nsshtb",
	.parse_qopt	= nsshtb_parse_opt,
	.print_qopt	= nsshtb_print_opt,
	.parse_copt	= nsshtb_parse_class_opt,
	.print_copt	= nsshtb_print_class_opt,
};

/* ======================== NSSBLACKHOLE ======================= */

static void nssblackhole_explain(void)
{
	fprintf(stderr, "Usage: ...  nssblackhole [ set_default ] [ accel_mode ]\n");
}

static int nssblackhole_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	struct rtattr *tail;
	struct tc_nssblackhole_qopt opt;
	bool accel_mode = false;

	memset(&opt, 0, sizeof(opt));

	while (argc > 0) {
		if (strcmp(*argv, "set_default") == 0) {
			opt.set_default = 1;
		} else if (strcmp(*argv, "accel_mode") == 0) {
			NEXT_ARG();
			if (get_u8(&opt.accel_mode, *argv, 0)) {
				fprintf(stderr, "Illegal accel_mode value\n");
				return -1;
			}
			accel_mode = true;
		} else if (strcmp(*argv, "help") == 0) {
			nssblackhole_explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			nssblackhole_explain();
			return -1;
		}
		argc--; argv++;
	}

	if (!accel_mode) {
		opt.accel_mode = TCA_NSS_ACCEL_MODE_PPE;
	} else if (opt.accel_mode >= TCA_NSS_ACCEL_MODE_MAX) {
		fprintf(stderr, "accel_mode should be < %d\n", TCA_NSS_ACCEL_MODE_MAX);
		return -1;
	}

	tail = NLMSG_TAIL(n);
	addattr_l(n, 1024, TCA_OPTIONS, NULL, 0);
	addattr_l(n, 1024, TCA_NSSBLACKHOLE_PARMS, &opt, sizeof(opt));
	tail->rta_len = (void *) NLMSG_TAIL(n) - (void *) tail;

	return 0;
}

static int nssblackhole_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct rtattr *tb[TCA_NSSBLACKHOLE_MAX + 1];
	struct tc_nssblackhole_qopt *qopt;

	if (opt == NULL)
		return 0;

	parse_rtattr_nested(tb, TCA_NSSBLACKHOLE_MAX, opt);

	if (tb[TCA_NSSBLACKHOLE_PARMS] == NULL)
		return -1;

	if (RTA_PAYLOAD(tb[TCA_NSSBLACKHOLE_PARMS]) < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(tb[TCA_NSSBLACKHOLE_PARMS]);

	if (qopt->set_default)
		fprintf(f, "set_default ");

	fprintf(f, "accel_mode %d ", qopt->accel_mode);

	return 0;
}

struct qdisc_util nssblackhole_qdisc_util = {
	.id		= "nssblackhole",
	.parse_qopt	= nssblackhole_parse_opt,
	.print_qopt	= nssblackhole_print_opt,
};
