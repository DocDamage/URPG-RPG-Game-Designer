#include "engine/core/message/message_core.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace urpg::message {

namespace {

constexpr int32_t kIconWidth = 32;
constexpr int32_t kIconSpacing = 4;
constexpr int32_t kFontStep = 12;
constexpr int32_t kFontSizeMin = 12;
constexpr int32_t kFontSizeMax = 96;

std::string toLowerCopy(const std::string& value) {
    std::string lowered = value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lowered;
}

bool parseBracketedInt(const std::string& text, size_t& cursor, int32_t& value) {
    if (cursor >= text.size() || text[cursor] != '[') {
        return false;
    }

    size_t i = cursor + 1;
    bool negative = false;
    if (i < text.size() && text[i] == '-') {
        negative = true;
        ++i;
    }
    if (i >= text.size() || !std::isdigit(static_cast<unsigned char>(text[i]))) {
        return false;
    }

    int64_t parsed = 0;
    while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) {
        parsed = parsed * 10 + static_cast<int64_t>(text[i] - '0');
        ++i;
    }

    if (i >= text.size() || text[i] != ']') {
        return false;
    }
    ++i;

    if (negative) {
        parsed = -parsed;
    }
    value = static_cast<int32_t>(parsed);
    cursor = i;
    return true;
}

void appendTextToken(std::vector<RichTextToken>& tokens, const std::string& text) {
    if (text.empty()) {
        return;
    }
    if (!tokens.empty() && tokens.back().type == RichTextTokenType::Text) {
        tokens.back().text += text;
        return;
    }
    RichTextToken token;
    token.type = RichTextTokenType::Text;
    token.text = text;
    tokens.push_back(std::move(token));
}

char32_t decodeUtf8Codepoint(const std::string& text, size_t& cursor) {
    if (cursor >= text.size()) {
        return U'\0';
    }

    const auto lead = static_cast<unsigned char>(text[cursor]);
    if (lead < 0x80) {
        ++cursor;
        return static_cast<char32_t>(lead);
    }

    if ((lead >> 5) == 0x6 && cursor + 1 < text.size()) {
        const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
        if ((b1 & 0xC0) == 0x80) {
            cursor += 2;
            return static_cast<char32_t>(((lead & 0x1F) << 6) | (b1 & 0x3F));
        }
    } else if ((lead >> 4) == 0xE && cursor + 2 < text.size()) {
        const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
        const auto b2 = static_cast<unsigned char>(text[cursor + 2]);
        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80) {
            cursor += 3;
            return static_cast<char32_t>(((lead & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F));
        }
    } else if ((lead >> 3) == 0x1E && cursor + 3 < text.size()) {
        const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
        const auto b2 = static_cast<unsigned char>(text[cursor + 2]);
        const auto b3 = static_cast<unsigned char>(text[cursor + 3]);
        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80 && (b3 & 0xC0) == 0x80) {
            cursor += 4;
            return static_cast<char32_t>(
                ((lead & 0x07) << 18) | ((b1 & 0x3F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F));
        }
    }

    ++cursor;
    return static_cast<char32_t>(lead);
}

int32_t glyphAdvance(char32_t cp, int32_t font_size) {
    const double size = static_cast<double>(std::max(1, font_size));
    if (cp == U'\t') {
        return static_cast<int32_t>(std::lround(size * 2.2));
    }
    if (cp == U' ') {
        return static_cast<int32_t>(std::lround(size * 0.35));
    }

    if (cp < 0x80) {
        const auto ch = static_cast<unsigned char>(cp);
        if (std::isdigit(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.56));
        }
        if (std::ispunct(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.45));
        }
        if (std::isupper(ch)) {
            return static_cast<int32_t>(std::lround(size * 0.62));
        }
        return static_cast<int32_t>(std::lround(size * 0.56));
    }

    if ((cp >= 0x2E80 && cp <= 0x9FFF) || (cp >= 0xF900 && cp <= 0xFAFF)) {
        return static_cast<int32_t>(std::lround(size));
    }
    if (cp >= 0x1F000) {
        return static_cast<int32_t>(std::lround(size * 1.1));
    }
    return static_cast<int32_t>(std::lround(size * 0.75));
}

int32_t measurePlainText(const std::string& text, int32_t font_size) {
    if (text.empty()) {
        return 0;
    }
    int32_t width = 0;
    size_t cursor = 0;
    while (cursor < text.size()) {
        const char32_t cp = decodeUtf8Codepoint(text, cursor);
        if (cp == U'\0' || cp == U'\r' || cp == U'\n') {
            continue;
        }
        width += glyphAdvance(cp, font_size);
    }
    return width;
}

} // namespace

MessagePresentationVariant variantFromCompatRoute(const std::string& route,
                                                  const std::string& speaker_default,
                                                  int32_t speaker_face_actor_id) {
    const std::string normalized_route = toLowerCopy(route);
    MessagePresentationVariant variant;
    variant.speaker = speaker_default;
    variant.face_actor_id = speaker_face_actor_id;

    if (normalized_route == "narration") {
        variant.mode = MessagePresentationMode::Narration;
        variant.tone = MessageTone::Neutral;
        variant.speaker.clear();
        variant.face_actor_id = 0;
        variant.route_token = "narration:narration:neutral";
        return variant;
    }

    if (normalized_route == "system") {
        variant.mode = MessagePresentationMode::System;
        variant.tone = MessageTone::System;
        variant.speaker = speaker_default.empty() ? "System" : speaker_default;
        variant.face_actor_id = 0;
        variant.route_token = "system:system:system";
        return variant;
    }

    variant.mode = MessagePresentationMode::Speaker;
    variant.tone = MessageTone::Portrait;
    variant.route_token = "speaker:speaker:portrait";
    return variant;
}

void PortraitBindingRegistry::registerBinding(int32_t actor_id, PortraitBinding binding) {
    if (actor_id <= 0) {
        return;
    }
    actor_bindings_[actor_id] = std::move(binding);
}

const PortraitBinding* PortraitBindingRegistry::resolveBinding(int32_t actor_id) const {
    const auto it = actor_bindings_.find(actor_id);
    return it == actor_bindings_.end() ? nullptr : &it->second;
}

void PortraitBindingRegistry::clear() {
    actor_bindings_.clear();
}

void ChoicePromptState::open(std::vector<ChoiceOption> options, int32_t default_index) {
    options_ = std::move(options);
    open_ = !options_.empty();
    if (!open_) {
        selected_index_ = 0;
        return;
    }

    const int32_t max_index = static_cast<int32_t>(options_.size()) - 1;
    const auto clamped_default = static_cast<size_t>(std::clamp(default_index, 0, max_index));
    selected_index_ = clamped_default;
    if (!options_[selected_index_].enabled) {
        const auto forward = findEnabledFrom(selected_index_, 1);
        if (forward.has_value()) {
            selected_index_ = *forward;
        }
    }
}

void ChoicePromptState::close() {
    open_ = false;
    selected_index_ = 0;
    options_.clear();
}

const ChoiceOption* ChoicePromptState::selectedOption() const {
    if (!open_ || options_.empty() || selected_index_ >= options_.size()) {
        return nullptr;
    }
    return &options_[selected_index_];
}

bool ChoicePromptState::setSelectedIndex(size_t index) {
    if (!open_ || index >= options_.size() || !options_[index].enabled) {
        return false;
    }
    selected_index_ = index;
    return true;
}

bool ChoicePromptState::moveNext() {
    return advanceSelection(1);
}

bool ChoicePromptState::movePrev() {
    return advanceSelection(-1);
}

bool ChoicePromptState::canConfirm() const {
    const ChoiceOption* option = selectedOption();
    return option != nullptr && option->enabled;
}

std::optional<std::string> ChoicePromptState::confirmSelection() const {
    const ChoiceOption* option = selectedOption();
    if (option == nullptr || !option->enabled) {
        return std::nullopt;
    }
    if (!option->id.empty()) {
        return option->id;
    }
    return option->label;
}

bool ChoicePromptState::advanceSelection(int32_t step) {
    if (!open_ || options_.empty()) {
        return false;
    }
    const auto next = findEnabledFrom(selected_index_, step);
    if (!next.has_value()) {
        return false;
    }
    const bool changed = (*next != selected_index_);
    selected_index_ = *next;
    return changed;
}

std::optional<size_t> ChoicePromptState::findEnabledFrom(size_t start, int32_t step) const {
    if (!open_ || options_.empty() || step == 0) {
        return std::nullopt;
    }

    const size_t size = options_.size();
    size_t cursor = start;
    for (size_t i = 0; i < size; ++i) {
        if (step > 0) {
            cursor = (cursor + 1) % size;
        } else {
            cursor = (cursor + size - 1) % size;
        }
        if (options_[cursor].enabled) {
            return cursor;
        }
    }

    if (options_[start].enabled) {
        return start;
    }
    return std::nullopt;
}

void RichTextLayoutEngine::setVariableResolver(VariableResolver resolver) {
    variable_resolver_ = std::move(resolver);
}

void RichTextLayoutEngine::setActorNameResolver(ActorNameResolver resolver) {
    actor_name_resolver_ = std::move(resolver);
}

void RichTextLayoutEngine::setPartyMemberResolver(PartyMemberResolver resolver) {
    party_member_resolver_ = std::move(resolver);
}

void RichTextLayoutEngine::setCurrencyUnit(std::string unit) {
    currency_unit_ = unit.empty() ? "G" : std::move(unit);
}

void RichTextLayoutEngine::setBaseFontSize(int32_t size) {
    base_font_size_ = std::max(1, size);
}

void RichTextLayoutEngine::setLineHeight(int32_t height) {
    line_height_ = std::max(1, height);
}

RichTextLayoutResult RichTextLayoutEngine::layout(const std::string& text) const {
    RichTextLayoutResult result;
    result.tokens = tokenize(text);
    result.metrics.line_height = line_height_;

    if (result.tokens.empty()) {
        result.metrics.width = 0;
        result.metrics.height = line_height_;
        result.metrics.line_count = 1;
        return result;
    }

    int32_t line_width = 0;
    int32_t line_height = line_height_;
    int32_t font_size = base_font_size_;
    int32_t total_height = 0;
    int32_t line_count = 1;

    for (const auto& token : result.tokens) {
        switch (token.type) {
            case RichTextTokenType::Text:
                line_width += measurePlainText(token.text, font_size);
                line_height = std::max(line_height, font_size + 8);
                result.metrics.width = std::max(result.metrics.width, line_width);
                break;
            case RichTextTokenType::Icon:
                line_width += kIconWidth + kIconSpacing;
                line_height = std::max(line_height, kIconWidth);
                result.metrics.width = std::max(result.metrics.width, line_width);
                break;
            case RichTextTokenType::Color:
                break;
            case RichTextTokenType::FontBigger:
                font_size = std::min(kFontSizeMax, font_size + kFontStep);
                line_height = std::max(line_height, font_size + 8);
                break;
            case RichTextTokenType::FontSmaller:
                font_size = std::max(kFontSizeMin, font_size - kFontStep);
                line_height = std::max(line_height, font_size + 8);
                break;
            case RichTextTokenType::NewLine:
                total_height += line_height;
                line_width = 0;
                line_height = line_height_;
                ++line_count;
                break;
        }
    }

    result.metrics.width = std::max(result.metrics.width, line_width);
    result.metrics.height = total_height + line_height;
    result.metrics.line_count = line_count;
    return result;
}

int32_t RichTextLayoutEngine::textWidth(const std::string& text) const {
    return layout(text).metrics.width;
}

std::vector<RichTextToken> RichTextLayoutEngine::tokenize(const std::string& text) const {
    std::vector<RichTextToken> tokens;
    std::string plain_buffer;
    plain_buffer.reserve(text.size());

    const auto flush_plain = [&]() {
        if (!plain_buffer.empty()) {
            appendTextToken(tokens, plain_buffer);
            plain_buffer.clear();
        }
    };

    size_t i = 0;
    while (i < text.size()) {
        const char ch = text[i];
        if (ch == '\r') {
            ++i;
            continue;
        }
        if (ch == '\n') {
            flush_plain();
            RichTextToken token;
            token.type = RichTextTokenType::NewLine;
            tokens.push_back(token);
            ++i;
            continue;
        }
        if (ch != '\\') {
            plain_buffer.push_back(ch);
            ++i;
            continue;
        }

        if (i + 1 >= text.size()) {
            plain_buffer.push_back('\\');
            ++i;
            continue;
        }

        const char raw_command = text[i + 1];
        if (raw_command == '\\') {
            plain_buffer.push_back('\\');
            i += 2;
            continue;
        }

        const char command = static_cast<char>(std::toupper(static_cast<unsigned char>(raw_command)));
        size_t cursor = i + 2;
        int32_t arg = 0;

        const auto push_literal_command = [&]() {
            plain_buffer.push_back('\\');
            plain_buffer.push_back(raw_command);
            i += 2;
        };

        if (command == '{') {
            flush_plain();
            RichTextToken token;
            token.type = RichTextTokenType::FontBigger;
            tokens.push_back(token);
            i = cursor;
            continue;
        }
        if (command == '}') {
            flush_plain();
            RichTextToken token;
            token.type = RichTextTokenType::FontSmaller;
            tokens.push_back(token);
            i = cursor;
            continue;
        }
        if (command == 'G') {
            flush_plain();
            appendTextToken(tokens, currency_unit_);
            i = cursor;
            continue;
        }

        if (command == 'C' || command == 'I' || command == 'V' || command == 'N' || command == 'P') {
            if (!parseBracketedInt(text, cursor, arg)) {
                push_literal_command();
                continue;
            }

            flush_plain();
            if (command == 'C') {
                RichTextToken token;
                token.type = RichTextTokenType::Color;
                token.value = arg;
                tokens.push_back(token);
            } else if (command == 'I') {
                RichTextToken token;
                token.type = RichTextTokenType::Icon;
                token.value = arg;
                tokens.push_back(token);
            } else {
                appendTextToken(tokens, resolveEscape(command, arg));
            }
            i = cursor;
            continue;
        }

        push_literal_command();
    }

    flush_plain();
    return tokens;
}

std::string RichTextLayoutEngine::resolveEscape(char command, int32_t arg) const {
    switch (command) {
        case 'V':
            return variable_resolver_ ? std::to_string(variable_resolver_(arg)) : std::to_string(arg);
        case 'N': {
            if (actor_name_resolver_) {
                return actor_name_resolver_(arg);
            }
            return "Actor " + std::to_string(std::max(0, arg));
        }
        case 'P': {
            if (party_member_resolver_ && actor_name_resolver_) {
                const int32_t actor_id = party_member_resolver_(arg - 1);
                if (actor_id > 0) {
                    return actor_name_resolver_(actor_id);
                }
            }
            return "";
        }
        case 'G':
            return currency_unit_;
        default:
            break;
    }
    return "";
}

void MessageFlowRunner::begin(std::vector<DialoguePage> pages) {
    pages_ = std::move(pages);
    page_index_ = 0;
    choice_prompt_.close();
    if (pages_.empty()) {
        state_ = MessageFlowState::Completed;
        return;
    }
    enterCurrentPage();
}

void MessageFlowRunner::cancel() {
    pages_.clear();
    page_index_ = 0;
    choice_prompt_.close();
    state_ = MessageFlowState::Idle;
}

bool MessageFlowRunner::isActive() const {
    return state_ == MessageFlowState::Presenting ||
           state_ == MessageFlowState::AwaitingAdvance ||
           state_ == MessageFlowState::AwaitingChoice;
}

const DialoguePage* MessageFlowRunner::currentPage() const {
    if (page_index_ >= pages_.size()) {
        return nullptr;
    }
    return &pages_[page_index_];
}

bool MessageFlowRunner::markPagePresented() {
    if (state_ != MessageFlowState::Presenting) {
        return false;
    }

    const DialoguePage* page = currentPage();
    if (page == nullptr) {
        return false;
    }

    if (!page->choices.empty()) {
        choice_prompt_.open(page->choices, page->default_choice_index);
        state_ = MessageFlowState::AwaitingChoice;
        return true;
    }

    if (page->wait_for_advance) {
        state_ = MessageFlowState::AwaitingAdvance;
        return true;
    }

    stepToNextPage();
    return true;
}

bool MessageFlowRunner::advance() {
    if (state_ == MessageFlowState::Presenting) {
        return markPagePresented();
    }
    if (state_ != MessageFlowState::AwaitingAdvance) {
        return false;
    }
    stepToNextPage();
    return true;
}

bool MessageFlowRunner::moveChoiceNext() {
    if (state_ != MessageFlowState::AwaitingChoice) {
        return false;
    }
    return choice_prompt_.moveNext();
}

bool MessageFlowRunner::moveChoicePrev() {
    if (state_ != MessageFlowState::AwaitingChoice) {
        return false;
    }
    return choice_prompt_.movePrev();
}

std::optional<std::string> MessageFlowRunner::confirmChoice() {
    if (state_ != MessageFlowState::AwaitingChoice) {
        return std::nullopt;
    }

    const auto selected = choice_prompt_.confirmSelection();
    if (!selected.has_value()) {
        return std::nullopt;
    }

    stepToNextPage();
    return selected;
}

MessageFlowSnapshot MessageFlowRunner::snapshot() const {
    MessageFlowSnapshot snapshot;
    snapshot.page_index = page_index_;
    snapshot.state = state_;
    snapshot.selected_choice_index = choice_prompt_.selectedIndex();
    return snapshot;
}

bool MessageFlowRunner::restore(const MessageFlowSnapshot& snapshot) {
    if (snapshot.state == MessageFlowState::Completed) {
        if (!pages_.empty() && snapshot.page_index > pages_.size()) {
            return false;
        }
        page_index_ = std::min(snapshot.page_index, pages_.size());
        state_ = MessageFlowState::Completed;
        choice_prompt_.close();
        return true;
    }

    if (snapshot.page_index >= pages_.size()) {
        return false;
    }

    page_index_ = snapshot.page_index;
    state_ = snapshot.state;
    choice_prompt_.close();

    if (state_ == MessageFlowState::AwaitingChoice) {
        const DialoguePage* page = currentPage();
        if (page == nullptr || page->choices.empty()) {
            return false;
        }
        choice_prompt_.open(page->choices, page->default_choice_index);
        if (!choice_prompt_.isOpen()) {
            return false;
        }
        const size_t requested_index =
            std::min(snapshot.selected_choice_index, choice_prompt_.optionCount() - 1);
        if (!choice_prompt_.setSelectedIndex(requested_index)) {
            const ChoiceOption* option = choice_prompt_.selectedOption();
            if (option == nullptr || !option->enabled) {
                return false;
            }
        }
    }

    if (state_ == MessageFlowState::Idle) {
        pages_.clear();
        page_index_ = 0;
        choice_prompt_.close();
    }

    return true;
}

void MessageFlowRunner::enterCurrentPage() {
    choice_prompt_.close();
    if (page_index_ >= pages_.size()) {
        state_ = MessageFlowState::Completed;
        return;
    }
    state_ = MessageFlowState::Presenting;
}

void MessageFlowRunner::stepToNextPage() {
    if (page_index_ < pages_.size()) {
        ++page_index_;
    }
    enterCurrentPage();
}

} // namespace urpg::message
