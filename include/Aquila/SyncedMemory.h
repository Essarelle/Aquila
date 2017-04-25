#pragma once
#include "Aquila/Detail/Export.hpp"
#include "Aquila/rcc/external_includes/cv_core.hpp"
#include <opencv2/core/cuda.hpp>
#include <MetaObject/Parameters/TypedInputParameter.hpp>
namespace mo
{
    class Context;
}

namespace aq
{
    template<typename T> struct FN : public T
    {
        template<class...U> FN(int frame_number_ = 0) : frame_number(frame_number_) {}
        template<class...U> FN(int frame_number_, const U&... args) : frame_number(frame_number_), T(args...) {}
        FN& operator=(const T& other)
        {
            T::operator=(other);
            return *this;
        }
        template<class A> void serialize(A& ar)
        {
            ar(frame_number);
            ar(*static_cast<T*>(this));
        }

        int frame_number;
    };
    template<typename T> struct TS: public T
    {
        template<class...U> TS(U...args):T(args...)
        {
            timestamp = 0.0 * mo::second;
            frame_number = 0;
        }
        template<class...U> TS(mo::time_t ts, size_t fn, U...args) : T(args...)
        {
            timestamp = ts;
            this->frame_number = fn;
        }
        template<class A> void serialize(A& ar)
        {
            ar(frame_number);
            ar(timestamp);
            ar(*static_cast<T*>(this));
        }

        mo::time_t timestamp;
        size_t frame_number;
    };


    class AQUILA_EXPORTS SyncedMemory
    {
    public:
        enum SYNC_STATE
        {
            SYNCED = 0,
            HOST_UPDATED,
            DEVICE_UPDATED,
            DO_NOT_SYNC
        };
        SyncedMemory();
        SyncedMemory(const cv::Mat& h_mat, const cv::cuda::GpuMat& d_mat);
        SyncedMemory(const cv::Mat& h_mat);
        SyncedMemory(const cv::cuda::GpuMat& d_mat);
        SyncedMemory(const std::vector<cv::cuda::GpuMat> & d_mats);
        SyncedMemory(const std::vector<cv::Mat>& h_mats);
        SyncedMemory(cv::MatAllocator* cpu_allocator, cv::cuda::GpuMat::Allocator* gpu_allocator);
        SyncedMemory(const std::vector<cv::Mat>& h_mat, const std::vector<cv::cuda::GpuMat>& d_mat, SYNC_STATE state = SYNCED);
        SyncedMemory(const std::vector<cv::Mat>& h_mat, const std::vector<cv::cuda::GpuMat>& d_mat, const std::vector<SYNC_STATE> state);
        SyncedMemory clone(cv::cuda::Stream& stream);

        const cv::Mat&                         GetMat(cv::cuda::Stream& stream, int idx = 0) const;
        cv::Mat&                               GetMatMutable(cv::cuda::Stream& stream, int idx = 0);
        const cv::Mat&                         GetMatNoSync(int idx = 0) const;

        const cv::cuda::GpuMat&                GetGpuMat(cv::cuda::Stream& stream, int idx = 0) const;
        cv::cuda::GpuMat&                      GetGpuMatMutable(cv::cuda::Stream& stream, int idx = 0);
        const cv::cuda::GpuMat&                GetGpuMatNoSync(int idx = 0) const;

        const std::vector<cv::Mat>&            GetMatVec(cv::cuda::Stream& stream) const;
        std::vector<cv::Mat>&                  GetMatVecMutable(cv::cuda::Stream& stream);

        const std::vector<cv::cuda::GpuMat>&   GetGpuMatVec(cv::cuda::Stream& stream) const;
        std::vector<cv::cuda::GpuMat>&         GetGpuMatVecMutable(cv::cuda::Stream& stream);

        SYNC_STATE                             GetSyncState(int index = 0) const;

        mo::Context*                           GetContext() const;
        void                                   SetContext(mo::Context* ctx);

        bool Clone(cv::Mat& dest, cv::cuda::Stream& stream, int idx = 0) const;
        bool Clone(cv::cuda::GpuMat& dest, cv::cuda::Stream& stream, int idx = 0) const;

        void Synchronize(cv::cuda::Stream& stream = cv::cuda::Stream::Null()) const;
        void ResizeNumMats(int new_size = 1);
        void ReleaseGpu(cv::cuda::Stream& stream = cv::cuda::Stream::Null());

        int GetNumMats() const;
        bool empty() const;
        cv::Size GetSize() const;
        int GetChannels() const;
        std::vector<int> GetShape() const;
        int GetDim(int dim) const;
        int GetDepth() const;
        int GetType() const;
        int GetElemSize() const;
        template<typename A> void load(A& ar);
        template<typename A> void save(A & ar) const;
    private:
        struct impl
        {
            impl():
                _ctx(nullptr)
            {
            }
            std::vector<cv::Mat> h_data;
            std::vector<cv::cuda::GpuMat> d_data;
            std::vector<SyncedMemory::SYNC_STATE> sync_flags;
            mo::Context* _ctx;
        };

        std::shared_ptr<impl> _pimpl;
    };
}

namespace mo
{
template<>
class AQUILA_EXPORTS TypedInputParameterPtr<aq::SyncedMemory> : public ITypedInputParameter<aq::SyncedMemory>
{
public:
    TypedInputParameterPtr(const std::string& name = "", const aq::SyncedMemory** userVar_ = nullptr, Context* ctx = nullptr);
    bool SetInput(std::shared_ptr<IParameter> input);
    bool SetInput(IParameter* input);
    void SetUserDataPtr(const aq::SyncedMemory** user_var_);
    bool GetInput(boost::optional<mo::time_t> ts, size_t* fn = nullptr);
    bool GetInput(size_t fn, boost::optional<mo::time_t>* ts = nullptr);
protected:
    const aq::SyncedMemory** userVar; // Pointer to the user space pointer variable of type T
    void updateUserVar();
    virtual void onInputUpdate(Context* ctx, IParameter* param);
    virtual void onInputDelete(IParameter const* param);
};
}
