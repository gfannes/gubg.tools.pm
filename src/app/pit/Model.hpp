#ifndef HEADER_pit_Model_hpp_ALREADY_INCLUDED
#define HEADER_pit_Model_hpp_ALREADY_INCLUDED

#include <pit/Types.hpp>
#include <pit/TagPath.hpp>
#include <pit/ResourceMgr.hpp>
#include <gubg/xtree/Model.hpp>
#include <gubg/mss.hpp>
#include <gubg/std/filesystem.hpp>
#include <gubg/file/System.hpp>
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
        bool traverse(const Node_cptr &node, Ftor &&ftor, bool show_xlinks) const;
        template <typename Ftor>
        bool traverse(const Node_ptr &node, Ftor &&ftor, bool show_xlinks);

        Node_ptr resolve(const std::string &dep, const Node &from, std::string *error = nullptr) const;

        std::string str() const;

    private:
        bool parse_(Node &parent, gubg::naft::Range &range);

        XTree xtree_;
        std::map<TagPath, Node_ptr> tagpath__node_;
    };

    template <typename Ftor>
    bool Model::traverse(const Node_cptr &node, Ftor &&ftor, bool show_xlinks) const
    {
        MSS_BEGIN(bool);
        MSS(!!node);
        MSS(xtree_.traverse(ftor, show_xlinks, node));
        MSS_END();
    }
    template <typename Ftor>
    bool Model::traverse(const Node_ptr &node, Ftor &&ftor, bool show_xlinks)
    {
        MSS_BEGIN(bool);
        MSS(!!node);
        MSS(xtree_.traverse(ftor, show_xlinks, node));
        MSS_END();
    }

} 

#endif
