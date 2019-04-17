#ifndef HEADER_CPARSER
#define HEADER_CPARSER

#include "raft_rpc.pb.h"

class Msg;
class CParser
{
public:
	static void EncodeHeartRequest(Msg& msg, ::raft_rpc::HeartRequest* request);
    static void EncodeHeartResponse(Msg& msg, ::raft_rpc::HeartResponse* response);
    static void DecodeHeartRequest(::raft_rpc::HeartRequest* request, Msg& msg);
	static void DecodeHeartResponse(::raft_rpc::HeartResponse* response, Msg& msg);

    static void EncodeVoteRequest(Msg& msg, ::raft_rpc::VoteResuest* request);
    static void EncodeVoteResponse(Msg& msg, ::raft_rpc::VoteToResponse* response);
    static void DecodeVoteRequest(::raft_rpc::VoteResuest* request, Msg& msg);
    static void DecodeVoteResponse(::raft_rpc::VoteToResponse* response, Msg& msg);

    static void EncodeSyncRequest(Msg& msg, ::raft_rpc::SyncToResuest* request);
    static void EncodeSyncResponse(Msg& msg, ::raft_rpc::SyncToResponse* response);
    static void DecodeSyncRequest(::raft_rpc::SyncToResuest* request, Msg& msg);
    static void DecodeSyncResponse(::raft_rpc::SyncToResponse* response, Msg& msg);
};

#endif
