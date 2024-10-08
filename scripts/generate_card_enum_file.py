import toml

def generate_enum_class(toml_file, output_file):
    # Load the TOML file
    with open(toml_file, 'r') as file:
        toml_data = toml.load(file)
    
    # Extract keys from the TOML file
    enum_names = [key for key in toml_data["card"].keys()]
    
    # Generate the C++ enum class code
    enum_code = ''
    enum_code += '// This file is generated by generate_card_enum_file.py\n'
    enum_code += '\n'
    enum_code += '#pragma once\n'
    enum_code += '\n'
    enum_code += '#include <cpp-dump.hpp>\n'
    enum_code += '\n'
    enum_code += 'namespace card {\n'
    enum_code += '	enum class Card {\n'
    enum_code += ',\n'.join(f'    	{name.title().replace(" ", "")}' for name in enum_names)
    enum_code += '\n	};\n'
    enum_code += '}\n'
    enum_code += '\n'

    # Generate the C++ code for exporting the enum class
    enum_code += 'namespace cpp_dump {\n'
    enum_code += 'namespace _detail {\n'
    enum_code += '	template <> inline constexpr bool _is_exportable_enum<card::Card> = true; template <> inline std::string export_enum(const card::Card& enum_const, const std::string&, std::size_t, std::size_t, bool, const export_command&) {\n'
    enum_code += '		static const std::unordered_map<card::Card, std::string_view> enum_to_string{\n'
    enum_code += ',\n'.join(f'			{{card::Card::{name.title().replace(" ", "")}, "{name.title().replace(" ", "")}"}}' for name in enum_names)
    enum_code += '       };\n'
    enum_code += '			return enum_to_string.count(enum_const) ? es::enumerator(enum_to_string.at(enum_const)) : es::class_name("card::Card") + es::op("::") + es::unsupported("?");\n'
    enum_code += '		}\n'
    enum_code += '	}\n'
    enum_code += '}\n'
    # Write the C++ code to the output file
    with open(output_file, 'w') as file:
        file.write(enum_code)

    print(f'Enum class code has been written to {output_file}')

# Example usage
toml_file = 'CR_Bot-CPP/constants/card.toml'
output_file = 'temp/CardEnum.h'
generate_enum_class(toml_file, output_file)
