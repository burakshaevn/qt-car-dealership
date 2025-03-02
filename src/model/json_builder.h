#pragma once

#include "json.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <QString>

namespace json {

    class Builder {
    private:
        class BaseContext;
        class DictValueContext;
        class DictItemContext;
        class ArrayItemContext;

    public:
        Builder();
        Node Build();
        DictValueContext Key(QString key);
        BaseContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();

    private:
        Node root_{ nullptr };
        std::vector<Node*> nodes_stack_;
        std::optional<QString> key_{ std::nullopt };

        Node::Value& GetCurrentValue();
        const Node::Value& GetCurrentValue() const;

        void AssertNewObjectContext() const;
        void AddObject(Node::Value value, bool one_shot);

        class BaseContext {
        public:
            BaseContext(Builder& builder)
                : builder_(builder)
            {}
            Node Build() {
                return builder_.Build();
            }
            DictValueContext Key(QString key) {
                return builder_.Key(std::move(key));
            }
            BaseContext Value(Node::Value value) {
                return builder_.Value(std::move(value));
            }
            DictItemContext StartDict() {
                return builder_.StartDict();
            }
            ArrayItemContext StartArray() {
                return builder_.StartArray();
            }
            BaseContext EndDict() {
                return builder_.EndDict();
            }
            BaseContext EndArray() {
                return builder_.EndArray();
            }
        protected:
            Builder& builder_;
        };

        class DictValueContext : public BaseContext {
        public:
            using BaseContext::BaseContext;
            DictItemContext Value(Node::Value value) {
                BaseContext::Value(std::move(value));
                return DictItemContext(builder_);
            }
            Node Build() = delete;
            DictValueContext Key(QString key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;
        };

        class DictItemContext : public BaseContext {
        public:
            using BaseContext::BaseContext;
            BaseContext Value(Node::Value value) = delete;
            BaseContext EndArray() = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
        };

        class ArrayItemContext : public BaseContext {
        public:
            using BaseContext::BaseContext;
            ArrayItemContext Value(Node::Value value) {
                BaseContext::Value(std::move(value));
                return ArrayItemContext(builder_);
            }
            Node Build() = delete;
            DictValueContext Key(QString key) = delete;
            BaseContext EndDict() = delete;
        };
    };
 
}  // namespace json
