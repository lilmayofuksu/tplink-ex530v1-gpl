#!/bin/sh

#update the modify time of below files to the time of those files were unzipped from tar.gz,
#and then libevent will be configed correctly.

touch -m -d "2017-01-30 01:51:12" aclocal.m4
touch -m -d "2017-01-30 01:51:00" configure.ac
touch -m -d "2016-10-05 03:55:31" include/include.am

touch -m -d "2016-10-05 03:55:31" m4/ac_backport_259_ssizet.m4
touch -m -d "2016-10-05 03:55:31" m4/acx_pthread.m4
touch -m -d "2016-10-05 03:55:31" m4/libevent_openssl.m4
touch -m -d "2017-01-15 17:34:26" m4/libtool.m4
touch -m -d "2017-01-15 17:34:26" m4/lt~obsolete.m4
touch -m -d "2017-01-15 17:34:26" m4/ltoptions.m4
touch -m -d "2017-01-15 17:34:26" m4/ltsugar.m4
touch -m -d "2017-01-15 17:34:26" m4/ltversion.m4
touch -m -d "2016-10-05 03:55:31" m4/ntp_pkg_config.m4

touch -m -d "2016-10-05 03:55:31" sample/include.am

touch -m -d "2017-01-20 21:26:30" test/include.am
touch -m -d "2016-10-05 03:55:31" test/Makefile.nmake
