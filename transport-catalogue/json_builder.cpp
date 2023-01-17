#include "json_builder.h"

namespace json {

bool Builder::IsLastObjectIsArray() const {
    return nodes_stack_.back()->IsArray();
}

bool Builder::IsLastObjectIsDict() const {
    return nodes_stack_.back()->IsDict();
}

void Builder::CheckIfObjectNotBuilt() const {
    if (nodes_stack_.size() == 0 && root_ == Node{}) {
        using namespace std::literals;
        throw LogicError("Json object is not built"s);        
    }
}

void Builder::CheckIfObjectNotFinished() const {
    if (nodes_stack_.size() == 1) {
        using namespace std::literals;
        throw LogicError("Json object is not finished"s);        
    }
}

void Builder::CheckIfObjectFinished() const {
    if (nodes_stack_.size() == 0 && root_ != Node{}) {
        using namespace std::literals;
        throw LogicError("The json object is finished"s);
    }
}

Node Builder::GetValue(Node::Value value) const {
    Node node = Node{};
    if (IsValue<int>(value)) {
        node = Node(std::get<int>(value));
    } else if (IsValue<double>(value)) {
        node = Node(std::get<double>(value));
    } else if (IsValue<bool>(value)) {
        node = Node(std::get<bool>(value));
    } else if (IsValue<std::nullptr_t>(value)) {
        node = Node(std::get<std::nullptr_t>(value));
    } else if (IsValue<std::string>(value)) {
        node = Node(std::get<std::string>(value));
    } else if (IsValue<Array>(value)) {
        node = Node(std::get<Array>(value));
    } else if (IsValue<Dict>(value)) {
        node = Node(std::get<Dict>(value));
    }
    return node;
}

Node Builder::GetNode() const {
    return root_;
}

Builder& Builder::Value(Node::Value value) {
    Node val = GetValue(value);
    if (nodes_stack_.size() == 0) {
        CheckIfObjectFinished();
        root_ = val;

    } else if (IsLastObjectIsArray()) {
        nodes_stack_.back()->AddToArray(val);

    } else if (IsLastObjectIsDict() && key_.IsString()) {
        nodes_stack_.back()->AddToDict(key_.AsString(), val);
        key_ = Node{};

    } 
    return *this;
}

KeyItemContext Builder::Key(std::string str) {
    CheckIfObjectNotBuilt();
    key_ = Node{str};;
    return *this;
}

DictItemContext Builder::StartDict() {
    Node dict = Node(Dict{});
    if (nodes_stack_.size() == 0) {
        CheckIfObjectFinished();
        root_ = dict;
        nodes_stack_.emplace_back(&root_);
        
    } else if (IsLastObjectIsArray()) {
        Node* dict_ptr = nodes_stack_.back()->AddToArray(dict);
        nodes_stack_.emplace_back(dict_ptr);

    } else if (IsLastObjectIsDict() && key_.IsString()) {
        Node* dict_ptr = nodes_stack_.back()->AddToDict(key_.AsString(), dict);
        key_ = Node{};
        nodes_stack_.emplace_back(dict_ptr);
    } 
    return *this;
}

StartArrayItemContext Builder::StartArray() {
    Node arr = Node(Array{});
    if (nodes_stack_.size() == 0) {
        CheckIfObjectFinished();
        root_ = std::move(arr);
        nodes_stack_.emplace_back(&root_);
        return *this;

    } else if (IsLastObjectIsArray()) {
        Node* arr_ptr = nodes_stack_.back()->AddToArray(arr);
        nodes_stack_.emplace_back(arr_ptr);
    
    } else if (IsLastObjectIsDict() && key_.IsString()) {
        Node* arr_ptr = nodes_stack_.back()->AddToDict(key_.AsString(), arr);
        key_ = Node{};
        nodes_stack_.emplace_back(arr_ptr);
    } 
    return *this;
}

Builder& Builder::EndDict() {
    CheckIfObjectNotBuilt();
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    CheckIfObjectNotBuilt();
    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    CheckIfObjectNotBuilt();
    CheckIfObjectNotFinished();
    return root_;
}

ValueAfterKeyItemContext KeyItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}

DictItemContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}

StartArrayItemContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}

KeyItemContext ValueAfterKeyItemContext::Key(std::string str) {
    return builder_.Key(str);
}

Builder& ValueAfterKeyItemContext::EndDict() {
        return builder_.EndDict();
}

KeyItemContext DictItemContext::Key(std::string str) {
    return builder_.Key(str);
}

Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}

ValueAfterStartArrayItemContext StartArrayItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}

DictItemContext StartArrayItemContext::StartDict() {
    return builder_.StartDict();
}

StartArrayItemContext StartArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& StartArrayItemContext::EndArray() {
    return builder_.EndArray();
}

ValueAfterStartArrayItemContext ValueAfterStartArrayItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}

DictItemContext ValueAfterStartArrayItemContext::StartDict() {
    return builder_.StartDict();
}

StartArrayItemContext ValueAfterStartArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& ValueAfterStartArrayItemContext::EndArray() {
    return builder_.EndArray();
}
} //namespace json
