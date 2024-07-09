#ifndef RTCP_API_H
#define RTCP_API_H

/* Data types includes. */
#include "rtcp_data_types.h"

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_Init( RtcpContext_t * pCtx );

RtcpResult_t Rtcp_SerializeSenderReport( RtcpContext_t * pCtx,
                                         const RtcpSenderReport_t * pSenderReport,
                                         uint8_t * pBuffer,
                                         size_t * pBufferLength );

RtcpResult_t Rtcp_SerializeReceiverReport( RtcpContext_t * pCtx,
                                           const RtcpReceiverReport_t * pReceiverReport,
                                           uint8_t * pBuffer,
                                           size_t * pBufferLength );

RtcpResult_t Rtcp_DeSerializePacket( RtcpContext_t * pCtx,
                                     const uint8_t * pSerializedPacket,
                                     size_t serializedPacketLength,
                                     RtcpPacket_t * pRtcpPacket );

RtcpResult_t Rtcp_ParseFirPacket( RtcpContext_t * pCtx,
                                  const RtcpPacket_t * pRtcpPacket,
                                  RtcpFirPacket_t * pFirPacket );

RtcpResult_t Rtcp_ParsePliPacket( RtcpContext_t * pCtx,
                                  const RtcpPacket_t * pRtcpPacket,
                                  RtcpPliPacket_t * pPliPacket );

RtcpResult_t Rtcp_ParseSliPacket( RtcpContext_t * pCtx,
                                  const RtcpPacket_t * pRtcpPacket,
                                  RtcpSliPacket_t * pSliPacket );

RtcpResult_t Rtcp_ParseRembPacket( RtcpContext_t * pCtx,
                                   const RtcpPacket_t * pRtcpPacket,
                                   RtcpRembPacket_t * pRembPacket );

RtcpResult_t Rtcp_ParseSenderReport( RtcpContext_t * pCtx,
                                     const RtcpPacket_t * pRtcpPacket,
                                     RtcpSenderReport_t * pSenderReport );

RtcpResult_t Rtcp_ParseReceiverReport( RtcpContext_t * pCtx,
                                       const RtcpPacket_t * pRtcpPacket,
                                       RtcpReceiverReport_t * pReceiverReport );

RtcpResult_t Rtcp_ParseNackPacket( RtcpContext_t * pCtx,
                                   const RtcpPacket_t * pRtcpPacket,
                                   RtcpNackPacket_t * pNackPacket );

RtcpResult_t Rtcp_ParseTwccPacket( RtcpContext_t * pCtx,
                                   const RtcpPacket_t * pRtcpPacket,
                                   RtcpTwccPacket_t * pTwccPacket );

RtcpPacketType_t GetRtcpPacketType( uint8_t packetType,
                                    uint8_t fmt );

/*-----------------------------------------------------------*/

#endif /* RTCP_API_H */
