/***************************************************************************/
/*                                                                         */
/* Project:     OpenSLP - OpenSource implementation of Service Location    */
/*              Protocol Version 2                                         */
/*                                                                         */
/* File:        slpd_knownda.c                                             */
/*                                                                         */
/* Abstract:    Keeps track of known DAs                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/* Copyright (c) 1995, 1999  Caldera Systems, Inc.                         */
/*                                                                         */
/* This program is free software; you can redistribute it and/or modify it */
/* under the terms of the GNU Lesser General Public License as published   */
/* by the Free Software Foundation; either version 2.1 of the License, or  */
/* (at your option) any later version.                                     */
/*                                                                         */
/*     This program is distributed in the hope that it will be useful,     */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       */
/*     GNU Lesser General Public License for more details.                 */
/*                                                                         */
/*     You should have received a copy of the GNU Lesser General Public    */
/*     License along with this program; see the file COPYING.  If not,     */
/*     please obtain a copy from http://www.gnu.org/copyleft/lesser.html   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*     Please submit patches to http://www.openslp.org                     */
/*                                                                         */
/***************************************************************************/

#include "slpd.h"

/*=========================================================================*/
SLPList G_KnownDAList = {0,0,0};                                         
/* The list of DAs known to slpd.                                          */
/*=========================================================================*/

/*-------------------------------------------------------------------------*/
void SLPDKnownDARegisterAll(SLPDAEntry* daentry)
/* Forks a child process to register all services with specified DA        */
/*-------------------------------------------------------------------------*/
{
    SLPDDatabaseEntry*  dbentry;
    SLPDSocket*         sock;
    SLPBuffer           buf;
    size_t              size;
    void*               handle      = 0;
    
    /* Establish a new connection with the known DA */
    sock = SLPDOutgoingConnect(&daentry->daaddr);
    if(sock)
    {
        while( SLPDDatabaseEnum(&handle,&dbentry) == 0)
        {
            if(SLPIntersectStringList(daentry->scopelistlen,
                                      daentry->scopelist,
                                      dbentry->scopelistlen,
                                      dbentry->scopelist) )
            {
                /*-------------------------------------------------------------*/
                /* ensure the buffer is big enough to handle the whole srvrply */
                /*-------------------------------------------------------------*/
                size = dbentry->langtaglen + 27; /* 14 bytes for header     */
                                                 /*  6 for static portions of urlentry  */
                                                 /*  2 bytes for srvtypelen */
                                                 /*  2 bytes for scopelen */
                                                 /*  2 bytes for attr list len */
                                                 /*  1 byte for authblock count */
                size += dbentry->urllen;
                size += dbentry->srvtypelen;
                size += dbentry->scopelistlen;
                size += dbentry->attrlistlen;
                
                /* TODO: room for authstuff */
                
                buf = SLPBufferAlloc(size);
                if(buf)
                {                               
                    /*--------------------*/
                    /* Construct a SrvReg */
                    /*--------------------*/
                    /*version*/
                    *(buf->start)       = 2;
                    /*function id*/
                    *(buf->start + 1)   = SLP_FUNCT_SRVREG;
                    /*length*/
                    ToUINT24(buf->start + 2, size);
                    /*flags*/
                    ToUINT16(buf->start + 5,
                             size > SLP_MAX_DATAGRAM_SIZE ? SLP_FLAG_OVERFLOW : 0);
                    /*ext offset*/
                    ToUINT24(buf->start + 7,0);
                    /*xid*/
                    ToUINT16(buf->start + 10,rand());
                    /*lang tag len*/
                    ToUINT16(buf->start + 12,dbentry->langtaglen);
                    /*lang tag*/
                    memcpy(buf->start + 14,
                           dbentry->langtag,
                           dbentry->langtaglen);
                    buf->curpos = buf->start + 14 + dbentry->langtaglen;
                
                    /* url-entry reserved */
                    *buf->curpos = 0;        
                    buf->curpos = buf->curpos + 1;
                    /* url-entry lifetime */
                    ToUINT16(buf->curpos,dbentry->lifetime);
                    buf->curpos = buf->curpos + 2;
                    /* url-entry urllen */
                    ToUINT16(buf->curpos,dbentry->urllen);
                    buf->curpos = buf->curpos + 2;
                    /* url-entry url */
                    memcpy(buf->curpos,dbentry->url,dbentry->urllen);
                    buf->curpos = buf->curpos + dbentry->urllen;
                    /* url-entry authcount */
                    *buf->curpos = 0;        
                    buf->curpos = buf->curpos + 1;
                    /* srvtype */
                    ToUINT16(buf->curpos,dbentry->srvtypelen);
                    buf->curpos = buf->curpos + 2;
                    memcpy(buf->curpos,dbentry->srvtype,dbentry->srvtypelen);
                    buf->curpos = buf->curpos + dbentry->srvtypelen;
                    /* scope list */
                    ToUINT16(buf->curpos, dbentry->scopelistlen);
                    buf->curpos = buf->curpos + 2;
                    memcpy(buf->curpos,dbentry->scopelist,dbentry->scopelistlen);
                    buf->curpos = buf->curpos + dbentry->scopelistlen;
                    /* attr list */
                    ToUINT16(buf->curpos, dbentry->attrlistlen);
                    buf->curpos = buf->curpos + 2;
                    memcpy(buf->curpos,dbentry->attrlist,dbentry->attrlistlen);
                    buf->curpos = buf->curpos + dbentry->attrlistlen;;
                    /* authblock count */
                    *(buf->curpos) = 0;
                    buf->curpos = buf->curpos + 1;
    
                    /*--------------------------------------------------*/
                    /* link newly constructed buffer to socket sendlist */
                    /*--------------------------------------------------*/
                    SLPListLinkTail(&(sock->sendlist),(SLPListItem*)buf);
                    if (sock->state == STREAM_CONNECT_IDLE)
                    {
                        sock->state = STREAM_WRITE_FIRST;
                    }
                }
            }
        }
    }
}  


/*=========================================================================*/
int SLPDKnownDAInit()
/* Initializes the KnownDA list.  Removes all entries and adds entries     */
/* that are statically configured.                                         */
/*                                                                         */
/* returns  zero on success, Non-zero on failure                           */
/*=========================================================================*/
{
    char*               temp;
    char*               tempend;
    char*               slider1;
    char*               slider2;
    struct hostent*     he;
    struct in_addr      daaddr;

    /*---------------------------------------*/
    /* Skip static DA parsing if we are a DA */
    /*---------------------------------------*/
    if(G_SlpdProperty.isDA)
    {
        /* TODO: some day we may put something here for DA to DA communication */
        return 0;
    }
    
    /*------------------------------------------------------*/
    /* Added statically configured DAs to the Known DA List */
    /*------------------------------------------------------*/
    if (G_SlpdProperty.DAAddresses && *G_SlpdProperty.DAAddresses)
    {
        temp = slider1 = slider2 = strdup(G_SlpdProperty.DAAddresses);
        if(temp)
        {
            tempend = temp + strlen(temp);
            while (slider1 != tempend)
            {
                while (*slider2 && *slider2 != ',') slider2++;
                *slider2 = 0;
    
                he = gethostbyname(slider1);
                if (he)
                {
                    daaddr.s_addr = *((unsigned long*)(he->h_addr_list[0]));
    
                    SLPDKnownDAEvaluate(&daaddr,
                                        1, 
                                        G_SlpdProperty.useScopes,
                                        G_SlpdProperty.useScopesLen);
                }
    
                slider1 = slider2;
                slider2++;
            }
    
            free(temp);
        }
    }
    
    /* Lastly, Perform active discovery */
    SLPDKnownDAActiveDiscovery();
    
    return 0;
}


/*=========================================================================*/
SLPDAEntry* SLPDKnownDAEvaluate(struct in_addr* addr,
                                unsigned long bootstamp,
                                const char* scopelist,
                                int scopelistlen)
/* Adds a DA to the known DA list if it is new, removes it if DA is going  */
/* down or adjusts entry if DA changed.                                    */
/*                                                                         */
/* addr     (IN) pointer to in_addr of the DA to add                       */
/*                                                                         */
/* scopelist (IN) scope list of the DA to add                              */
/*                                                                         */
/* scopelistlen (IN) the length of the scope list                          */
/*                                                                         */
/* returns  Pointer to the added or updated entry                          */
/*=========================================================================*/
{
    SLPDAEntry* entry;

    /* Iterate through the list looking for an identical entry */
    entry = (SLPDAEntry*)G_KnownDAList.head;
    while (entry)
    {
        /* for now assume entries are the same if in_addrs match */
        if (memcmp(&entry->daaddr,addr,sizeof(struct in_addr)) == 0)
        {
            /* Update an existing entry */
            if(entry->bootstamp == 0)
            {
                /* DA is going down. Remove it from our list */
                SLPDKnownDARemove(entry);
            }
            else if(entry->bootstamp > bootstamp)
            {
                /* DA went down and came up. Record new entry and */
                /* Re-register all services                       */
                entry->bootstamp = bootstamp;
                entry->scopelist = realloc(entry->scopelist,scopelistlen);
                if (entry->scopelist)
                {
                    memcpy(entry->scopelist,scopelist,scopelistlen);
                    entry->scopelistlen = scopelistlen;
                    
                    /* Register all services with the new DA */
                    SLPDKnownDARegisterAll(entry); 
                }
                else
                {
                    /* TODO: Out of memory */
                } 
            }
            else
            {
                /* Just a routine passive discovery */
                entry->bootstamp = bootstamp;
            }

            break;
        }

        entry = (SLPDAEntry*)entry->listitem.next;
    }

    if(entry == 0)
    {
        /* Create and link in a new entry */    
        entry = SLPDAEntryCreate(addr,bootstamp,scopelist,scopelistlen);
        SLPListLinkHead(&G_KnownDAList,(SLPListItem*)entry);

        /* Log that we are adding a new known DA */
        SLPDLogKnownDA("Added",&(entry->daaddr));

        /* Register all services with the new DA */
        SLPDKnownDARegisterAll(entry); 
    }

    return entry;
}


/*=========================================================================*/
void SLPDKnownDARemove(SLPDAEntry* daentry)
/* Remove the specified entry from the list of KnownDAs                    */
/*                                                                         */
/* daentry (IN) the entry to remove.                                       */
/*                                                                         */
/* Warning! memory pointed to by daentry will be freed                     */
/*=========================================================================*/
{
    SLPDLogKnownDA("Removed",&(daentry->daaddr));
    SLPDAEntryFree((SLPDAEntry*)SLPListUnlink(&G_KnownDAList,
                   (SLPListItem*)daentry));
}


/*=========================================================================*/
void SLPDKnownDAEcho(struct sockaddr_in* peeraddr,
                     SLPMessage msg,
                     SLPBuffer buf)
/* Echo a srvreg message to a known DA                                     */
/*									                                       */
/* peerinfo (IN) the peer that the registration came from                  */    
/*                                                                         */ 
/* msg (IN) the translated message to echo                                 */
/*                                                                         */
/* buf (IN) the message buffer to echo                                     */
/*                                                                         */
/* Returns:  none                                                          */
/*=========================================================================*/
{
    SLPDAEntry* daentry;
    SLPDSocket* sock;
    SLPBuffer   dup;
    const char* msgscope;
    int         msgscopelen;
    
    /* TODO: make sure that we do not echo to ourselves */
    
    if(msg->header.functionid == SLP_FUNCT_SRVREG)
    {
        msgscope = msg->body.srvreg.scopelist;
        msgscopelen = msg->body.srvreg.scopelistlen;
    }
    else if(msg->header.functionid == SLP_FUNCT_SRVDEREG)
    {
        msgscope = msg->body.srvdereg.scopelist;
        msgscopelen = msg->body.srvdereg.scopelistlen;
    }
    else
    {
        /* We only echo SRVREG and SRVDEREG */
        return;
    }

    daentry = (SLPDAEntry*)G_KnownDAList.head;
    while (daentry)
    {
        /*-----------------------------------------------------*/
        /* Do not echo to agents that do not support the scope */
        /*-----------------------------------------------------*/
        if(SLPIntersectStringList(daentry->scopelistlen,
                                  daentry->scopelist,
                                  msgscopelen,
                                  msgscope) )
        {
            /*------------------------------------------*/
            /* Load the socket with the message to send */
            /*------------------------------------------*/
            sock = SLPDOutgoingConnect(&daentry->daaddr);
            if(sock)
            {
                dup = SLPBufferDup(buf);
                if (dup)
                {
                    if (sock->state == STREAM_CONNECT_IDLE)
                    {
                        sock->state = STREAM_WRITE_FIRST;
                    }
                    SLPListLinkTail(&(sock->sendlist),(SLPListItem*)dup);
                }
            }
        }

        daentry = (SLPDAEntry*)daentry->listitem.next;
    }
}


/*=========================================================================*/
void SLPDKnownDAActiveDiscovery()
/* Add a socket to the outgoing list to do active DA discovery SrvRqst     */
/*									                                       */
/* Returns:  none                                                          */
/*=========================================================================*/
{
    size_t          bufsize;
    SLPDAEntry*     daentry;
    SLPDSocket*     sock;
    char*           prlist;
    size_t          prlistlen;
    struct in_addr  peeraddr;
    
    if(G_SlpdProperty.activeDiscoveryAttempts <= 0)
    {
        return;
    }
    G_SlpdProperty.activeDiscoveryAttempts --;   

    /*--------------------------------------------------*/
    /* Create new DATAGRAM socket with appropriate peer */
    /*--------------------------------------------------*/
    if(G_SlpdProperty.isBroadcastOnly == 0)
    {
        peeraddr.s_addr = htonl(SLP_MCAST_ADDRESS);
        sock = SLPDSocketCreateDatagram(&peeraddr,DATAGRAM_MULTICAST);
    }
    else
    {   
        peeraddr.s_addr = htonl(SLP_BCAST_ADDRESS);
        sock = SLPDSocketCreateDatagram(&peeraddr,DATAGRAM_BROADCAST);
    }
    if(sock == 0)
    {
        /* Could not create socket */
        return;
    }
    
    /*-------------------------------------------------*/
    /* Generate a DA service request buffer to be sent */
    /*-------------------------------------------------*/
    /* determine the size of the fixed portion of the SRVRQST         */
    bufsize  = 47;  /* 14 bytes for the header                        */
                    /*  2 bytes for the prlistlen                     */
                    /*  2 bytes for the srvtype length                */ 
                    /* 23 bytes for "service:directory-agent" srvtype */
                    /*  2 bytes for scopelistlen                      */
                    /*  2 bytes for predicatelen                      */
                    /*  2 bytes for sprstrlen                         */
    
    /* figure out what our Prlist will be by going through our list of  */
    /* known DAs                                                        */
    prlistlen = 0;
    prlist = malloc(SLP_MAX_DATAGRAM_SIZE);
    if(prlist == 0)
    {
        /* out of memory */
        return;
    }
    
    *prlist = 0;
    daentry = (SLPDAEntry*)G_KnownDAList.head;
    while(daentry && prlistlen < SLP_MAX_DATAGRAM_SIZE)
    {
        strcat(prlist,inet_ntoa(daentry->daaddr));
        daentry = (SLPDAEntry*)daentry->listitem.next;
        if(daentry)
        {
            strcat(prlist,",");
        }
        prlistlen = strlen(prlist);
    }                                  

    /* Allocate the send buffer */
    bufsize += G_SlpdProperty.localeLen + prlistlen;
    sock->sendbuf = SLPBufferRealloc(sock->sendbuf,bufsize);
    if(sock->sendbuf == 0)
    {
        /* out of memory */
        return;
    }                            

    /*------------------------------------------------------------*/
    /* Build a buffer containing the fixed portion of the SRVRQST */
    /*------------------------------------------------------------*/
    /*version*/
    *(sock->sendbuf->start)       = 2;
    /*function id*/
    *(sock->sendbuf->start + 1)   = SLP_FUNCT_SRVRQST;
    /*length*/
    ToUINT24(sock->sendbuf->start + 2, bufsize);
    /*flags*/
    ToUINT16(sock->sendbuf->start + 5,  SLP_FLAG_MCAST);
    /*ext offset*/
    ToUINT24(sock->sendbuf->start + 7,0);
    /*xid*/
    ToUINT16(sock->sendbuf->start + 10, 0);  /* TODO: generate a real XID */
    /*lang tag len*/
    ToUINT16(sock->sendbuf->start + 12, G_SlpdProperty.localeLen);
    /*lang tag*/
    memcpy(sock->sendbuf->start + 14,
           G_SlpdProperty.locale,
           G_SlpdProperty.localeLen);
    sock->sendbuf->curpos = sock->sendbuf->start + G_SlpdProperty.localeLen + 14;
    /* Prlist */
    ToUINT16(sock->sendbuf->curpos,prlistlen);
    sock->sendbuf->curpos = sock->sendbuf->curpos + 2;
    memcpy(sock->sendbuf->curpos,prlist,prlistlen);
    sock->sendbuf->curpos = sock->sendbuf->curpos + prlistlen;
    /* service type */
    ToUINT16(sock->sendbuf->curpos,23);                                         
    sock->sendbuf->curpos = sock->sendbuf->curpos + 2;
    memcpy(sock->sendbuf->curpos,"service:directory-agent",23);
    sock->sendbuf->curpos = sock->sendbuf->curpos + 23;
    /* scope list zero length */
    ToUINT16(sock->sendbuf->curpos,0);
    sock->sendbuf->curpos = sock->sendbuf->curpos + 2;
    /* predicate zero length */
    ToUINT16(sock->sendbuf->curpos,0);
    sock->sendbuf->curpos = sock->sendbuf->curpos + 2;
    /* spi list zero length */
    ToUINT16(sock->sendbuf->curpos,0);
    sock->sendbuf->curpos = sock->sendbuf->curpos + 2;
    
    /*-------------------------------------*/
    /* Add the socket to the outgoing list */
    /*-------------------------------------*/
    SLPDOutgoingDatagramWrite(sock); 
}


/*=========================================================================*/
void SLPDKnownDAPassiveDiscovery(int seconds)
/* Send passive discovery packets if properly configured and running as    */
/* a DA                                                                    */
/*	                                                                       */
/* seconds (IN) number seconds that elapsed since the last call to this    */
/*              function                                                   */
/* Returns:  none                                                          */
/*=========================================================================*/
{
    /* TODO: finish this function */
}
