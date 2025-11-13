#pragma once

#include "model.hpp"
#include "../base/type.hpp"

namespace wibot {

template <typename TIN, u8 CHANNELS_OUT, typename TOUT>
class PipelineAdapter : public SyncPipeline<TOUT> {
   public:
    PipelineAdapter(SyncPipeline<TIN>& upstream);
    ~PipelineAdapter();

    Result mapChannel(u8 channelOut, u8 channelIn);
    Result mapChannels(const u8 channelMap[CHANNELS_OUT]);

    void setUpstreamControl(bool enableUpstreamControl);
    bool getUpstreamControl() const;

    TOUT getValue(u8 channel) const override;
    void reset() override;
    void update() override;

   private:
    SyncPipeline<TIN>& _upstream;
    u8                 _channelIndex[CHANNELS_OUT];
    bool               _enableUpstreamControl;
};

}  // namespace wibot