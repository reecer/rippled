//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012, 2013 Ripple Labs Inc.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#include <BeastConfig.h>
#include <ripple/app/tx/apply.h>
#include <ripple/app/tx/impl/ApplyContext.h>

#include <ripple/app/tx/impl/CancelOffer.h>
#include <ripple/app/tx/impl/CancelTicket.h>
#include <ripple/app/tx/impl/Change.h>
#include <ripple/app/tx/impl/CreateOffer.h>
#include <ripple/app/tx/impl/CreateTicket.h>
#include <ripple/app/tx/impl/Payment.h>
#include <ripple/app/tx/impl/SetAccount.h>
#include <ripple/app/tx/impl/SetRegularKey.h>
#include <ripple/app/tx/impl/SetSignerList.h>
#include <ripple/app/tx/impl/SetTrust.h>

namespace ripple {

template <class Processor,
    class... Args>
static
std::pair<TER, bool>
do_apply (Args&&... args)
{
    ApplyContext ctx (
        std::forward<Args>(args)...);
    Processor p(ctx);
    return p();
}

template <class... Args>
static
std::pair<TER, bool>
invoke (TxType type,
    Args&&... args)
{
    switch(type)
    {
    case ttACCOUNT_SET:     return do_apply< SetAccount    >(std::forward<Args>(args)...);
    case ttOFFER_CANCEL:    return do_apply< CancelOffer   >(std::forward<Args>(args)...);
    case ttOFFER_CREATE:    return do_apply< CreateOffer   >(std::forward<Args>(args)...);
    case ttPAYMENT:         return do_apply< Payment       >(std::forward<Args>(args)...);
    case ttREGULAR_KEY_SET: return do_apply< SetRegularKey >(std::forward<Args>(args)...);
    case ttSIGNER_LIST_SET: return do_apply< SetSignerList >(std::forward<Args>(args)...);
    case ttTICKET_CANCEL:   return do_apply< CancelTicket  >(std::forward<Args>(args)...);
    case ttTICKET_CREATE:   return do_apply< CreateTicket  >(std::forward<Args>(args)...);
    case ttTRUST_SET:       return do_apply< SetTrust      >(std::forward<Args>(args)...);

    // VFALCO These are both the same?
    case ttAMENDMENT:
    case ttFEE:             return do_apply< Change        >(std::forward<Args>(args)...);
    default:
        break;
    }
    return { temUNKNOWN, false };
}

std::pair<TER, bool>
apply (OpenView& view,
    STTx const& tx, ApplyFlags flags,
        Config const& config,
            beast::Journal j)
{
    try
    {
        return invoke (tx.getTxnType(),
            view, tx, flags, config, j);
    }
    catch(std::exception const& e)
    {
        JLOG(j.fatal) <<
            "Caught exception: " << e.what();
        return { tefEXCEPTION, false };
    }
    catch(...)
    {
        JLOG(j.fatal) <<
            "Caught unknown exception";
        return { tefEXCEPTION, false };
    }
}

} // ripple
