#pragma once
/**
 * https://github.com/pytorch/pytorch/blob/b2e79ed5ecabcf4be299dc2ed085223ab5c22fd7/torch/csrc/api/include/torch/data/datasets/mnist.h
 */
#include "els_nn_export.h"
#include <torch/data/datasets/base.h>
#include <torch/data/example.h>
#include <torch/types.h>


#include <cstddef>
#include <string>

/// The MNIST dataset.
class ELS_NN_EXPORT MNIST : public torch::data::datasets::Dataset<MNIST> {
public:
    /// The mode in which the dataset is loaded.
    enum class Mode {
        kTrain, kTest
    };

    /// Loads the MNIST dataset from the `root` path.
    ///
    /// The supplied `root` path should contain the *content* of the unzipped
    /// MNIST dataset, available from http://yann.lecun.com/exdb/mnist.
    explicit MNIST(const std::string &root, Mode mode = Mode::kTrain);

    /// Returns the `Example` at the given `index`.
    torch::data::Example<> get(size_t index) override;

    /// Returns the size of the dataset.
    torch::optional<size_t> size() const override;

    /// Returns true if this is the training subset of MNIST.
    // NOLINTNEXTLINE(bugprone-exception-escape)
    bool is_train() const noexcept;

    /// Returns all images stacked into a single tensor.
    const torch::Tensor &images() const;

    /// Returns all targets stacked into a single tensor.
    const torch::Tensor &targets() const;

private:
    torch::Tensor images_, targets_;
};
