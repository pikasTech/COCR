#include "object_detector_opencv_impl.hpp"

#include <opencv2/imgproc.hpp>
#include <ncnn/net.h>

#include <iostream>

bool xgd::ObjectDetectorOpenCVImpl::initModel(const std::string &_cfgFile, const std::string &_weightsFile) {
    try {
        net = cv::dnn::readNetFromDarknet(_cfgFile, _weightsFile);
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
//        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
//        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
        auto outLayers = net.getUnconnectedOutLayers();
        auto layersNames = net.getLayerNames();
        outBlobNames.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); i++) {
            outBlobNames[i] = layersNames[outLayers[i] - 1];
        }
        std::vector<cv::Mat> outs;
        std::vector<int> minSize = {1, 1, sizeBase, sizeBase};
        cv::Mat emptyBlob(4, minSize.data(), CV_32F);
        net.setInput(emptyBlob);
        net.forward(outs, outBlobNames);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;// FIXME: 不返回的话，gcc-Release编译报错
}

void xgd::ObjectDetectorOpenCVImpl::freeModel() {

}

std::pair<cv::Mat, std::vector<xgd::DetectorObject>>
xgd::ObjectDetectorOpenCVImpl::detect(const cv::Mat &_originImage) {
    cv::Mat blob;
    cv::Mat input = preProcess(_originImage);
    cv::dnn::blobFromImage(input, blob, 1 / 255.0);
    net.setInput(blob);
    std::vector<cv::Mat> outputBlobs;
    net.forward(outputBlobs, outBlobNames);
    blob.release();
    std::vector<float> confs;
    std::vector<cv::Rect2d> boxes;
    std::vector<int> labels;
    for (size_t i = 0; i < outputBlobs.size(); i++) {
        const auto &vec = outputBlobs[i];
        for (int j = 0; j < vec.rows; j++) {
            double maxProb;
            cv::Point2i maxLoc;
            minMaxLoc(vec.row(j).colRange(5, vec.cols),
                      nullptr, &maxProb, nullptr, &maxLoc);
            if (maxProb < confThresh) continue;
            const int &label = maxLoc.x;
            if (!DetectorObject::isValidLabel(label)) continue;
            float cx = vec.at<float>(j, 0) * input.cols;
            float cy = vec.at<float>(j, 1) * input.rows;
            float bw = vec.at<float>(j, 2) * input.cols;
            float bh = vec.at<float>(j, 3) * input.rows;
            boxes.emplace_back(cx - bw / 2, cy - bh / 2, bw, bh);
            labels.push_back(label);
            confs.push_back(maxProb);
        }
    }
    std::vector<int> selectedBoxIndices;
    cv::dnn::NMSBoxes(boxes, confs, confThresh, iouThresh, selectedBoxIndices);
    std::vector<DetectorObject> objects;
    for (const auto &i:selectedBoxIndices) {
        objects.emplace_back(boxes[i].x, boxes[i].y, boxes[i].width, boxes[i].height,
                             labels[i], confs[i]);
    }
    return {input, objects};
}

void xgd::ObjectDetectorOpenCVImpl::setConfThresh(float confThresh) {
    ObjectDetectorOpenCVImpl::confThresh = confThresh;
}

void xgd::ObjectDetectorOpenCVImpl::setIouThresh(float iouThresh) {
    ObjectDetectorOpenCVImpl::iouThresh = iouThresh;
}
