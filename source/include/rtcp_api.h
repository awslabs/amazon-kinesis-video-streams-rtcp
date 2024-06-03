#ifndef RTCP_API_H
#define RTCP_API_H

/* Data types includes. */
#include "rtcp_data_types.h"

RtcpResult_t Rtcp_Init( RtcpContext_t * pCtx );

RtcpResult_t Rtcp_Serialize( RtcpContext_t * pCtx,
                             const RtcpPacket_t * pRtcpPacket,
                             uint8_t * pBuffer,
                             size_t * pLength );

RtcpResult_t Rtcp_CreatePayloadSenderReport( RtcpContext_t * pCtx,
                                             RtcpPacket_t * pRtcpPacket,
                                             size_t paylaodLength,
                                             const RtcpSenderReport_t * pRtcpSenderReport );

RtcpResult_t Rtcp_DeSerialize( RtcpContext_t * pCtx,
                               uint8_t * pSerializedPacket,
                               size_t serializedPacketLength,
                               RtcpPacket_t * pRtcpPacket );

RtcpResult_t Rtcp_ParseFIRPacket( RtcpContext_t * pCtx,
                                  uint8_t * pPayload,
                                  size_t paylaodLength,
                                  uint32_t * pMediaSSRC );

RtcpResult_t Rtcp_ParseRembPacket( RtcpContext_t * pCtx,
                                   uint8_t * pPayload,
                                   size_t paylaodLength,
                                   size_t * pSsrcListLength,
                                   uint32_t ** pSsrcList,
                                   uint64_t * pBitRate );

#endif /* RTCP_API_H */