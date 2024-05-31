#include "config_loader.h"

#include <algorithm>
#include <expected>
#include <fstream>
#include <functional>
#include <ranges>
#include <string_view>
#include <tuple>

#include "toki/core/assert.h"
#include "toki/core/errors.h"
#include "toki/core/logging.h"
#include "toki/resources/resource_types.h"

namespace Toki {

std::expected<AttachmentsConfig, Error> parseAttachmentsConfig(std::string source, const std::filesystem::path& path);
std::expected<ShaderConfig, Error> parseShaderConfig(std::string source, const std::filesystem::path& path);

static std::array ws = { ' ', '\t', '\n', '\v', '\r', '\f' };
static auto isSpace = [](char c) { return std::ranges::any_of(ws, [c](char s) { return c == s; }); };

constexpr std::string trim(std::string_view in) {
    auto view = in | std::views::drop_while(isSpace) | std::views::reverse | std::views::drop_while(isSpace) | std::views::reverse;
    return std::string{ view.begin(), view.end() };
}

template <typename T>
static constexpr std::optional<T> extract(std::string_view val);

Error parseConfig(const std::filesystem::path& path, std::function<std::optional<std::string>(const std::string&, const std::string&)> handleFn) {
    std::ifstream file(path);

    if (!file.is_open() || !file.good()) {
        LOG_ERROR("Could not open file '{}'", path.string());
        return Error::FileOpenError;
    }

    static const uint32_t MAX_SUPPORTED_LINE_WIDTH = 256;
    char buffer[MAX_SUPPORTED_LINE_WIDTH];

    static auto logError = [](const std::filesystem::path& path, uint32_t lineNumber, std::string_view message) {
        LOG_ERROR("Error parsing file {}:{}:\n\t{}", path.string(), lineNumber, message);
    };

    bool foundError = false;

    for (uint32_t lineNumber = 1; file.good(); ++lineNumber) {
        file.getline(buffer, MAX_SUPPORTED_LINE_WIDTH);
        std::string trimmedLine = trim(buffer);
        buffer[0] = 0;

        if (trimmedLine.size() == 0 || trimmedLine.starts_with('#')) {
            continue;
        }

        auto splitLine = std::ranges::views::split(trimmedLine, '=');

        std::vector<std::string_view> splitVector{ splitLine.begin(), splitLine.end() };

        if (splitVector.size() != 2) {
            LOG_WARN("Skipping parsing line '{}' of file {}:{}", lineNumber, path.string(), lineNumber);
            continue;
        }

        if (auto result = handleFn(trim(splitVector[0]), trim(splitVector[1])); result.has_value()) {
            logError(path, lineNumber, result.value());
            foundError = true;
        }
    }

    if (foundError) {
        return Error::FileParseError;
    }

    return Error::NoError;
}

template <uint32_t Index, typename Tuple, typename Type, typename... OtherTypes>
    requires std::is_default_constructible_v<Type>
constexpr void splitByCommaInternal(std::string_view str, Tuple& tuple) {
    uint32_t commaIndex = str.find_first_of(',');

    std::get<Index>(tuple) = extract<Type>(str.substr(0, commaIndex)).value_or(Type{});

    if constexpr (sizeof...(OtherTypes) > 0) {
        splitByCommaInternal<Index + 1, Tuple, OtherTypes...>(str.substr(commaIndex + 1), tuple);
    }
}

template <typename... Types>
constexpr std::tuple<Types...> splitByCommaTuple(std::string_view str) {
    std::tuple<Types...> results;
    splitByCommaInternal<0, std::tuple<Types...>, Types...>(str, results);
    return results;
}

template <typename StructType, typename... StructTypes>
    requires std::is_class_v<StructType>
constexpr StructType splitByCommaStruct(std::string_view str) {
    using TupleType = std::tuple<StructTypes...>;

    TupleType tuple;
    splitByCommaInternal<0, TupleType, StructTypes...>(str, tuple);
    return StructType{ tuple };
}

template <typename Type>
    requires std::is_default_constructible_v<Type> || std::is_constructible_v<Type, std::string>
constexpr std::vector<Type> splitByCommaVector(std::string_view str) {
    auto split = str | std::views::split(',') | std::views::transform([](auto&& range) { return std::string{ range.begin(), range.end() }; });

    std::vector<Type> results;

    for (auto it = split.begin(); it != split.end(); ++it) {
        results.emplace_back(extract<Type>(trim(*it)).value_or(Type{}));
    }

    return results;
}

#define EASY_EXTRACT(k, type, code)                                   \
    if (key == k) {                                                   \
        if (auto val = extract<type>(value); val.has_value()) {       \
            code;                                                     \
            return {};                                                \
        } else {                                                      \
            return std::format("Invalid value for '{}': ", k, value); \
        }                                                             \
    }

std::expected<AttachmentsConfig, Error> ConfigLoader::loadAttachmentsConfig(const std::filesystem::path& path) {
    AttachmentsConfig config{};
    int currentAttachmentIndex = -1;

    Error error = parseConfig(
        path,
        [&config = config, &i = currentAttachmentIndex](const std::string& key, const std::string& value) mutable -> std::optional<std::string> {
            EASY_EXTRACT("attachment", uint32_t, {
                i = val.value();
                config.attachments.resize(i + 1);
            });

            if (i == -1) {
                return std::format("No attachment index specified before parsing '{}'", key);
            }

            EASY_EXTRACT("format", ColorFormat, { config.attachments[i].colorFormat = val.value(); });
            EASY_EXTRACT("load_op", AttachmentLoadOp, { config.attachments[i].loadOp = val.value(); });
            EASY_EXTRACT("store_op", AttachmentStoreOp, { config.attachments[i].storeOp = val.value(); });
            EASY_EXTRACT("presentable", bool, { config.attachments[i].presentable = val.value(); });

            return std::nullopt;
        });

    TK_ASSERT(error == Error::NoError, "Error loading config");

    if (error != Error::NoError) {
        return std::unexpected(error);
    }

    uint32_t presentableAttachmentCount = 0;
    bool moreThanOnePresentableAttachmentFound = false;

    for (auto& a : config.attachments) {
        if (a.presentable) {
            ++presentableAttachmentCount;
        }

        if (presentableAttachmentCount > 1) {
            if (!moreThanOnePresentableAttachmentFound) {
                LOG_WARN(
                    "More than one presentable attachment specified in shader config file {}, all but the first provided will be set to not "
                    "presentable",
                    path.string());
            }
            moreThanOnePresentableAttachmentFound = true;
            a.presentable = false;
        }
    }

    return config;
}

std::expected<ShaderConfig, Error> ConfigLoader::loadShaderConfig(const std::filesystem::path& path) {
    ShaderConfig config{};

    Error error = parseConfig(path, [&config = config](const std::string& key, const std::string& value) mutable -> std::optional<std::string> {
        // EASY_EXTRACT("attachments_config_path", std::string, { config.attachmentsConfigPath = value; }) else
        // EASY_EXTRACT("name", std::string, { config.name = value; }) else
        EASY_EXTRACT("depth_test_enable", bool, { config.options.depthTest.enable = val.value(); }) else
        EASY_EXTRACT("depth_test_write", bool, { config.options.depthTest.write = val.value(); }) else
        EASY_EXTRACT("depth_compare_op", CompareOp, { config.options.depthTest.compareOp = val.value(); }) else
        EASY_EXTRACT("primitive_topology", PrimitiveTopology, { config.options.primitiveTopology = val.value(); }) else
        EASY_EXTRACT("cull_mode", CullMode, { config.options.cullMode = val.value(); }) else
        EASY_EXTRACT("polygon_mode", PolygonMode, { config.options.polygonMode = val.value(); }) else
        EASY_EXTRACT("front_face", FrontFace, { config.options.frontFace = val.value(); }) else
        EASY_EXTRACT("primitive_restart", bool, { config.options.primitiveRestart = val.value(); }) else

        if (key == "stage") {
            config.stages.emplace_back(splitByCommaStruct<ShaderStageType, ShaderStage, std::filesystem::path, std::filesystem::path>(value));
            return std::nullopt;
        }

        if (key == "binding") {
            config.bindings.emplace_back(splitByCommaStruct<Binding, uint8_t, VertexInputRate, uint8_t>(value));
            return std::nullopt;
        }

        else if (key == "attribute") {
            config.attributes.emplace_back(splitByCommaStruct<Attribute, uint8_t, AttributeType, uint8_t>(value));

            // config.attributes.back().binding

            auto foundBinding =
                std::find_if(config.bindings.begin(), config.bindings.end(), [binding = config.attributes.back().binding](const Binding& b) -> bool {
                    return b.binding == binding;
                });

            if (foundBinding == config.bindings.end()) {
                return std::format("Binding {} not found in config, skipping", config.attributes.back().binding);
            }

            foundBinding->stride += config.attributes.back().getTypeSize();

            return std::nullopt;
        }

        return std::nullopt;
    });

    TK_ASSERT(error == Error::NoError, "Error loading config");

    if (error != Error::NoError) {
        return std::unexpected(error);
    }

    return config;
}

#undef EASY_EXTRACT

template <typename T>
static std::optional<T> parseNumber(std::string_view val)
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
{
    try {
        if constexpr (std::is_integral_v<T>) {
            return std::atoll(std::string(val).c_str());
        } else if constexpr (std::is_floating_point_v<T>) {
            return std::atof(std::string(val).c_str());
        }
    } catch (...) {}

    return std::nullopt;
}

static std::optional<ColorFormat> parseAttachmentFormat(std::string_view val) {
    if (val == "rgba") {
        return ColorFormat::RGBA8;
    } else if (val == "rg" || val == "double_channel") {
        return ColorFormat::RG8;
    } else if (val == "r" || val == "single_channel") {
        return ColorFormat::R8;
    } else if (val == "depth") {
        return ColorFormat::Depth;
    } else if (val == "stencil") {
        return ColorFormat::Stencil;
    } else if (val == "depth_stencil") {
        return ColorFormat::DepthStencil;
    }

    return std::nullopt;
}

static std::optional<AttachmentLoadOp> parseAttachmentLoadOp(std::string_view val) {
    if (val == "load") {
        return AttachmentLoadOp::Load;
    } else if (val == "clear") {
        return AttachmentLoadOp::Clear;
    } else if (val == "dont_care") {
        return AttachmentLoadOp::DontCare;
    }

    return std::nullopt;
}

static std::optional<AttachmentStoreOp> parseAttachmentStoreOp(std::string_view val) {
    if (val == "store") {
        return AttachmentStoreOp::Store;
    } else if (val == "dont_care") {
        return AttachmentStoreOp::DontCare;
    }

    return std::nullopt;
}

static std::optional<ShaderStage> parseShaderStage(std::string_view val) {
    if (val == "fragment") {
        return ShaderStage::Fragment;
    } else if (val == "vertex") {
        return ShaderStage::Vertex;
    }

    return std::nullopt;
}

static std::optional<bool> parseBool(std::string_view val) {
    if (val == "true" || val == "1") {
        return true;
    } else if (val == "false" || val == "0") {
        return false;
    }

    return std::nullopt;
}

static std::optional<CompareOp> parseCompareOp(std::string_view val) {
    if (val == "never") {
        return CompareOp::Never;
    } else if (val == "less") {
        return CompareOp::Less;
    } else if (val == "equal") {
        return CompareOp::Equal;
    } else if (val == "less_or_rqual") {
        return CompareOp::LessOrEqual;
    } else if (val == "greater") {
        return CompareOp::Greater;
    } else if (val == "not_equal") {
        return CompareOp::NotEqual;
    } else if (val == "greater_or_equal") {
        return CompareOp::GreaterOrEqual;
    } else if (val == "always") {
        return CompareOp::Always;
    }

    return std::nullopt;
}

static std::optional<VertexInputRate> parseBindingType(std::string_view val) {
    if (val == "vertex") {
        return VertexInputRate::Vertex;
    } else if (val == "instance") {
        return VertexInputRate::Instance;
    }

    return std::nullopt;
}

static std::optional<AttributeType> parseAttributeType(std::string_view val) {
    if (val == "vec1") {
        return AttributeType::vec1;
    } else if (val == "vec2") {
        return AttributeType::vec2;
    } else if (val == "vec3") {
        return AttributeType::vec3;
    } else if (val == "vec4") {
        return AttributeType::vec4;
    }

    return std::nullopt;
}

static std::optional<PrimitiveTopology> parsePrimitiveTopology(std::string_view val) {
    if (val == "point_list") {
        return PrimitiveTopology::PointList;
    } else if (val == "line_list") {
        return PrimitiveTopology::LineList;
    } else if (val == "line_strip") {
        return PrimitiveTopology::LineStrip;
    } else if (val == "triangle_list") {
        return PrimitiveTopology::TriangleList;
    } else if (val == "triangle_strip") {
        return PrimitiveTopology::TriangleStrip;
    } else if (val == "triangle_fan") {
        return PrimitiveTopology::TriangleFan;
    } else if (val == "line_list_with_adjacency") {
        return PrimitiveTopology::LineListWithAdjacency;
    } else if (val == "line_strip_with_adjacency") {
        return PrimitiveTopology::LineStripWithAdjacency;
    } else if (val == "triangle_list_with_adjacency") {
        return PrimitiveTopology::TriangleListWithAdjacency;
    } else if (val == "triangle_strip_with_adjacency") {
        return PrimitiveTopology::TriangleStripWithAdjacency;
    }

    return std::nullopt;
}

static std::optional<CullMode> parseCullMode(std::string_view val) {
    if (val == "none") {
        return CullMode::None;
    } else if (val == "front") {
        return CullMode::Back;
    } else if (val == "back") {
        return CullMode::Back;
    } else if (val == "front_and_back" || val == "both") {
        return CullMode::FrontAndBack;
    }

    return std::nullopt;
}

static std::optional<PolygonMode> parsePolygonMode(std::string_view val) {
    if (val == "fill") {
        return PolygonMode::Fill;
    } else if (val == "line") {
        return PolygonMode::Line;
    } else if (val == "point") {
        return PolygonMode::Point;
    }

    return std::nullopt;
}

static std::optional<FrontFace> parseFrontFace(std::string_view val) {
    if (val == "clockwise") {
        return FrontFace::Clockwise;
    } else if (val == "counter_clockwise") {
        return FrontFace::CounterClockwise;
    }

    return std::nullopt;
}

template <typename T>
static constexpr std::optional<T> extract(std::string_view val) {
    if constexpr (std::is_same_v<T, std::string>) {
        return std::optional<T>(val);
    }

    else if constexpr (std::is_same_v<T, bool>) {
        return parseBool(val);
    }

    else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
        return parseNumber<T>(val);
    }

    else if constexpr (std::is_same_v<T, ColorFormat>) {
        return parseAttachmentFormat(val);
    }

    else if constexpr (std::is_same_v<T, AttachmentLoadOp>) {
        return parseAttachmentLoadOp(val);
    }

    else if constexpr (std::is_same_v<T, AttachmentStoreOp>) {
        return parseAttachmentStoreOp(val);
    }

    else if constexpr (std::is_same_v<T, ShaderStage>) {
        return parseShaderStage(val);
    }

    else if constexpr (std::is_same_v<T, CompareOp>) {
        return parseCompareOp(val);
    }

    else if constexpr (std::is_same_v<T, VertexInputRate>) {
        return parseBindingType(val);
    }

    else if constexpr (std::is_same_v<T, AttributeType>) {
        return parseAttributeType(val);
    }

    else if constexpr (std::is_same_v<T, PrimitiveTopology>) {
        return parsePrimitiveTopology(val);
    }

    else if constexpr (std::is_same_v<T, CullMode>) {
        return parseCullMode(val);
    }

    else if constexpr (std::is_same_v<T, PolygonMode>) {
        return parsePolygonMode(val);
    }

    else if constexpr (std::is_same_v<T, FrontFace>) {
        return parseFrontFace(val);
    }

    else {
        return T{ val };
    }
}

}  // namespace Toki
