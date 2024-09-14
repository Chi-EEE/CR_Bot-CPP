#include <iostream>
#include <filesystem>

#include <boost/dll.hpp>
#include <magic_enum.hpp>
#include <magic_enum_utility.hpp>

#include "card/Loader.hpp"

int main() {
	auto program_dir = boost::dll::program_location().parent_path();
	auto cards_path = program_dir / "assets" / "cards";
	std::set<std::string> card_images;
	for (const auto& entry : std::filesystem::directory_iterator(cards_path.string()))
	{
		if (entry.is_directory())
			continue;
		std::string card_image = std::filesystem::relative(entry.path(), program_dir.string()).make_preferred().string();
		card_images.insert(card_image);
	}
	card::Loader card_loader;

	magic_enum::enum_for_each<card::Card>([&](auto val) {
		constexpr card::Card card_type = val;
		card::BaseCard card = card_loader.get_card(card_type);
		std::filesystem::path card_path = std::filesystem::path(card.path).make_preferred();
		if (!card_images.count(card_path.string())) {
			std::cerr << fmt::format("Card image not found: {}", card_path.string()) << std::endl;
		}
		else {
			card_images.erase(card_path.string());
		}
		if (card.evo_path.has_value())
		{
			auto card_evo_path = std::filesystem::path(card.evo_path.value()).make_preferred();
			if (!card_images.count(card_evo_path.string())) {
				std::cerr << fmt::format("Evo Card image not found: {}", card_evo_path.string()) << std::endl;
			}
			else {
				card_images.erase(card_evo_path.string());
			}
		}
		if (card.china) {
			std::filesystem::path card_china_path = card_path.replace_filename(
				(card_path
					.filename()
					.replace_extension("")
					.string() +
					"_china"
					) +
				card_path.extension().string()
			).make_preferred();
			if (!card_images.count(card_china_path.string())) {
				std::cerr << fmt::format("China Card image not found: {}", card_china_path.string()) << std::endl;
			}
			else {
				card_images.erase(card_china_path.string());
			}
		}
		}
	);

	for (const auto& card_image : card_images) {
		std::cerr << fmt::format("Unused card image: {}", card_image) << std::endl;
	}

	return 0;
}
