#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::message {

enum class MessagePresentationMode : uint8_t {
    Speaker = 0,
    Narration = 1,
    System = 2,
};

enum class MessageTone : uint8_t {
    Portrait = 0,
    Neutral = 1,
    System = 2,
};

struct MessagePresentationVariant {
    MessagePresentationMode mode = MessagePresentationMode::Speaker;
    MessageTone tone = MessageTone::Portrait;
    std::string speaker;
    int32_t face_actor_id = 0;
    std::string route_token;
};

MessagePresentationVariant variantFromCompatRoute(const std::string& route,
                                                  const std::string& speaker_default,
                                                  int32_t speaker_face_actor_id = 0);

struct NameboxChrome {
    bool visible = true;
    int32_t margin_x = 12;
    int32_t margin_y = 8;
    int32_t max_width = 320;
};

struct PortraitChrome {
    bool visible = true;
    bool dock_left = true;
    int32_t x = 16;
    int32_t y = 12;
    int32_t width = 144;
    int32_t height = 144;
};

struct DialoguePresentationModel {
    std::string style_id = "default";
    MessagePresentationVariant variant;
    NameboxChrome namebox;
    PortraitChrome portrait;
};

struct PortraitBinding {
    std::string face_name;
    int32_t face_index = 0;
    bool mirror = false;
};

class PortraitBindingRegistry {
public:
    void registerBinding(int32_t actor_id, PortraitBinding binding);
    const PortraitBinding* resolveBinding(int32_t actor_id) const;
    void clear();

private:
    std::map<int32_t, PortraitBinding> actor_bindings_;
};

struct ChoiceOption {
    std::string id;
    std::string label;
    bool enabled = true;
    std::string disabled_reason;
};

class ChoicePromptState {
public:
    void open(std::vector<ChoiceOption> options, int32_t default_index = 0);
    void close();

    bool isOpen() const { return open_; }
    size_t optionCount() const { return options_.size(); }
    size_t selectedIndex() const { return selected_index_; }
    const ChoiceOption* selectedOption() const;
    bool setSelectedIndex(size_t index);
    bool moveNext();
    bool movePrev();
    bool canConfirm() const;
    std::optional<std::string> confirmSelection() const;

private:
    bool advanceSelection(int32_t step);
    std::optional<size_t> findEnabledFrom(size_t start, int32_t step) const;

    bool open_ = false;
    size_t selected_index_ = 0;
    std::vector<ChoiceOption> options_;
};

enum class RichTextTokenType : uint8_t {
    Text = 0,
    Icon = 1,
    Color = 2,
    FontBigger = 3,
    FontSmaller = 4,
    NewLine = 5,
    LineOffset = 6, // Horizontal pixel offset for a line (used for alignment)
};

struct RichTextToken {
    RichTextTokenType type = RichTextTokenType::Text;
    std::string text;
    int32_t value = 0;
};

enum class MessageAlignment : uint8_t {
    Left = 0,
    Center = 1,
    Right = 2,
};

struct RichTextLayoutMetrics {
    int32_t width = 0;
    int32_t height = 0;
    int32_t line_count = 1;
    int32_t line_height = 36;
};

struct RichTextLayoutResult {
    std::vector<RichTextToken> tokens;
    RichTextLayoutMetrics metrics;
};

class RichTextLayoutEngine {
public:
    using VariableResolver = std::function<int32_t(int32_t)>;
    using ActorNameResolver = std::function<std::string(int32_t)>;
    using PartyMemberResolver = std::function<int32_t(int32_t)>;

    void setVariableResolver(VariableResolver resolver);
    void setActorNameResolver(ActorNameResolver resolver);
    void setPartyMemberResolver(PartyMemberResolver resolver);

    void setCurrencyUnit(std::string unit);
    void setBaseFontSize(int32_t size);
    void setLineHeight(int32_t height);
    void setMaxWidth(int32_t max_width);
    void setAlignment(MessageAlignment alignment);

    [[nodiscard]] RichTextLayoutResult layout(const std::string& text) const;
    [[nodiscard]] int32_t textWidth(const std::string& text) const;

    /**
     * @brief Resolves all escapes in the text (variables, actor names, etc.) 
     * but does NOT tokenize or layout. Used for simple string expansion.
     */
    [[nodiscard]] std::string resolveEscapes(const std::string& text) const;

private:
    std::vector<RichTextToken> tokenize(const std::string& text) const;
    std::string resolveEscape(char command, int32_t arg) const;

    VariableResolver variable_resolver_;
    ActorNameResolver actor_name_resolver_;
    PartyMemberResolver party_member_resolver_;
    std::string currency_unit_ = "G";
    int32_t base_font_size_ = 22;
    int32_t line_height_ = 36;
    int32_t max_width_ = 0;
    MessageAlignment alignment_ = MessageAlignment::Left;
};

struct DialoguePage {
    std::string id;
    std::string body;
    MessagePresentationVariant variant;
    bool wait_for_advance = true;
    std::vector<ChoiceOption> choices;
    int32_t default_choice_index = 0;
    std::string command; // Hook for "tool usage"
};

enum class MessageFlowState : uint8_t {
    Idle = 0,
    Presenting = 1,
    AwaitingAdvance = 2,
    AwaitingChoice = 3,
    Completed = 4,
};

struct MessageFlowSnapshot {
    size_t page_index = 0;
    MessageFlowState state = MessageFlowState::Idle;
    size_t selected_choice_index = 0;
};

class MessageFlowRunner {
public:
    using CommandExecutor = std::function<void(const std::string&)>;

    void begin(std::vector<DialoguePage> pages);
    void resetWithPages(std::vector<DialoguePage> pages);
    void cancel();

    void setCommandExecutor(CommandExecutor executor) { command_executor_ = std::move(executor); }

    [[nodiscard]] bool isActive() const;
    [[nodiscard]] MessageFlowState state() const { return state_; }
    [[nodiscard]] size_t currentPageIndex() const { return page_index_; }
    [[nodiscard]] const DialoguePage* currentPage() const;
    [[nodiscard]] const std::vector<DialoguePage>& pages() const { return pages_; }
    [[nodiscard]] const ChoicePromptState& choicePrompt() const { return choice_prompt_; }

    bool markPagePresented();
    bool advance();
    bool moveChoiceNext();
    bool moveChoicePrev();
    std::optional<std::string> confirmChoice();

    [[nodiscard]] MessageFlowSnapshot snapshot() const;
    bool restore(const MessageFlowSnapshot& snapshot);

private:
    void enterCurrentPage();
    void stepToNextPage();

    std::vector<DialoguePage> pages_;
    size_t page_index_ = 0;
    MessageFlowState state_ = MessageFlowState::Idle;
    ChoicePromptState choice_prompt_;
    CommandExecutor command_executor_;
};

} // namespace urpg::message
