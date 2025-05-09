#include "node.hpp"

#include "node_create_collection.hpp"
#include "node_create_database.hpp"
#include "node_drop_collection.hpp"
#include "node_drop_database.hpp"

#include <algorithm>
#include <boost/container_hash/hash.hpp>

namespace components::logical_plan {

    node_t::node_t(std::pmr::memory_resource* resource, node_type type, const collection_full_name_t& collection)
        : type_(type)
        , collection_(collection)
        , children_(resource)
        , expressions_(resource) {}

    node_type node_t::type() const { return type_; }

    const collection_full_name_t& node_t::collection_full_name() const { return collection_; }

    const database_name_t& node_t::database_name() const { return collection_.database; }

    const collection_name_t& node_t::collection_name() const { return collection_.collection; }

    const std::pmr::vector<node_ptr>& node_t::children() const { return children_; }
    std::pmr::vector<node_ptr>& node_t::children() { return children_; }

    const std::pmr::vector<expression_ptr>& node_t::expressions() const { return expressions_; }

    void node_t::reserve_child(std::size_t count) { children_.reserve(count); }

    void node_t::append_child(const node_ptr& child) { children_.push_back(child); }

    void node_t::append_expression(const expression_ptr& expression) { expressions_.push_back(expression); }

    void node_t::append_expressions(const std::vector<expression_ptr>& expressions) {
        expressions_.reserve(expressions_.size() + expressions.size());
        std::copy(expressions.begin(), expressions.end(), std::back_inserter(expressions_));
    }
    void node_t::append_expressions(const std::pmr::vector<expression_ptr>& expressions) {
        expressions_.reserve(expressions_.size() + expressions.size());
        std::copy(expressions.begin(), expressions.end(), std::back_inserter(expressions_));
    }

    std::unordered_set<collection_full_name_t, collection_name_hash> node_t::collection_dependencies() {
        std::unordered_set<collection_full_name_t, collection_name_hash> dependencies{collection_full_name()};
        for (const auto& child : children_) {
            child->collection_dependencies_(dependencies);
        }
        return dependencies;
    }

    hash_t node_t::hash() const {
        hash_t hash_{0};
        boost::hash_combine(hash_, type_);
        boost::hash_combine(hash_, hash_impl());
        std::for_each(expressions_.cbegin(), expressions_.cend(), [&hash_](const expression_ptr& expression) {
            boost::hash_combine(hash_, expression->hash());
        });
        std::for_each(children_.cbegin(), children_.cend(), [&hash_](const node_ptr& child) {
            boost::hash_combine(hash_, child->hash());
        });
        return hash_;
    }

    std::string node_t::to_string() const { return to_string_impl(); }

    std::pmr::memory_resource* node_t::resource() const noexcept { return children_.get_allocator().resource(); }

    bool node_t::operator==(const node_t& rhs) const {
        bool result = type_ == rhs.type_ && children_.size() == rhs.children_.size() &&
                      expressions_.size() == rhs.expressions_.size();
        if (result) {
            for (auto it = children_.cbegin(), it2 = rhs.children_.cbegin(), it_end = children_.cend(); it != it_end;
                 ++it, ++it2) {
                result &= (*it == *it2);
            }
            if (result) {
                for (auto it = expressions_.cbegin(), it2 = rhs.expressions_.cbegin(), it_end = expressions_.cend();
                     it != it_end;
                     ++it, ++it2) {
                    result &= (*it == *it2);
                }
            }
        }
        return result;
    }

    bool node_t::operator!=(const node_t& rhs) const { return !operator==(rhs); }

    void node_t::collection_dependencies_(
        std::unordered_set<collection_full_name_t, collection_name_hash>& upper_dependencies) {
        upper_dependencies.insert(collection_full_name());
        for (const auto& child : children_) {
            child->collection_dependencies_(upper_dependencies);
        }
    }
    node_ptr to_node_default(const msgpack::object& msg_object, node_type type, std::pmr::memory_resource* resource) {
        if (msg_object.type != msgpack::type::ARRAY) {
            throw msgpack::type_error();
        }
        if (msg_object.via.array.size != 2) {
            throw msgpack::type_error();
        }
        auto database = msg_object.via.array.ptr[0].as<std::string>();
        auto collection = msg_object.via.array.ptr[1].as<std::string>();
        switch (type) {
            case node_type::create_collection_t:
                return make_node_create_collection(resource, {database, collection});
            case node_type::create_database_t:
                return make_node_create_database(resource, {database, collection});
            case node_type::drop_collection_t:
                return make_node_drop_collection(resource, {database, collection});
            case node_type::drop_database_t:
                return make_node_drop_database(resource, {database, collection});
            default:
                assert(false && "unsupported node_type for to_node_default()");
                return nullptr;
        }
    }

} // namespace components::logical_plan
