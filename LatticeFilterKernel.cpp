//
// Created by Miguel Monteiro on 29/01/2018.
//

#include "LatticeFilterKernel.h"
#include "PermutohedralLatticeCPU.h"
#include "cstdio"
#include "tensorflow/core/framework/op_kernel.h"

using namespace tensorflow;

#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "cuda_runtime.h"

using namespace tensorflow;

REGISTER_OP("LatticeFilter")
        .Attr("T: {float32, float64}")
        .Attr("reverse: bool = false")
        .Attr("bilateral: bool = true")
        .Attr("theta_alpha: float = 1.0")
        .Attr("theta_beta: float = 1.0")
        .Attr("theta_gamma: float = 1.0")
        .Input("input_image: T")
        .Input("reference_image: T")
        .Output("output: T")
        .SetShapeFn([](::tensorflow::shape_inference::InferenceContext* c) {
            c->set_output(0, c->input(0));
            return Status::OK();
        });


using CPUDevice = Eigen::ThreadPoolDevice;
using GPUDevice = Eigen::GpuDevice;

/*
// CPU specialization of actual computation.
template <typename T>
struct LatticeFilter<CPUDevice, T> {
    void operator()(const CPUDevice& d,
                    T * output,
                    const T *input,
                    const T *reference_image,
                    int num_super_pixels,
                    int n_spatial_dims,
                    int *spatial_dims,
                    int n_input_channels,
                    int n_reference_channels,
                    float theta_alpha,
                    float theta_beta,
                    bool reverse) {

        int pd = n_reference_channels + n_spatial_dims;
        int vd = n_input_channels + 1;
        int n = num_super_pixels;



        auto positions= new float[num_super_pixels * pd];

        compute_bilateral_kernel_cpu(reference_image,
                                     positions,
                                     num_super_pixels,
                                     n_reference_channels,
                                     n_spatial_dims,
                                     spatial_dims,
                                     theta_alpha,
                                     theta_beta);

        lattice_filter_cpu(output, input, positions, pd, vd, n);

        delete[] positions;
    }
};
*/

// OpKernel definition.
// template parameter <T> is the datatype of the tensors.
template <typename Device, typename T>
class LatticeFilterOp : public OpKernel {
public:
    explicit LatticeFilterOp(OpKernelConstruction* context) : OpKernel(context) {
        OP_REQUIRES_OK(context, context->GetAttr("reverse", &reverse));
        OP_REQUIRES_OK(context, context->GetAttr("bilateral", &bilateral));
        OP_REQUIRES_OK(context, context->GetAttr("theta_alpha", &theta_alpha));
        OP_REQUIRES_OK(context, context->GetAttr("theta_beta", &theta_beta));
        OP_REQUIRES_OK(context, context->GetAttr("theta_gamma", &theta_gamma));
    }

    void Compute(OpKernelContext* context) override {
        // Grab the input tensor
        const Tensor& input_tensor = context->input(0);
        const Tensor& reference_image_tensor = context->input(1);


        // Create an output tensor
        Tensor* output_tensor = nullptr;
        OP_REQUIRES_OK(context, context->allocate_output(0, input_tensor.shape(), &output_tensor));

        // Do the computation.
        OP_REQUIRES(context, input_tensor.NumElements() <= tensorflow::kint32max,
                    errors::InvalidArgument("Too many elements in tensor"));

        // calculate dimensions; assumes channel is last dimension
        int rank = input_tensor.dims();
        int n_spatial_dims = rank -1;
        auto input_channels = static_cast<int>(input_tensor.dim_size(n_spatial_dims));

        auto spatial_dims = new int[rank-1];

        int num_super_pixels{1};
        for (int i = 0; i < n_spatial_dims; i++){
            num_super_pixels *= input_tensor.dim_size(i);
            spatial_dims[i] = static_cast<int>(input_tensor.dim_size(i));
        }


        int* spatial_dims_gpu;
        cudaMalloc((void**)&(spatial_dims_gpu), n_spatial_dims*sizeof(int));
        cudaMemcpy(spatial_dims_gpu, spatial_dims, n_spatial_dims*sizeof(int), cudaMemcpyHostToDevice);

        /*long long * a = input_tensor.shape().dim_sizes().data(); //this is only on CPU
        for(int i = 0; i < 2; i++){
            printf("%d\n ", static_cast<int>(a[i]));
        }*/

        auto n_input_channels = static_cast<int>(input_tensor.dim_size(rank - 1));
        vd = n_input_channels + 1;

        T spatial_std;
        T features_std;
        int n_reference_channels;

        if(bilateral){
            assert(reference_image_tensor.dims() ==  rank);
            n_reference_channels = static_cast<int>(reference_image_tensor.dim_size(rank - 1));
            pd = n_reference_channels + n_spatial_dims;
            spatial_std = theta_alpha;
            features_std = theta_beta;
        }else{
            pd = n_spatial_dims;
            n_reference_channels = 0; //set to zero so ComputeKernel does not use reference image channels
            spatial_std = theta_gamma;
            features_std = -1; //does not matter
        }


        // Allocate kernel positions and calculate them
        Tensor positions;
        OP_REQUIRES_OK(context, context->allocate_temp(DataTypeToEnum<T>::v(),
                                                       TensorShape({num_super_pixels * pd}), &positions));


        ComputeKernel<Device, T>()(context->eigen_device<Device>(),
                                   reference_image_tensor.flat<T>().data(),
                                   positions.flat<T>().data(),
                                   num_super_pixels,
                                   n_spatial_dims,
                                   spatial_dims_gpu,
                                   n_reference_channels,
                                   spatial_std,
                                   features_std);


        LatticeFilter<Device, T>()(context->eigen_device<Device>(),
                                   output_tensor->flat<T>().data(),
                                   input_tensor.flat<T>().data(),
                                   positions.flat<T>().data(),
                                   num_super_pixels,
                                   pd,
                                   vd,
                                   reverse);

        cudaFree(spatial_dims_gpu);
        delete[](spatial_dims);
    }
private:
    bool reverse;
    bool bilateral;
    float theta_alpha;
    float theta_beta;
    float theta_gamma;

    int pd;
    int vd;

};

// Register the CPU kernels.
#define REGISTER_CPU(T) REGISTER_KERNEL_BUILDER(Name("Bilateral").Device(DEVICE_CPU).TypeConstraint<T>("T"), BilateralOp<CPUDevice, T>);

//REGISTER_CPU(float);
//REGISTER_CPU(int32);

// Register the GPU kernels.
#ifdef GOOGLE_CUDA
/* Declare explicit instantiations in kernel_example.cu.cc. */
extern template struct LatticeFilter<GPUDevice, float>;
extern template struct LatticeFilter<GPUDevice, double>;

#define REGISTER_GPU(T) REGISTER_KERNEL_BUILDER(Name("LatticeFilter").Device(DEVICE_GPU).TypeConstraint<T>("T"), LatticeFilterOp<GPUDevice, T>);

REGISTER_GPU(float);
REGISTER_GPU(double);
#endif  // GOOGLE_CUDA