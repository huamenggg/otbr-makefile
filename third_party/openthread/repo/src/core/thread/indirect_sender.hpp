/*
 *  Copyright (c) 2019, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes definitions for handling indirect transmission.
 */

#ifndef INDIRECT_SENDER_HPP_
#define INDIRECT_SENDER_HPP_

#include "openthread-core-config.h"

#include "common/locator.hpp"
#include "common/message.hpp"
#include "mac/data_poll_handler.hpp"
#include "mac/mac_frame.hpp"
#include "thread/src_match_controller.hpp"

namespace ot {

/**
 * @addtogroup core-mesh-forwarding
 *
 * @brief
 *   This module includes definitions for handling indirect transmissions.
 *
 * @{
 */

class Child;

/**
 * This class implements indirect transmission.
 *
 */
class IndirectSender : public InstanceLocator
{
    friend class Instance;
    friend class DataPollHandler::Callbacks;

public:
    /**
     * This class defines all the child info required for indirect transmission.
     *
     * `Child` class publicly inherits from this class.
     *
     */
    class ChildInfo
    {
        friend class IndirectSender;
        friend class SourceMatchController;

    public:
        /**
         * This method returns the number of queued messages for the child.
         *
         * @returns Number of queued messages for the child.
         *
         */
        uint16_t GetIndirectMessageCount(void) const { return mQueuedMessageCount; }

    private:
        Message *GetIndirectMessage(void) { return mIndirectMessage; }
        void     SetIndirectMessage(Message *aMessage) { mIndirectMessage = aMessage; }

        uint16_t GetIndirectFragmentOffset(void) const { return mIndirectFragmentOffset; }
        void     SetIndirectFragmentOffset(uint16_t aFragmentOffset) { mIndirectFragmentOffset = aFragmentOffset; }

        bool GetIndirectTxSuccess(void) const { return mIndirectTxSuccess; }
        void SetIndirectTxSuccess(bool aTxStatus) { mIndirectTxSuccess = aTxStatus; }

        bool IsIndirectSourceMatchShort(void) const { return mUseShortAddress; }
        void SetIndirectSourceMatchShort(bool aShort) { mUseShortAddress = aShort; }

        bool IsIndirectSourceMatchPending(void) const { return mSourceMatchPending; }
        void SetIndirectSourceMatchPending(bool aPending) { mSourceMatchPending = aPending; }

        void IncrementIndirectMessageCount(void) { mQueuedMessageCount++; }
        void DecrementIndirectMessageCount(void) { mQueuedMessageCount--; }
        void ResetIndirectMessageCount(void) { mQueuedMessageCount = 0; }

        bool IsWaitingForMessageUpdate(void) const { return mWaitingForMessageUpdate; }
        void SetWaitingForMessageUpdate(bool aNeedsUpdate) { mWaitingForMessageUpdate = aNeedsUpdate; }

        const Mac::Address &GetMacAddress(Mac::Address &aMacAddress) const;

        Message *mIndirectMessage;             // Current indirect message.
        uint16_t mIndirectFragmentOffset : 14; // 6LoWPAN fragment offset for the indirect message.
        bool     mIndirectTxSuccess : 1;       // Indicates tx success/failure of current indirect message.
        bool     mWaitingForMessageUpdate : 1; // Indicates waiting for updating the indirect message.
        uint16_t mQueuedMessageCount : 14;     // Number of queued indirect messages for the child.
        bool     mUseShortAddress : 1;         // Indicates whether to use short or extended address.
        bool     mSourceMatchPending : 1;      // Indicates whether or not pending to add to src match table.

        OT_STATIC_ASSERT(OPENTHREAD_CONFIG_NUM_MESSAGE_BUFFERS < (1UL << 14),
                         "mQueuedMessageCount cannot fit max required!");
    };

    /**
     * This constructor initializes the object.
     *
     * @param[in]  aInstance  A reference to the OpenThread instance.
     *
     */
    explicit IndirectSender(Instance &aInstance);

    /**
     * This method enables indirect transmissions.
     *
     */
    void Start(void) { mEnabled = true; }

    /**
     * This method disables indirect transmission.
     *
     * Any previously scheduled indirect transmission is canceled.
     *
     */
    void Stop(void);

    /**
     * This method adds a message for indirect transmission to a sleepy child.
     *
     * @param[in] aMessage  The message to add.
     * @param[in] aChild    The (sleepy) child for indirect transmission.
     *
     * @retval OT_ERROR_NONE           Successfully added the message for indirect transmission.
     * @retval OT_ERROR_ALREADY        The message was already added for indirect transmission to same child.
     * @retval OT_ERROR_INVALID_STATE  The child is not sleepy.
     *
     */
    otError AddMessageForSleepyChild(Message &aMessage, Child &aChild);

    /**
     * This method removes a message for indirect transmission to a sleepy child.
     *
     * @param[in] aMessage  The message to update.
     * @param[in] aChild    The (sleepy) child for indirect transmission.
     *
     * @retval OT_ERROR_NONE           Successfully removed the message for indirect transmission.
     * @retval OT_ERROR_NOT_FOUND      The message was not scheduled for indirect transmission to the child.
     *
     */
    otError RemoveMessageFromSleepyChild(Message &aMessage, Child &aChild);

    /**
     * This method removes all added messages for a specific child and frees message (with no indirect/direct tx).
     *
     * @param[in]  aChild  A reference to a child whose messages shall be removed.
     *
     */
    void ClearAllMessagesForSleepyChild(Child &aChild);

    /**
     * This method sets whether to use the extended or short address for a child.
     *
     * @param[in] aChild            A reference to the child.
     * @param[in] aUseShortAddress  `true` to use short address, `false` to use extended address.
     *
     */
    void SetChildUseShortAddress(Child &aChild, bool aUseShortAddress);

private:
    enum
    {
        /**
         * Indicates whether to set/enable 15.4 ack request in the MAC header of a supervision message.
         *
         */
        kSupervisionMsgAckRequest = (OPENTHREAD_CONFIG_SUPERVISION_MSG_NO_ACK_REQUEST == 0) ? true : false,
    };

    // Callbacks from DataPollHandler
    otError PrepareFrameForChild(Mac::Frame &aFrame, Child &aChild);
    void    HandleSentFrameToChild(const Mac::Frame &aFrame, otError aError, Child &aChild);
    void    HandleFrameChangeDone(Child &aChild);

    void     UpdateIndirectMessage(Child &aChild);
    Message *FindIndirectMessage(Child &aChild);
    void     RequestMessageUpdate(Child &aChild);
    uint16_t PrepareDataFrame(Mac::Frame &aFrame, Child &aChild, Message &aMessage);
    void     PrepareEmptyFrame(Mac::Frame &aFrame, Child &aChild, bool aAckRequest);
    void     ClearMessagesForRemovedChildren(void);

    bool                  mEnabled;
    SourceMatchController mSourceMatchController;
    DataPollHandler       mDataPollHandler;
    uint16_t              mMessageNextOffset;
};

/**
 * @}
 *
 */

} // namespace ot

#endif // INDIRECT_SENDER_HPP_
