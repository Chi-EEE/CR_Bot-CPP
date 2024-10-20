#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>

#include "common/card/Cropper.hpp"
#include "common/card/BaseCard.hpp"
#include "munkres.hpp"

namespace card {
	// Constants
	constexpr int HAND_SIZE = 5;
	constexpr float MULTI_HASH_SCALE = 0.355f;
	constexpr int MULTI_HASH_INTERCEPT = 163;
	constexpr int HASH_SIZE = 8;
	constexpr float GREY_STD_THRESHOLD = 5.0f;

	class Detector {
	private:
		card::Cropper cropper;
		std::vector<card::BaseCard> cards;
		std::vector<std::vector<Eigen::MatrixXf>> card_hashes;

	public:
		Detector(card::Cropper cropper, const std::vector<card::BaseCard>& cards) {
			this->cropper = cropper;
			this->cards = cards;
			this->card_hashes = calculateCardHashes();
		}

		std::vector<card::BaseCard> detect_cards(std::vector<cv::Mat> cards) {
			Eigen::MatrixXf cropHashes(HASH_SIZE * HASH_SIZE, cards.size());

			for (int i = 0; i < cards.size(); ++i) {
				Eigen::MatrixXf hash = calculateHash(cards[i]);
				cropHashes.col(i) = Eigen::Map<Eigen::VectorXf>(hash.data(), hash.size());
			}

			// Use Munkres Algorithm to find the best match
			auto assignments = munkres_algorithm<float>(
				this->card_hashes.size(), cards.size(),
				[&](unsigned l, unsigned r) -> float {
					// Cost is the mean absolute difference between the hashes
					return (cropHashes.col(r) - Eigen::Map<Eigen::VectorXf>(this->card_hashes[l][0].data(), HASH_SIZE * HASH_SIZE)).cwiseAbs().mean();
				}
			);

			std::sort(assignments.begin(), assignments.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
				return a.second < b.second;
				}
			);

			std::vector<card::BaseCard> detectedCards;
			for (const auto& [card_index, order] : assignments) {
				detectedCards.push_back(this->cards[card_index]);
			}

			return detectedCards;
		}

		std::vector<card::BaseCard> run(std::vector<cv::Mat> card_images) {
			auto cards = detect_cards(card_images);
			return cards;
		}

	private:
		Eigen::MatrixXf calculateMultiHash(const cv::Mat& image) {
			Eigen::MatrixXf grayImage = calculateHash(image);

			// Light and dark transformations
			Eigen::MatrixXf lightImage = MULTI_HASH_SCALE * grayImage.array() + MULTI_HASH_INTERCEPT;
			Eigen::MatrixXf darkImage = (grayImage.array() - MULTI_HASH_INTERCEPT) / MULTI_HASH_SCALE;

			// Stack the hashes vertically into one matrix
			Eigen::MatrixXf multiHash(grayImage.rows() * 3, grayImage.cols());
			multiHash.topRows(grayImage.rows()) = grayImage;
			multiHash.middleRows(grayImage.rows(), grayImage.rows()) = lightImage;
			multiHash.bottomRows(grayImage.rows()) = darkImage;

			return multiHash;
		}

		Eigen::MatrixXf calculateHash(const cv::Mat& image) {
			cv::Mat card_hash_image;
			cv::cvtColor(image, card_hash_image, cv::COLOR_BGR2GRAY);
			cv::resize(card_hash_image, card_hash_image, cv::Size(HASH_SIZE, HASH_SIZE), 0, 0, cv::INTER_LINEAR);

			// Convert image to Eigen matrix
			Eigen::MatrixXf hashMatrix(HASH_SIZE, HASH_SIZE);
			for (int i = 0; i < HASH_SIZE; ++i) {
				for (int j = 0; j < HASH_SIZE; ++j) {
					hashMatrix(i, j) = static_cast<float>(card_hash_image.at<uchar>(i, j));
				}
			}

			return hashMatrix;
		}

		std::vector<std::vector<Eigen::MatrixXf>> calculateCardHashes() {
			std::vector<std::vector<Eigen::MatrixXf>> cardHashes(this->cards.size(), std::vector<Eigen::MatrixXf>(3));

			for (int i = 0; i < this->cards.size(); ++i) {
				std::string path = (std::filesystem::current_path() / this->cards[i].path).string();
				cv::Mat card_image = cv::imread(path, cv::IMREAD_COLOR);
				cv::resize(card_image, card_image, cv::Size(120, 150), 0, 0, cv::INTER_LINEAR);
				card_image = card_image(cv::Rect(this->cropper.inner_card_coords.x(), this->cropper.inner_card_coords.y(), this->cropper.inner_card_size.x(), this->cropper.inner_card_size.y()));
				cv::resize(card_image, card_image, cv::Size(HASH_SIZE, HASH_SIZE), 0, 0, cv::INTER_LINEAR);

				if (card_image.empty()) {
					std::cerr << "Error: Could not open or find the image!" << std::endl;
					continue;
				}

				Eigen::MatrixXf multiHash = calculateMultiHash(card_image);
				for (int j = 0; j < 3; ++j) {
					cardHashes[i][j] = multiHash.block(j * HASH_SIZE, 0, HASH_SIZE, HASH_SIZE);
				}
			}

			return cardHashes;
		}

	};
}