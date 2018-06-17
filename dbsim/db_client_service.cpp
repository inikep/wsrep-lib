//
// Copyright (C) 2018 Codership Oy <info@codership.com>
//

#include "db_client_service.hpp"
#include "db_client.hpp"

int db::client_service::apply(wsrep::client_state&,
                              const wsrep::const_buffer&)
{
    db::client* client(client_state_.client());
    client->se_trx_.start(client);
    client->se_trx_.apply(client_state_.transaction());
    return 0;
}

int db::client_service::commit(wsrep::client_state&,
                               const wsrep::ws_handle&,
                               const wsrep::ws_meta&)
{
    db::client* client(client_state_.client());
    int ret(client_state_.before_commit());
    if (ret == 0) client->se_trx_.commit();
    ret = ret || client_state_.ordered_commit();
    ret = ret || client_state_.after_commit();
    return ret;
}

int db::client_service::rollback(wsrep::client_state&)
{
    db::client* client(client_state_.client());
    int ret(client_state_.before_rollback());
    assert(ret == 0);
    client->se_trx_.rollback();
    ret = client_state_.after_rollback();
    assert(ret == 0);
    return ret;
}
enum wsrep::provider::status
db::client_service::replay(wsrep::client_state&,
                           wsrep::transaction_context& transaction_context)
{
    wsrep::high_priority_context high_priority_context(client_state_);
    auto ret(provider_.replay(transaction_context.ws_handle(),
                              &client_state_));
    if (ret == wsrep::provider::success)
    {
        ++client_state_.client()->stats_.replays;
    }
    return ret;
}