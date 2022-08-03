#include <array>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <stack>
#include <string_view>
#include <string>
#include <unordered_set>
#include <utility>


class mk_join_c
{
public:
	mk_join_c(std::string_view const& curr, std::string_view const& input, std::string_view const& output);
public:
	void join();
private:
	void process_frame();
private:
	typedef std::array<char, 4 * 1024> line_buff_t;
	struct frame_t
	{
		std::string m_name;
		std::filesystem::path m_path;
		std::ifstream m_ifs;
		long m_line_nr;
		int m_depth;
	};
private:
	std::ofstream m_ofs;
	std::unordered_set<std::string> m_guards;
	std::stack<frame_t> m_frames;
};


mk_join_c::mk_join_c(std::string_view const& curr, std::string_view const& input, std::string_view const& output) :
	m_ofs(output.data(), std::ios_base::out | std::ios_base::trunc),
	m_guards(),
	m_frames()
{
	m_frames.emplace();
	auto& top = m_frames.top();
	top.m_name = input;
	top.m_path = std::filesystem::path{curr}.remove_filename().append(input);
	top.m_ifs = std::ifstream{top.m_path};
	top.m_line_nr = 0;
	top.m_depth = 0;
}

void mk_join_c::join()
{
	assert(!m_frames.empty());
	do
	{
		process_frame();
	}while(!m_frames.empty());
}

void mk_join_c::process_frame()
{
	using namespace std::literals::string_view_literals;

	assert(!m_frames.empty());
	auto& frame = m_frames.top();
	bool visited = false;
	line_buff_t line_buff;
	std::string_view guard;
	while(frame.m_ifs)
	{
		++frame.m_line_nr;
		unsigned line_len;
		frame.m_ifs.getline(line_buff.data(), line_buff.size());
		auto line_len_ = frame.m_ifs.gcount();
		if(line_len_ != 0) --line_len_;
		line_len = (unsigned)line_len_;
		std::string_view line_view{line_buff.data(), line_len};
		if(frame.m_line_nr == 1)
		{
			m_ofs << std::format("/* Begin of mk_join-ed \"{}\" depth {}. */\n"sv, frame.m_name, frame.m_depth);
			static auto const s_guard_prefix_1 = "#ifndef "sv;
			if(line_view.starts_with(s_guard_prefix_1))
			{
				guard = line_view;
				guard.remove_prefix(s_guard_prefix_1.length());
				if(!guard.contains(' '))
				{
					auto const itb = m_guards.emplace(guard);
					visited = !itb.second;
				}
			}
		}
		else if(frame.m_line_nr == 2)
		{
			if(visited)
			{
				static auto const s_guard_prefix_2 = "#define "sv;
				if(line_view.starts_with(s_guard_prefix_2) && line_view.ends_with(guard))
				{
					m_ofs << "/* Canceled by mk_join. */\n"sv;
					m_ofs << "#endif\n"sv;
					break;
				}
			}
		}
		static constexpr auto prefix = "#include \""sv;
		static constexpr auto suffix = "\""sv;
		if(line_view.starts_with(prefix) && line_view.ends_with(suffix))
		{
			line_view.remove_prefix(prefix.size());
			line_view.remove_suffix(suffix.size());
			auto const depth = frame.m_depth;
			auto path = frame.m_path;
			m_frames.emplace();
			auto& top = m_frames.top();
			top.m_name = line_view;
			top.m_path = std::move(path).remove_filename().append(line_view);
			top.m_ifs = std::ifstream{top.m_path, std::ios_base::in};
			top.m_line_nr = 0;
			top.m_depth = depth + 1;
			return;
		}
		m_ofs << line_view << '\n';
	}
	m_ofs << std::format("/* End of mk_join-ed \"{}\" depth {}. */\n"sv, frame.m_name, frame.m_depth);
	m_frames.pop();
}


int mk_join(int argc, char const* const* argv)
{
	using namespace std::literals::string_view_literals;
	if(argc != 3)
	{
		return EXIT_FAILURE;
	}
	std::string_view const curr = "."sv;
	char const* const input = argv[1];
	char const* const output = argv[2];
	mk_join_c join{"."sv, input, output};;
	join.join();
	return EXIT_SUCCESS;
}


int main(int argc, char const* const* argv)
{
	return mk_join(argc, argv);
}
