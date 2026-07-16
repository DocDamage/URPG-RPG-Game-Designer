#include "apps/editor/windows_accessibility_bridge.h"

#ifdef _WIN32

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <oleacc.h>
#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <atomic>
#include <limits>
#include <string>
#include <utility>

namespace urpg::editor_app {
namespace {

constexpr wchar_t kBridgeProperty[] = L"URPG.WindowsAccessibilityBridge";
constexpr LONG kProjectTop = 46;
constexpr LONG kProjectHeight = 62;
constexpr LONG kPanelTop = 142;
constexpr LONG kPanelHeight = 18;

std::wstring utf8ToWide(const std::string& value) {
    if (value.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                            static_cast<int>(value.size()), nullptr, 0);
    if (length <= 0) return std::wstring(value.begin(), value.end());
    std::wstring result(static_cast<std::size_t>(length), L'\0');
    (void)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                              static_cast<int>(value.size()), result.data(), length);
    return result;
}

std::string treeFingerprint(const editor::NativeAccessibilitySnapshot& tree) {
    std::string result = tree.name + "\n" + tree.description;
    for (const auto& node : tree.nodes) {
        result += "\n" + node.id + "\t" + node.name + "\t" + node.value + "\t" + node.default_action +
                  "\t" + std::to_string(static_cast<int>(node.role)) + (node.enabled ? "1" : "0") +
                  (node.focusable ? "1" : "0") + (node.selected ? "1" : "0") + (node.editable ? "1" : "0");
    }
    return result;
}

HRESULT copyBstr(const std::string& value, BSTR* output) {
    if (output == nullptr) return E_INVALIDARG;
    *output = nullptr;
    if (value.empty()) return S_FALSE;
    const auto wide = utf8ToWide(value);
    *output = SysAllocStringLen(wide.data(), static_cast<UINT>(wide.size()));
    return *output == nullptr ? E_OUTOFMEMORY : S_OK;
}

LONG roleFor(const editor::NativeAccessibilityRole role) {
    switch (role) {
    case editor::NativeAccessibilityRole::Button: return ROLE_SYSTEM_PUSHBUTTON;
    case editor::NativeAccessibilityRole::TextField: return ROLE_SYSTEM_TEXT;
    case editor::NativeAccessibilityRole::ListItem: return ROLE_SYSTEM_LISTITEM;
    case editor::NativeAccessibilityRole::Group: return ROLE_SYSTEM_GROUPING;
    case editor::NativeAccessibilityRole::Diagnostic: return ROLE_SYSTEM_ALERT;
    case editor::NativeAccessibilityRole::Text: return ROLE_SYSTEM_STATICTEXT;
    }
    return ROLE_SYSTEM_CLIENT;
}

} // namespace

class WindowsAccessibilityBridge::Impl {
public:
    class AccessibleRoot final : public IAccessible {
    public:
        explicit AccessibleRoot(Impl* owner) : owner_(owner) {}

        void detach() { owner_ = nullptr; }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
            if (object == nullptr) return E_INVALIDARG;
            *object = nullptr;
            if (iid == IID_IUnknown || iid == IID_IDispatch || iid == IID_IAccessible) {
                *object = static_cast<IAccessible*>(this);
                AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override { return ++reference_count_; }

        ULONG STDMETHODCALLTYPE Release() override {
            const auto remaining = --reference_count_;
            if (remaining == 0) delete this;
            return remaining;
        }

        HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* count) override {
            if (count == nullptr) return E_INVALIDARG;
            *count = 0;
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT, LCID, ITypeInfo**) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) override {
            return E_NOTIMPL;
        }
        HRESULT STDMETHODCALLTYPE Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*) override {
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE get_accParent(IDispatch** parent) override {
            if (parent == nullptr) return E_INVALIDARG;
            *parent = nullptr;
            return S_FALSE;
        }

        HRESULT STDMETHODCALLTYPE get_accChildCount(long* count) override {
            if (count == nullptr) return E_INVALIDARG;
            const auto tree = snapshot();
            *count = static_cast<long>(std::min<std::size_t>(tree.nodes.size(),
                                                             static_cast<std::size_t>(std::numeric_limits<long>::max())));
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE get_accChild(VARIANT child, IDispatch** dispatch) override {
            if (dispatch == nullptr) return E_INVALIDARG;
            *dispatch = nullptr;
            std::size_t index = 0;
            return resolveChild(child, &index, false) ? S_FALSE : E_INVALIDARG;
        }

        HRESULT STDMETHODCALLTYPE get_accName(VARIANT child, BSTR* name) override {
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, true, tree)) return E_INVALIDARG;
            return copyBstr(index == kSelf ? tree.name : tree.nodes[index].name, name);
        }

        HRESULT STDMETHODCALLTYPE get_accValue(VARIANT child, BSTR* value) override {
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, true, tree)) return E_INVALIDARG;
            if (index == kSelf) {
                if (value != nullptr) *value = nullptr;
                return S_FALSE;
            }
            return copyBstr(tree.nodes[index].value, value);
        }

        HRESULT STDMETHODCALLTYPE get_accDescription(VARIANT child, BSTR* description) override {
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, true, tree)) return E_INVALIDARG;
            return copyBstr(index == kSelf ? tree.description : tree.nodes[index].description, description);
        }

        HRESULT STDMETHODCALLTYPE get_accRole(VARIANT child, VARIANT* role) override {
            if (role == nullptr) return E_INVALIDARG;
            VariantInit(role);
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, true, tree)) return E_INVALIDARG;
            role->vt = VT_I4;
            role->lVal = index == kSelf ? ROLE_SYSTEM_APPLICATION : roleFor(tree.nodes[index].role);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE get_accState(VARIANT child, VARIANT* state) override {
            if (state == nullptr) return E_INVALIDARG;
            VariantInit(state);
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, true, tree)) return E_INVALIDARG;
            LONG flags = 0;
            if (index == kSelf) {
                flags = STATE_SYSTEM_FOCUSABLE;
                if (owner_ != nullptr && GetFocus() == owner_->window_) flags |= STATE_SYSTEM_FOCUSED;
            } else {
                const auto& node = tree.nodes[index];
                if (!node.enabled) flags |= STATE_SYSTEM_UNAVAILABLE;
                if (node.focusable) flags |= STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;
                if (node.role == editor::NativeAccessibilityRole::TextField && !node.editable)
                    flags |= STATE_SYSTEM_READONLY;
                if (node.selected) flags |= STATE_SYSTEM_SELECTED;
                if (owner_ != nullptr && owner_->focused_child_ == static_cast<long>(index + 1)) {
                    flags |= STATE_SYSTEM_FOCUSED;
                }
            }
            state->vt = VT_I4;
            state->lVal = flags;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE get_accHelp(VARIANT, BSTR* help) override {
            if (help == nullptr) return E_INVALIDARG;
            *help = nullptr;
            return S_FALSE;
        }
        HRESULT STDMETHODCALLTYPE get_accHelpTopic(BSTR* help_file, VARIANT, long* topic_id) override {
            if (help_file == nullptr || topic_id == nullptr) return E_INVALIDARG;
            *help_file = nullptr;
            *topic_id = -1;
            return S_FALSE;
        }
        HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut(VARIANT, BSTR* shortcut) override {
            if (shortcut == nullptr) return E_INVALIDARG;
            *shortcut = nullptr;
            return S_FALSE;
        }

        HRESULT STDMETHODCALLTYPE get_accFocus(VARIANT* focus) override {
            if (focus == nullptr) return E_INVALIDARG;
            VariantInit(focus);
            if (owner_ == nullptr) return S_FALSE;
            focus->vt = VT_I4;
            focus->lVal = owner_->focused_child_ > 0 ? owner_->focused_child_ : CHILDID_SELF;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE get_accSelection(VARIANT* selection) override {
            if (selection == nullptr) return E_INVALIDARG;
            VariantInit(selection);
            const auto tree = snapshot();
            const auto selected = std::find_if(tree.nodes.begin(), tree.nodes.end(),
                                               [](const auto& node) { return node.selected; });
            if (selected == tree.nodes.end()) return S_FALSE;
            selection->vt = VT_I4;
            selection->lVal = static_cast<LONG>(std::distance(tree.nodes.begin(), selected) + 1);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE get_accDefaultAction(VARIANT child, BSTR* action) override {
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, false, tree)) return E_INVALIDARG;
            return copyBstr(tree.nodes[index].default_action, action);
        }

        HRESULT STDMETHODCALLTYPE accSelect(long flags, VARIANT child) override {
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, false, tree) || !tree.nodes[index].enabled ||
                !tree.nodes[index].focusable || owner_ == nullptr) {
                return E_INVALIDARG;
            }
            if ((flags & (SELFLAG_TAKEFOCUS | SELFLAG_TAKESELECTION)) == 0) return E_INVALIDARG;
            owner_->focused_child_ = static_cast<long>(index + 1);
            SetFocus(owner_->window_);
            NotifyWinEvent(EVENT_OBJECT_FOCUS, owner_->window_, OBJID_CLIENT, owner_->focused_child_);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE accLocation(long* left, long* top, long* width, long* height,
                                              VARIANT child) override {
            if (left == nullptr || top == nullptr || width == nullptr || height == nullptr || owner_ == nullptr) {
                return E_INVALIDARG;
            }
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, true, tree)) return E_INVALIDARG;
            RECT rectangle{};
            if (index == kSelf) {
                if (!GetWindowRect(owner_->window_, &rectangle)) return E_FAIL;
            } else if (!owner_->childRectangle(tree, index, &rectangle)) return E_FAIL;
            *left = rectangle.left;
            *top = rectangle.top;
            *width = std::max<LONG>(0, rectangle.right - rectangle.left);
            *height = std::max<LONG>(0, rectangle.bottom - rectangle.top);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE accNavigate(long direction, VARIANT start, VARIANT* destination) override {
            if (destination == nullptr) return E_INVALIDARG;
            VariantInit(destination);
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(start, &index, true, tree)) return E_INVALIDARG;
            long target = 0;
            if (index == kSelf && direction == NAVDIR_FIRSTCHILD && !tree.nodes.empty()) target = 1;
            else if (index == kSelf && direction == NAVDIR_LASTCHILD && !tree.nodes.empty())
                target = static_cast<long>(tree.nodes.size());
            else if (index != kSelf && direction == NAVDIR_NEXT && index + 1 < tree.nodes.size())
                target = static_cast<long>(index + 2);
            else if (index != kSelf && direction == NAVDIR_PREVIOUS && index > 0)
                target = static_cast<long>(index);
            else if (index != kSelf && direction == NAVDIR_UP)
                target = index > 0 ? static_cast<long>(index) : 0;
            else if (index != kSelf && direction == NAVDIR_DOWN && index + 1 < tree.nodes.size())
                target = static_cast<long>(index + 2);
            if (target == 0) return S_FALSE;
            destination->vt = VT_I4;
            destination->lVal = target;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE accHitTest(long x, long y, VARIANT* child) override {
            if (child == nullptr || owner_ == nullptr) return E_INVALIDARG;
            VariantInit(child);
            RECT window{};
            if (!GetWindowRect(owner_->window_, &window) || x < window.left || x >= window.right ||
                y < window.top || y >= window.bottom) return S_FALSE;
            const auto tree = snapshot();
            child->vt = VT_I4;
            child->lVal = CHILDID_SELF;
            for (std::size_t index = 0; index < tree.nodes.size(); ++index) {
                RECT node{};
                if (owner_->childRectangle(tree, index, &node) && x >= node.left && x < node.right &&
                    y >= node.top && y < node.bottom) {
                    child->lVal = static_cast<LONG>(index + 1);
                    break;
                }
            }
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE accDoDefaultAction(VARIANT child) override {
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, false, tree) || owner_ == nullptr) return E_INVALIDARG;
            const auto& node = tree.nodes[index];
            if (!node.enabled || node.default_action.empty() || !owner_->action_handler_) return S_FALSE;
            if (!owner_->action_handler_(node.id)) return S_FALSE;
            owner_->focused_child_ = static_cast<long>(index + 1);
            NotifyWinEvent(EVENT_OBJECT_SELECTION, owner_->window_, OBJID_CLIENT, owner_->focused_child_);
            NotifyWinEvent(EVENT_OBJECT_FOCUS, owner_->window_, OBJID_CLIENT, owner_->focused_child_);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE put_accName(VARIANT, BSTR) override { return E_NOTIMPL; }
        HRESULT STDMETHODCALLTYPE put_accValue(VARIANT child, BSTR value) override {
            const auto tree = snapshot();
            std::size_t index = 0;
            if (!resolveChild(child, &index, false, tree) || owner_ == nullptr || value == nullptr) {
                return E_INVALIDARG;
            }
            const auto& node = tree.nodes[index];
            if (!node.enabled || !node.editable || !owner_->value_handler_) return E_NOTIMPL;
            const int length = WideCharToMultiByte(CP_UTF8, 0, value, static_cast<int>(SysStringLen(value)),
                                                   nullptr, 0, nullptr, nullptr);
            if (length < 0) return E_INVALIDARG;
            std::string utf8(static_cast<std::size_t>(length), '\0');
            if (length > 0) {
                (void)WideCharToMultiByte(CP_UTF8, 0, value, static_cast<int>(SysStringLen(value)),
                                          utf8.data(), length, nullptr, nullptr);
            }
            if (!owner_->value_handler_(node.id, utf8)) return E_FAIL;
            NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, owner_->window_, OBJID_CLIENT,
                           static_cast<LONG>(index + 1));
            return S_OK;
        }

    private:
        static constexpr std::size_t kSelf = std::numeric_limits<std::size_t>::max();

        editor::NativeAccessibilitySnapshot snapshot() const {
            return owner_ != nullptr && owner_->snapshot_provider_
                       ? owner_->snapshot_provider_()
                       : editor::NativeAccessibilitySnapshot{};
        }

        bool resolveChild(const VARIANT child, std::size_t* index, const bool allow_self) const {
            return resolveChild(child, index, allow_self, snapshot());
        }

        static bool resolveChild(const VARIANT child, std::size_t* index, const bool allow_self,
                                 const editor::NativeAccessibilitySnapshot& tree) {
            if (index == nullptr || child.vt != VT_I4) return false;
            if (child.lVal == CHILDID_SELF) {
                if (!allow_self) return false;
                *index = kSelf;
                return true;
            }
            if (child.lVal < 1 || static_cast<std::size_t>(child.lVal) > tree.nodes.size()) return false;
            *index = static_cast<std::size_t>(child.lVal - 1);
            return true;
        }

        std::atomic<ULONG> reference_count_{1};
        Impl* owner_ = nullptr;
    };

    bool install(void* sdl_window, SnapshotProvider snapshot_provider, ActionHandler action_handler,
                 ValueHandler value_handler) {
        if (window_ != nullptr || sdl_window == nullptr || !snapshot_provider || !action_handler) return false;
        SDL_SysWMinfo info{};
        SDL_VERSION(&info.version);
        if (SDL_GetWindowWMInfo(static_cast<SDL_Window*>(sdl_window), &info) != SDL_TRUE ||
            info.subsystem != SDL_SYSWM_WINDOWS || info.info.win.window == nullptr) {
            return false;
        }
        window_ = info.info.win.window;
        snapshot_provider_ = std::move(snapshot_provider);
        action_handler_ = std::move(action_handler);
        value_handler_ = std::move(value_handler);
        const HRESULT ole_result = OleInitialize(nullptr);
        ole_initialized_ = SUCCEEDED(ole_result);
        provider_ = new AccessibleRoot(this);
        SetLastError(0);
        original_window_proc_ = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Impl::windowProcedure)));
        if (original_window_proc_ == nullptr && GetLastError() != 0) {
            provider_->Release();
            provider_ = nullptr;
            if (ole_initialized_) OleUninitialize();
            ole_initialized_ = false;
            window_ = nullptr;
            snapshot_provider_ = {};
            action_handler_ = {};
            value_handler_ = {};
            return false;
        }
        if (!SetPropW(window_, kBridgeProperty, this)) {
            (void)SetWindowLongPtrW(window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_window_proc_));
            provider_->Release();
            provider_ = nullptr;
            if (ole_initialized_) OleUninitialize();
            ole_initialized_ = false;
            window_ = nullptr;
            original_window_proc_ = nullptr;
            snapshot_provider_ = {};
            action_handler_ = {};
            value_handler_ = {};
            return false;
        }
        const auto tree = snapshot_provider_();
        last_tree_fingerprint_ = treeFingerprint(tree);
        const auto selected = std::find_if(tree.nodes.begin(), tree.nodes.end(),
                                           [](const auto& node) { return node.selected; });
        focused_child_ = selected == tree.nodes.end()
                             ? 0
                             : static_cast<long>(std::distance(tree.nodes.begin(), selected) + 1);
        NotifyWinEvent(EVENT_OBJECT_REORDER, window_, OBJID_CLIENT, CHILDID_SELF);
        return true;
    }

    void uninstall() {
        if (window_ != nullptr) {
            (void)RemovePropW(window_, kBridgeProperty);
            if (original_window_proc_ != nullptr && IsWindow(window_)) {
                (void)SetWindowLongPtrW(window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_window_proc_));
            }
        }
        if (provider_ != nullptr) {
            provider_->detach();
            provider_->Release();
            provider_ = nullptr;
        }
        window_ = nullptr;
        original_window_proc_ = nullptr;
        focused_child_ = 0;
        snapshot_provider_ = {};
        action_handler_ = {};
        value_handler_ = {};
        last_tree_fingerprint_.clear();
        if (ole_initialized_) OleUninitialize();
        ole_initialized_ = false;
    }

    bool childRectangle(const editor::NativeAccessibilitySnapshot& tree, const std::size_t index,
                        RECT* rectangle) const {
        if (rectangle == nullptr || window_ == nullptr || index >= tree.nodes.size()) return false;
        RECT client{};
        if (!GetClientRect(window_, &client)) return false;
        POINT origin{client.left, client.top};
        if (!ClientToScreen(window_, &origin)) return false;
        const bool project = tree.nodes[index].id == "project.current";
        const LONG panel_index = project ? 0 : static_cast<LONG>(index > 0 ? index - 1 : 0);
        rectangle->left = origin.x + 12;
        rectangle->top = origin.y + (project ? kProjectTop : kPanelTop + panel_index * kPanelHeight);
        rectangle->right = origin.x + std::min<LONG>(340, std::max<LONG>(80, client.right - 24));
        rectangle->bottom = rectangle->top + (project ? kProjectHeight : kPanelHeight);
        return true;
    }

    void focusChildAtClientPoint(const LONG x, const LONG y) {
        if (!snapshot_provider_ || window_ == nullptr) return;
        POINT origin{0, 0};
        if (!ClientToScreen(window_, &origin)) return;
        const auto tree = snapshot_provider_();
        for (std::size_t index = 0; index < tree.nodes.size(); ++index) {
            RECT node{};
            if (!tree.nodes[index].focusable || !childRectangle(tree, index, &node)) continue;
            if (origin.x + x >= node.left && origin.x + x < node.right &&
                origin.y + y >= node.top && origin.y + y < node.bottom) {
                focused_child_ = static_cast<long>(index + 1);
                NotifyWinEvent(EVENT_OBJECT_FOCUS, window_, OBJID_CLIENT, focused_child_);
                return;
            }
        }
    }

    static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
        auto* bridge = static_cast<Impl*>(GetPropW(window, kBridgeProperty));
        if (bridge == nullptr) return DefWindowProcW(window, message, w_param, l_param);
        if (message == WM_GETOBJECT && static_cast<LONG>(l_param) == OBJID_CLIENT && bridge->provider_ != nullptr) {
            return LresultFromObject(IID_IAccessible, w_param, bridge->provider_);
        }
        if (message == WM_LBUTTONDOWN) {
            bridge->focusChildAtClientPoint(GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
        }
        const auto original = bridge->original_window_proc_;
        if (message == WM_NCDESTROY) {
            (void)RemovePropW(window, kBridgeProperty);
            bridge->window_ = nullptr;
            bridge->original_window_proc_ = nullptr;
        }
        return original != nullptr ? CallWindowProcW(original, window, message, w_param, l_param)
                                   : DefWindowProcW(window, message, w_param, l_param);
    }

    HWND window_ = nullptr;
    WNDPROC original_window_proc_ = nullptr;
    AccessibleRoot* provider_ = nullptr;
    SnapshotProvider snapshot_provider_;
    ActionHandler action_handler_;
    ValueHandler value_handler_;
    std::string last_tree_fingerprint_;
    long focused_child_ = 0;
    bool ole_initialized_ = false;
};

WindowsAccessibilityBridge::WindowsAccessibilityBridge() : impl_(std::make_unique<Impl>()) {}
WindowsAccessibilityBridge::~WindowsAccessibilityBridge() { uninstall(); }

bool WindowsAccessibilityBridge::install(void* sdl_window, SnapshotProvider snapshot_provider,
                                         ActionHandler action_handler, ValueHandler value_handler) {
    return impl_->install(sdl_window, std::move(snapshot_provider), std::move(action_handler),
                          std::move(value_handler));
}

void WindowsAccessibilityBridge::uninstall() { impl_->uninstall(); }

void WindowsAccessibilityBridge::notifyTreeChanged() {
    if (impl_->window_ != nullptr) NotifyWinEvent(EVENT_OBJECT_REORDER, impl_->window_, OBJID_CLIENT, CHILDID_SELF);
}

void WindowsAccessibilityBridge::synchronizeTree() {
    if (impl_->window_ == nullptr || !impl_->snapshot_provider_) return;
    const auto fingerprint = treeFingerprint(impl_->snapshot_provider_());
    if (fingerprint == impl_->last_tree_fingerprint_) return;
    impl_->last_tree_fingerprint_ = fingerprint;
    NotifyWinEvent(EVENT_OBJECT_REORDER, impl_->window_, OBJID_CLIENT, CHILDID_SELF);
}

bool WindowsAccessibilityBridge::installed() const { return impl_->window_ != nullptr; }

} // namespace urpg::editor_app

#endif
