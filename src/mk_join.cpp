#include <array>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <string_view>


using namespace std::literals::string_view_literals;


void recurse(std::string_view const& curr_file_name, std::string_view const& new_file_name, std::ostream& os, int depth)
{
	auto const p = std::filesystem::path{curr_file_name}.remove_filename().append(new_file_name);
	std::ifstream ifs{p, std::ios_base::in};
	std::array<char, 4 * 1024> line_buff;
	while(ifs)
	{
		ifs.getline(line_buff.data(), line_buff.size());
		auto line_len = ifs.gcount();
		if(line_len != 0) --line_len;
		std::string_view line{line_buff.data(), (unsigned)line_len};
		static constexpr auto prefix = "#include \""sv;
		static constexpr auto suffix = "\""sv;
		if(line.starts_with(prefix) && line.ends_with(suffix))
		{
			line.remove_prefix(prefix.size());
			line.remove_suffix(suffix.size());
			os << std::format("/* Begin of mk_join-ed \"{}\" depth {} */\n"sv, line, depth);
			recurse(p.string(), line, os, depth + 1);
			os << std::format("/* End of mk_join-ed \"{}\" depth {} */\n"sv, line, depth);
		}
		else
		{
			os << line << '\n';
		}
	}
}


int main(int argc, char const* const* argv)
{
	if(argc == 3)
	{
		std::string_view const curr = "."sv;
		char const* const input = argv[1];
		char const* const output = argv[2];
		std::ofstream ofs{output, std::ios_base::out | std::ios_base::trunc};
		recurse(curr, input, ofs, 0);
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}
