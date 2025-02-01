#include "CNode.h"

//--------------------Scene Graph Structure-----------------------
CNode& CNode::Add(CNode &item){
  item.Remove();        // If item is already in a tree, remove it first.
  item._parent=this;
  if(!_child_last) _child_first=_child_last=&item;
  else{ item._prev=_child_last; _child_last->_next=&item; _child_last=&item; }
  return *this;
}

void CNode::Remove(){
  if(!_parent) return;
  if(_prev) _prev->_next=_next;
  if(_next) _next->_prev=_prev;
  if(_parent->_child_first==this) _parent->_child_first=_next;
  if(_parent->_child_last ==this) _parent->_child_last =_prev;
  _prev=_next=_parent=0;
}
//----------------------------------------------------------------

uint CNode::Level() {
    uint level = 0;
    CNode* item = this;
    while(!!item->Parent()) {item = item->Parent(); level++;}
    return level;
};
