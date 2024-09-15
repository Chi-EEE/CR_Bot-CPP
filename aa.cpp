#include <functional>
#include <vector>
#include <utility>
#include <Eigen/Dense>
#include "munkres.hpp"


class CardDetector {
public:
    CardDetector(int hash_size, const std::vector<cv::Mat>& card_hashes)
        : hash_size(hash_size), card_hashes(card_hashes) {}

    Eigen::VectorXf calculateHash(const cv::Mat& image) {
        cv::Mat resized, gray;
        cv::resize(image, resized, cv::Size(hash_size, hash_size), 0, 0, cv::INTER_LINEAR);
        cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);
        Eigen::VectorXf hash = Eigen::Map<Eigen::VectorXf>(gray.ptr<float>(), gray.total());
        return hash;
    }

    std::pair<std::vector<int>, std::vector<cv::Mat>> detectCards(const cv::Mat& image) {
        std::vector<cv::Mat> crops;
        for (const auto& position : CARD_CONFIG) {
            cv::Rect crop_rect(position[0], position[1], position[2], position[3]);
            crops.push_back(image(crop_rect));
        }

        Eigen::MatrixXf cropHashes(crops.size(), hash_size * hash_size);
        for (size_t i = 0; i < crops.size(); ++i) {
            cropHashes.row(i) = calculateHash(crops[i]);
        }

        // Calculate the hash differences between cropHashes and card_hashes
        Eigen::MatrixXf hashDiffs(crops.size(), card_hashes.size());
        for (size_t i = 0; i < crops.size(); ++i) {
            for (size_t j = 0; j < card_hashes.size(); ++j) {
                Eigen::VectorXf diff = cropHashes.row(i) - Eigen::Map<Eigen::VectorXf>(card_hashes[j].ptr<float>(), card_hashes[j].total());
                hashDiffs(i, j) = diff.cwiseAbs().mean();
            }
        }

        // Use the munkres_algorithm to get the optimal assignment
        auto assignments = munkres_algorithm<float>(
            crops.size(), card_hashes.size(),
            [&hashDiffs](unsigned l, unsigned r) -> float {
                return hashDiffs(l, r);
            }
        );

        // Extract the indices from the assignments
        std::vector<int> idx;
        for (const auto& assignment : assignments) {
            idx.push_back(assignment.second);  // Take the index from the right-hand side
        }

        return { idx, crops };
    }

private:
    int hash_size;
    std::vector<cv::Mat> card_hashes;
};

// CARD_CONFIG should be defined as a global or passed into the constructor, it holds crop positions
std::vector<std::vector<int>> CARD_CONFIG = {
    // Each element is a {x, y, width, height} for each crop
};
