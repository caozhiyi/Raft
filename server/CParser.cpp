#include "CParser.h"
#include "common.h"

void CParser::EncodeHeartRequest(Msg& msg, ::raft_rpc::HeartRequest* request) {
    request->set_version(msg._head._newest_version);
    request->set_done_msg(msg._head._type & DoneMsg);
    for (auto iter : msg._msg) {
        request->add_msg(*iter);
    }
}

void CParser::EncodeHeartResponse(Msg& msg, ::raft_rpc::HeartResponse* response) {
    request->set_version(msg._head._newest_version);
}

void CParser::DecodeHeartRequest(::raft_rpc::HeartRequest* request, Msg& msg) {
    msg._head._newest_version = request->version();
    msg._head._type = Heart;
    if (request->done_msg()) {
        msg._head._type |= DoneMsg;
    }
    int len = request->msg_size();
    for (int i = 0; i < len; i++) {
        msg._msg.push_back(request->msg(i));
    }
}

void CParser::DecodeHeartResponse(::raft_rpc::HeartResponse* response, Msg& msg) {
    msg._head._newest_version = request->version();
    msg._head._type = ReHeart;
}


void CParser::EncodeVoteRequest(Msg& msg, ::raft_rpc::VoteResuest* request) {
    
}

void CParser::EncodeVoteResponse(Msg& msg, ::raft_rpc::VoteToResponse* response) {
    response->set_vote(true);
}

void CParser::DecodeVoteRequest(::raft_rpc::VoteResuest* request, Msg& msg) {
    
}

void CParser::DecodeVoteResponse(::raft_rpc::VoteToResponse* response, Msg& msg) {
    if (response->vote()) {
        msg._head._type = Vote;
    }
}


void CParser::EncodeSyncRequest(Msg& msg, ::raft_rpc::SyncToResuest* request) {
    request->set_version(msg._head._newest_version);
}

void CParser::EncodeSyncResponse(Msg& msg, ::raft_rpc::SyncToResponse* response) {
    request->set_version(msg._head._newest_version);
}

void CParser::DecodeSyncRequest(::raft_rpc::SyncToResuest* request, Msg& msg) {
    msg._head._type = ToSync;
    msg._head._newest_version = request->version();
}

void CParser::DecodeSyncResponse(::raft_rpc::SyncToResponse* response, Msg& msg) {
    msg._head._type = Sync;
    msg._head._newest_version = response->version();
}
