/****************************************************************************
 * net/socket/socket.h
 *
 *   Copyright (C) 2007-2009, 2011-2014, 2017 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef _NET_SOCKET_SOCKET_H
#define _NET_SOCKET_SOCKET_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#ifdef CONFIG_NET

#include <sys/types.h>
#include <stdint.h>
#include <time.h>

#include <nuttx/clock.h>
#include <nuttx/net/net.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Definitions of 8-bit socket flags */

                                  /* Bits 0-2: Socket state */
#define _SF_IDLE            0x00  /* - There is no socket activity */
#define _SF_ACCEPT          0x01  /* - Socket is waiting to accept a connection */
#define _SF_RECV            0x02  /* - Waiting for recv action to complete */
#define _SF_SEND            0x03  /* - Waiting for send action to complete */
#define _SF_MASK            0x03  /* - Mask to isolate the above actions */

#define _SF_NONBLOCK        0x08  /* Bit 3: Don't block if no data (TCP/READ only) */
#define _SF_LISTENING       0x10  /* Bit 4: SOCK_STREAM is listening */
#define _SF_BOUND           0x20  /* Bit 5: SOCK_STREAM is bound to an address */
                                  /* Bits 6-7: Connection state */
#define _SF_CONNECTED       0x40  /* Bit 6: SOCK_STREAM/SOCK_DGRAM is connected */
#define _SF_CLOSED          0x80  /* Bit 7: SOCK_STREAM was gracefully disconnected */

/* Connection state encoding:
 *
 *  _SF_CONNECTED==1 && _SF_CLOSED==0 - the socket is connected
 *  _SF_CONNECTED==0 && _SF_CLOSED==1 - the socket was gracefully disconnected
 *  _SF_CONNECTED==0 && _SF_CLOSED==0 - the socket was rudely disconnected
 */

/* Macro to manage the socket state and flags */

#define _SS_SETSTATE(s,f)   (((s) & ~_SF_MASK) | (f))
#define _SS_GETSTATE(s)     ((s) & _SF_MASK)
#define _SS_ISBUSY(s)       (_SS_GETSTATE(s) != _SF_IDLE)

#define _SS_ISNONBLOCK(s)   (((s) & _SF_NONBLOCK)  != 0)
#define _SS_ISLISTENING(s)  (((s) & _SF_LISTENING) != 0)
#define _SS_ISBOUND(s)      (((s) & _SF_BOUND)     != 0)
#define _SS_ISCONNECTED(s)  (((s) & _SF_CONNECTED) != 0)
#define _SS_ISCLOSED(s)     (((s) & _SF_CLOSED)    != 0)

/* This macro converts a socket option value into a bit setting */

#define _SO_BIT(o)       (1 << (o))

/* These define bit positions for each socket option (see sys/socket.h) */

#define _SO_ACCEPTCONN   _SO_BIT(SO_ACCEPTCONN)
#define _SO_BROADCAST    _SO_BIT(SO_BROADCAST)
#define _SO_DEBUG        _SO_BIT(SO_DEBUG)
#define _SO_DONTROUTE    _SO_BIT(SO_DONTROUTE)
#define _SO_ERROR        _SO_BIT(SO_ERROR)
#define _SO_KEEPALIVE    _SO_BIT(SO_KEEPALIVE)
#define _SO_LINGER       _SO_BIT(SO_LINGER)
#define _SO_OOBINLINE    _SO_BIT(SO_OOBINLINE)
#define _SO_RCVBUF       _SO_BIT(SO_RCVBUF)
#define _SO_RCVLOWAT     _SO_BIT(SO_RCVLOWAT)
#define _SO_RCVTIMEO     _SO_BIT(SO_RCVTIMEO)
#define _SO_REUSEADDR    _SO_BIT(SO_REUSEADDR)
#define _SO_SNDBUF       _SO_BIT(SO_SNDBUF)
#define _SO_SNDLOWAT     _SO_BIT(SO_SNDLOWAT)
#define _SO_SNDTIMEO     _SO_BIT(SO_SNDTIMEO)
#define _SO_TYPE         _SO_BIT(SO_TYPE)

/* This is the largest option value.  REVISIT: belongs in sys/socket.h */

#define _SO_MAXOPT       (15)

/* Macros to set, test, clear options */

#define _SO_SETOPT(s,o)  ((s) |= _SO_BIT(o))
#define _SO_CLROPT(s,o)  ((s) &= ~_SO_BIT(o))
#define _SO_GETOPT(s,o)  (((s) & _SO_BIT(o)) != 0)

/* These are macros that can be used to determine if socket option code is
 * valid (in range) and supported by API.
 */

#define _SO_GETONLYSET   (_SO_ACCEPTCONN|_SO_ERROR|_SO_TYPE)
#define _SO_GETONLY(o)   ((_SO_BIT(o) & _SO_GETONLYSET) != 0)
#define _SO_GETVALID(o)  (((unsigned int)(o)) <= _SO_MAXOPT)
#define _SO_SETVALID(o)  ((((unsigned int)(o)) <= _SO_MAXOPT) && !_SO_GETONLY(o))

/* Macro to set socket errors */

#ifdef CONFIG_NET_SOCKOPTS
#  define _SO_SETERRNO(s,e) \
    do \
      { \
        if (s != NULL) \
          { \
            s->s_error = (int16_t)e; \
          } \
        set_errno(e); \
      } \
    while (0)
#else
#  define _SO_SETERRNO(s,e) set_errno(e)
#endif /* CONFIG_NET_SOCKOPTS */

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: sockfd_allocate
 *
 * Description:
 *   Allocate a socket descriptor
 *
 * Input Parameters:
 *   Lowest socket descriptor index to be used.
 *
 * Returned Value:
 *   On success, a socket descriptor >= minsd is returned.  A negated errno
 *   value is returned on failure.
 *
 ****************************************************************************/

int sockfd_allocate(int minsd);

/****************************************************************************
 * Name: psock_release
 *
 * Description:
 *   Free a socket.
 *
 * Input Parameters:
 *   psock - A reference to the socket instance to be freed.
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void psock_release(FAR struct socket *psock);

/****************************************************************************
 * Name: sockfd_release
 *
 * Description:
 *   Free the socket by its socket descriptor.
 *
 * Input Parameters:
 *   sockfd - Socket descriptor identifies the socket to be released.
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void sockfd_release(int sockfd);

/****************************************************************************
 * Name: sockfd_socket
 *
 * Description:
 *   Given a socket descriptor, return the underlying socket structure.
 *
 * Input Parameters:
 *   sockfd - The socket descriptor index o use.
 *
 * Returned Value:
 *   On success, a reference to the socket structure associated with the
 *   the socket descriptor is returned.  NULL is returned on any failure.
 *
 ****************************************************************************/

FAR struct socket *sockfd_socket(int sockfd);

/****************************************************************************
 * Name: net_sockif
 *
 * Description:
 *   Return the socket interface associated with this address family.
 *
 * Input Parameters:
 *   family   - Socket address family
 *   type     - Socket type
 *   protocol - Socket protocol
 *
 * Returned Value:
 *   On success, a non-NULL instance of struct sock_intf_s is returned.  NULL
 *   is returned only if the address family is not supported.
 *
 ****************************************************************************/

FAR const struct sock_intf_s *
  net_sockif(sa_family_t family, int type, int protocol);

/****************************************************************************
 * Name: net_timeo
 *
 * Description:
 *   Check if a timeout has elapsed.  This can be called from a socket poll
 *   function to determine if a timeout has occurred.
 *
 * Input Parameters:
 *   start_time Timeout start time in system clock ticks
 *   timeout    Timeout value in deciseconds.
 *
 * Returned Value:
 *   0 (FALSE) if not timeout; 1 (TRUE) if timeout
 *
 * Assumptions:
 *
 ****************************************************************************/

#ifdef CONFIG_NET_SOCKOPTS
int net_timeo(clock_t start_time, socktimeo_t timeo);
#endif

/****************************************************************************
 * Name: psock_send
 *
 * Description:
 *   The send() call may be used only when the socket is in a connected state
 *   (so that the intended recipient is known). The only difference between
 *   send() and write() is the presence of flags. With zero flags parameter,
 *   send() is equivalent to write(). Also, send(sockfd,buf,len,flags) is
 *   equivalent to sendto(sockfd,buf,len,flags,NULL,0).
 *
 * Input Parameters:
 *   psock    An instance of the internal socket structure.
 *   buf      Data to send
 *   len      Length of data to send
 *   flags    Send flags
 *
 * Returned Value:
 *   On success, returns the number of characters sent.  On  error,
 *   -1 is returned, and errno is set appropriately:
 *
 *   EAGAIN or EWOULDBLOCK
 *     The socket is marked non-blocking and the requested operation
 *     would block.
 *   EBADF
 *     An invalid descriptor was specified.
 *   ECONNRESET
 *     Connection reset by peer.
 *   EDESTADDRREQ
 *     The socket is not connection-mode, and no peer address is set.
 *   EFAULT
 *      An invalid user space address was specified for a parameter.
 *   EINTR
 *      A signal occurred before any data was transmitted.
 *   EINVAL
 *      Invalid argument passed.
 *   EISCONN
 *     The connection-mode socket was connected already but a recipient
 *     was specified. (Now either this error is returned, or the recipient
 *     specification is ignored.)
 *   EMSGSIZE
 *     The socket type requires that message be sent atomically, and the
 *     size of the message to be sent made this impossible.
 *   ENOBUFS
 *     The output queue for a network interface was full. This generally
 *     indicates that the interface has stopped sending, but may be
 *     caused by transient congestion.
 *   ENOMEM
 *     No memory available.
 *   ENOTCONN
 *     The socket is not connected, and no target has been given.
 *   ENOTSOCK
 *     The argument s is not a socket.
 *   EOPNOTSUPP
 *     Some bit in the flags argument is inappropriate for the socket
 *     type.
 *   EPIPE
 *     The local end has been shut down on a connection oriented socket.
 *     In this case the process will also receive a SIGPIPE unless
 *     MSG_NOSIGNAL is set.
 *
 ****************************************************************************/

ssize_t psock_send(FAR struct socket *psock, FAR const void *buf, size_t len,
                   int flags);

/****************************************************************************
 * Name: net_clone
 *
 * Description:
 *   Performs the low level, common portion of net_dupsd() and net_dupsd2()
 *
 * Input Parameters:
 *   psock1 - The existing socket that is being cloned.
 *   psock2 - A reference to an uninitialized socket structure alloated by
 *            the caller.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   any failure.
 *
 ****************************************************************************/

int net_clone(FAR struct socket *psock1, FAR struct socket *psock2);

#endif /* CONFIG_NET */
#endif /* _NET_SOCKET_SOCKET_H */
