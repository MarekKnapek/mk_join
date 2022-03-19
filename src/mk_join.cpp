#include <array>
#include <filesystem>
#include <fstream>
#include <string_view>


using namespace std::literals::string_view_literals;


void recurse(std::string_view curr_file_name, std::string_view new_file_name, std::ostream& os, int depth)
{
	std::filesystem::path p{curr_file_name};
	p.remove_filename();
	p.append(new_file_name);
	std::ifstream ifs{p, std::ios_base::in};
	std::array<char, 512> line_buff;
	while(ifs)
	{
		ifs.getline(line_buff.data(), line_buff.size());
		auto line_len = std::strlen(line_buff.data());
		std::string_view line{line_buff.data(), line_len};
		static constexpr auto prefix = "#include \""sv;
		static constexpr auto suffix = "\""sv;
		if(!(line.starts_with(prefix) && line.ends_with(suffix)))
		{
			os << line;
			os << "\n";
			continue;
		}
		line.remove_prefix(prefix.size());
		line.remove_suffix(suffix.size());
		os << "/* Begin of mk_join-ed \""sv << line << "\" depth "sv << depth << " */\n"sv;
		recurse(p.string(), line, os, depth + 1);
		os << "/* End of mk_join-ed \""sv << line << "\" depth "sv << depth << " */\n"sv;
	}
}


int main(int argc, char const* const* argv)
{
	(void)argc;
	std::ofstream ofs{argv[2], std::ios_base::out | std::ios_base::trunc};
	recurse("."sv, argv[1], ofs, 0);
}
