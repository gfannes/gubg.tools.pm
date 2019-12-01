#ifndef HEADER_pit_Model_hpp_ALREADY_INCLUDED
#define HEADER_pit_Model_hpp_ALREADY_INCLUDED

#include <pit/Types.hpp>
#include <pit/TagPath.hpp>
#include <pit/ResourceMgr.hpp>
#include <gubg/xtree/Model.hpp>
#include <gubg/mss.hpp>
#include <gubg/std/filesystem.hpp>
#include <gubg/file/system.hpp>
#include <gubg/std/optional.hpp>
#include <gubg/naft/Range.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <list>

namespace pit { 

    class Model
    {
    public:
        using XTree = gubg::xtree::Model<Data>;
        using Node = typename XTree::Node;
        using Node_ptr = typename XTree::Node_ptr;
        using Node_cptr = typename XTree::Node_cptr;

        bool load(const InputFiles &input_files);

        bool plan(ResourceMgr &resource_mgr);

        bool aggregate();

        Node_cptr root() const {return xtree_.root_cptr();}
        Node_ptr root() {return xtree_.root_ptr();}

        Node &add_child(Node &parent, const Tag &tag);

        template <typename Ftor>
        void traverse(const Node_cptr &node, Ftor &&ftor) const { xtree_.traverse(ftor, node); }
        template <typename Ftor>
        void traverse(const Node_ptr &node, Ftor &&ftor) { xtree_.traverse(ftor, node); }

        Node_ptr resolve(const std::string &dep, const Node &from, std::string *error = nullptr) const;

        std::string str() const;

        const Days &work_days() const {return work_days_;}

    private:
        bool parse_(Node &parent, gubg::naft::Range &range);

        XTree xtree_;
        std::map<TagPath, Node_ptr> tagpath__node_;
        Days work_days_;
    };

} 

#endif
