// --------------------------- CNode ---------------------------
// This is the base class for all scene graph nodes.
// Use the Add / Remove functions to link nodes together in a tree structure.
//
//             parent
//               |
//          prev-*-next
//              / \
//   child_first   child_last
//
//--------------------------------------------------------------

#ifndef CNODE_H
#define CNODE_H

typedef unsigned int uint;

class CNode {
  friend class CObject;
  friend class CRenderer;
private:
    CNode* _child_first;
    CNode* _child_last;
    CNode* _parent;
    CNode* _prev;
    CNode* _next;

public:
   CNode(): _child_first(0), _child_last(0), _parent(0), _prev(0), _next(0){}
   CNode(const CNode&):_child_first(0), _child_last(0), _parent(0), _prev(0), _next(0){}  // copy-constructor
   virtual ~CNode(){Remove();}

   //--Manage scene hierarchy tree structure--
   CNode* Parent(){return _parent;}                // Parent node in scene tree.
   virtual CNode& Add(CNode &item);                // Add child to end of child list.
   virtual void   Remove();                        // Remove node from tree structure.

   //--Child-iteration--
   CNode* FirstChild(){return _child_first;}
   CNode* Next(){return _next;};
   //-------------------
};

#endif
