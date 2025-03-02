#include "json_builder.h"
#include <stdexcept>
#include <utility>

namespace json {

    Builder::Builder() {
        nodes_stack_.emplace_back(&root_);
    }

    Node Builder::Build() {
        if (root_.IsNull() || nodes_stack_.size() > 1) {
            throw std::logic_error("Attempt to build JSON which isn't finalized");
        }
        return std::move(root_);
    }

    Builder::DictValueContext Builder::Key(QString key) {
        Node::Value& host_value = GetCurrentValue();

        if (!std::holds_alternative<Dict>(host_value)) {
            throw std::logic_error("Key() outside a dict");
        }

        nodes_stack_.push_back(&std::get<Dict>(host_value)[std::move(key)]);
        return DictValueContext{ *this };
    }

    Builder::BaseContext Builder::Value(Node::Value value) {
        AddObject(std::move(value), true);
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        AddObject(Dict{}, false);
        return DictItemContext{ *this };
    }

    Builder::ArrayItemContext Builder::StartArray() {
        AddObject(Array{}, false);
        return ArrayItemContext{ *this };
    }

    Builder::BaseContext Builder::EndDict() {
        if (!std::holds_alternative<Dict>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside a dict");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder::BaseContext Builder::EndArray() {
        if (!std::holds_alternative<Array>(GetCurrentValue())) {
            throw std::logic_error("EndArray() outside an array");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node::Value& Builder::GetCurrentValue() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Attempt to change finalized JSON");
        }
        return nodes_stack_.back()->GetValue();
    }

    const Node::Value& Builder::GetCurrentValue() const {
        return const_cast<Builder*>(this)->GetCurrentValue();
    }

    void Builder::AssertNewObjectContext() const {
        if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
            throw std::logic_error("New object in wrong context");
        }
    }

    void Builder::AddObject(Node::Value value, bool one_shot) {
        Node::Value& host_value = GetCurrentValue();
        if (std::holds_alternative<Array>(host_value)) {
            Node& node = std::get<Array>(host_value).emplace_back(std::move(value));
            if (!one_shot) {
                nodes_stack_.push_back(&node);
            }
        } else {
            AssertNewObjectContext();
            host_value = std::move(value);
            if (one_shot) {
                nodes_stack_.pop_back();
            }
        }
    }

}  // namespace json
