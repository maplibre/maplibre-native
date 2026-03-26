#include <mbgl/shaders/webgpu/wgsl_preprocessor.hpp>

#include <cctype>
#include <sstream>
#include <stack>
#include <unordered_map>

namespace mbgl {
namespace webgpu {
namespace detail {

bool isDirective(const std::string& line, const std::string& directive) {
    std::size_t pos = 0;
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
        ++pos;
    }
    if (pos >= line.size() || line[pos] != '#') {
        return false;
    }
    ++pos;
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) {
        ++pos;
    }
    if (line.compare(pos, directive.size(), directive) != 0) {
        return false;
    }
    pos += directive.size();
    return pos == line.size() || std::isspace(static_cast<unsigned char>(line[pos]));
}

std::string getDirectiveArgument(const std::string& line) {
    auto hash = line.find('#');
    if (hash == std::string::npos) {
        return {};
    }
    auto pos = line.find_first_not_of(" \t", hash + 1);
    pos = line.find_first_of(" \t", pos);
    if (pos == std::string::npos) {
        return {};
    }
    pos = line.find_first_not_of(" \t", pos);
    if (pos == std::string::npos) {
        return {};
    }
    const auto end = line.find_first_of(" \t\r\n", pos);
    return line.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

std::string preprocessWGSL(const std::string& source, const std::unordered_map<std::string, bool>& defines) {
    std::stringstream input(source);
    std::string line;
    std::string output;
    output.reserve(source.size());

    struct ConditionalFrame {
        bool parentActive;
        bool condition;
        bool elseSeen;
    };

    std::stack<ConditionalFrame> stack;
    bool currentActive = true;

    while (std::getline(input, line)) {
        if (isDirective(line, "ifdef")) {
            const auto symbol = getDirectiveArgument(line);
            const bool condition = defines.contains(symbol);
            stack.push(ConditionalFrame{currentActive, condition, false});
            currentActive = currentActive && condition;
            continue;
        }

        if (isDirective(line, "ifndef")) {
            const auto symbol = getDirectiveArgument(line);
            const bool condition = !defines.contains(symbol);
            stack.push(ConditionalFrame{currentActive, condition, false});
            currentActive = currentActive && condition;
            continue;
        }

        if (isDirective(line, "else")) {
            if (stack.empty()) {
                continue;
            }
            auto& frame = stack.top();
            if (!frame.elseSeen) {
                frame.elseSeen = true;
                currentActive = frame.parentActive && !frame.condition;
            } else {
                currentActive = false;
            }
            continue;
        }

        if (isDirective(line, "endif")) {
            if (stack.empty()) {
                continue;
            }
            currentActive = stack.top().parentActive;
            stack.pop();
            continue;
        }

        if (currentActive) {
            output.append(line);
            output.push_back('\n');
        }
    }

    return output;
}

} // namespace detail
} // namespace webgpu
} // namespace mbgl
