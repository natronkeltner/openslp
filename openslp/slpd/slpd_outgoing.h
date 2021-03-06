/*-------------------------------------------------------------------------
 * Copyright (C) 2000 Caldera Systems, Inc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of Caldera Systems nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * `AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CALDERA
 * SYSTEMS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-------------------------------------------------------------------------*/

/** Header file for outgoing network conversations.
 *
 * @file       slpd_outgoing.h
 * @author     Matthew Peterson, John Calcote (jcalcote@novell.com)
 * @attention  Please submit patches to http://www.openslp.org
 * @ingroup    SlpdCode
 */

#ifndef SLPD_OUTGOING_H_INCLUDED
#define SLPD_OUTGOING_H_INCLUDED

/*!@defgroup SlpdCodeOutGoing Outgoing Conversations */

/*!@addtogroup SlpdCodeOutGoing
 * @ingroup SlpdCode
 * @{
 */

#include "slp_types.h"
#include "slpd_socket.h"
#include "slpd.h"

extern SLPList G_OutgoingSocketList;

void SLPDOutgoingAge(time_t seconds);
void SLPDOutgoingHandler(int * fdcount, SLPD_fdset * fdset);
void SLPDOutgoingRetry(time_t seconds);
int SLPDHaveOutgoingConnectedSocket(struct sockaddr_storage* addr);
void SLPDOutgoingDatagramWrite(SLPDSocket * sock, SLPBuffer buffer);
void SLPDOutgoingDatagramMcastWrite(SLPDSocket * sock, struct sockaddr_storage *maddr, SLPBuffer buffer);
SLPDSocket * SLPDOutgoingConnect(int is_TCP, struct sockaddr_storage * addr);
int SLPDOutgoingInit(void);
int SLPDOutgoingDeinit(int graceful);

#ifdef DEBUG
void SLPDOutgoingSocketDump(void);
#endif

/*! @} */

#endif   /* SLPD_OUTGOING_H_INCLUDED */

/*=========================================================================*/
