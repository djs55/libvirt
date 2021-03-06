/*
 * Copyright (C) 2013 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Author: Daniel P. Berrange <berrange@redhat.com>
 */

#include <config.h>

#ifdef MOCK_HELPER
# include "internal.h"
# include <sys/socket.h>
# include <errno.h>
# include <arpa/inet.h>
# include <netinet/in.h>

int bind(int sockfd ATTRIBUTE_UNUSED,
         const struct sockaddr *addr,
         socklen_t addrlen ATTRIBUTE_UNUSED)
{
    struct sockaddr_in saddr;

    memcpy(&saddr, addr, sizeof(saddr));

    if (saddr.sin_port == htons(5900) ||
        saddr.sin_port == htons(5904) ||
        saddr.sin_port == htons(5905) ||
        saddr.sin_port == htons(5906)) {
        errno = EADDRINUSE;
        return -1;
    }

    return 0;
}

#else
# include <stdlib.h>

# include "testutils.h"
# include "virutil.h"
# include "virerror.h"
# include "viralloc.h"
# include "virfile.h"
# include "virlog.h"
# include "virportallocator.h"
# include "virstring.h"

# define VIR_FROM_THIS VIR_FROM_RPC


static int testAllocAll(const void *args ATTRIBUTE_UNUSED)
{
    virPortAllocatorPtr alloc = virPortAllocatorNew(5900, 5909);
    int ret = -1;
    unsigned short p1, p2, p3, p4, p5, p6, p7;

    if (virPortAllocatorAcquire(alloc, &p1) < 0)
        goto cleanup;
    if (p1 != 5901) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5901, got %d", p1);
        goto cleanup;
    }

    if (virPortAllocatorAcquire(alloc, &p2) < 0)
        goto cleanup;
    if (p2 != 5902) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5902, got %d", p2);
        goto cleanup;
    }

    if (virPortAllocatorAcquire(alloc, &p3) < 0)
        goto cleanup;
    if (p3 != 5903) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5903, got %d", p3);
        goto cleanup;
    }

    if (virPortAllocatorAcquire(alloc, &p4) < 0)
        goto cleanup;
    if (p4 != 5907) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5907, got %d", p4);
        goto cleanup;
    }

    if (virPortAllocatorAcquire(alloc, &p5) < 0)
        goto cleanup;
    if (p5 != 5908) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5908, got %d", p5);
        goto cleanup;
    }

    if (virPortAllocatorAcquire(alloc, &p6) < 0)
        goto cleanup;
    if (p6 != 5909) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5909, got %d", p6);
        goto cleanup;
    }

    if (virPortAllocatorAcquire(alloc, &p7) < 0)
        goto cleanup;
    if (p7 != 0) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 0, got %d", p7);
        goto cleanup;
    }

    ret = 0;
cleanup:
    virObjectUnref(alloc);
    return ret;
}



static int testAllocReuse(const void *args ATTRIBUTE_UNUSED)
{
    virPortAllocatorPtr alloc = virPortAllocatorNew(5900, 5910);
    int ret = -1;
    unsigned short p1, p2, p3, p4;

    if (virPortAllocatorAcquire(alloc, &p1) < 0)
        goto cleanup;
    if (p1 != 5901) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5901, got %d", p1);
        goto cleanup;
    }

    if (virPortAllocatorAcquire(alloc, &p2) < 0)
        goto cleanup;
    if (p2 != 5902) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5902, got %d", p2);
        goto cleanup;
    }

    if (virPortAllocatorAcquire(alloc, &p3) < 0)
        goto cleanup;
    if (p3 != 5903) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5903, got %d", p3);
        goto cleanup;
    }


    if (virPortAllocatorRelease(alloc, p2) < 0)
        goto cleanup;

    if (virPortAllocatorAcquire(alloc, &p4) < 0)
        goto cleanup;
    if (p4 != 5902) {
        if (virTestGetDebug())
            fprintf(stderr, "Expected 5902, got %d", p4);
        goto cleanup;
    }

    ret = 0;
cleanup:
    virObjectUnref(alloc);
    return ret;
}


static int
mymain(void)
{
    int ret = 0;

    if (virtTestRun("Test alloc all", 1, testAllocAll, NULL) < 0)
        ret = -1;

    if (virtTestRun("Test alloc reuse", 1, testAllocReuse, NULL) < 0)
        ret = -1;

    return ret==0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

VIRT_TEST_MAIN_PRELOAD(mymain, abs_builddir "/.libs/libvirportallocatormock.so")
#endif
