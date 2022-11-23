#ifndef __INTERVALTREE_H
#define __INTERVALTREE_H

#include <set>

#include <avltree.hpp>

namespace intervaltree
{
   namespace exception {
      using namespace avltree::exception;
   }

   template<typename Key, typename KeyCompare>
   using AVLTree = avltree::AVLTree<Key, KeyCompare>;

   template <typename _ValueType, bool Inclusive=false>
   struct Interval
   {
      using ValueType = typename _ValueType;
      static const bool Inclusive = Inclusive;
      
      _ValueType low;
      _ValueType high;

      Interval() : low(0), high(0) {}
      Interval(const _ValueType &low, const _ValueType &high) {
         if (low < high) { this->low = low; this->high = high; }
         else { this->low = high; this->high = low; }
      }
      Interval(const Interval &other) : low(other.low), high(other.high) {}

      struct Compare {
         bool operator() (const Interval &left, const Interval &right) const {
            if (left == right) { return false; }

            if (left.low != right.low)
               return left.low < right.low;
            else
               return left.high < right.high;
         }
      };

      bool operator==(const Interval &other) const {
         return this->low == other.low && this->high == other.high;
      }
      bool operator!=(const Interval &other) const {
         return this->low != other.low || this->high != other.high;
      }

      inline bool overlaps(const _ValueType &low, const _ValueType &high) const {
         if constexpr (Inclusive) { return this->low <= high && low <= this->high; }
         else { return this->low < high && low < this->high; }
      }
      inline bool overlaps(const Interval &other) const {
         return this->overlaps(other.low, other.high);
      }
      inline bool contains(const _ValueType &point) const {
         if constexpr (Inclusive) { return point >= this->low && point <= this->high; }
         else { return point >= this->low && point < this->high; }
      }
      inline bool contains(const _ValueType &low, const _ValueType &high) const {
         return low >= this->low && high <= this->high;
      }
      inline bool contains(const Interval &other) const {
         return this->contains(other.low, other.high);
      }
      inline bool contained_by(const Interval &other) const {
         return other.contains(*this);
      }
      inline bool contained_by(const _ValueType &low, const _ValueType &high) const {
         return this->contained_by(Interval(low, high));
      }

      inline Interval join(const _ValueType &low, const _ValueType &high) const {
         return Interval(std::min(this->low, low), std::max(this->high, high));
      }

      inline Interval join(const Interval &other) const {
         return this->join(other.low, other.high);
      }
         
      inline _ValueType size() const {
         if constexpr (Inclusive) { return (this->high - this->low) + 1; }
         else { return this->high - this->low; }
      }
   };

   template <typename IntervalType>
   class IntervalTree : public AVLTree<IntervalType, typename IntervalType::Compare>
   {
      static_assert(std::is_base_of<Interval<typename IntervalType::ValueType, IntervalType::Inclusive>, IntervalType>::value,
                    "IntervalType template argument must derive the Interval structure.");
      
   public:
      using SetType = std::set<IntervalType, typename IntervalType::Compare>;
      
      class IntervalNode : public AVLTree::Node
      {
      protected:
         typename IntervalType::ValueType _max;

      public:
         friend class IntervalTree;
         
         IntervalNode() : _max(0), AVLTree::Node() {}
         IntervalNode(const typename AVLTree::ValueType &value) : _max(value.high), AVLTree::Node(value) {}
         IntervalNode(const IntervalNode &other) : _max(other._max), AVLTree::Node(other) {}

         virtual void copy_node_data(const typename AVLTree::Node &node) {
            AVLTree::Node::copy_node_data(node);

            auto int_node = dynamic_cast<const IntervalNode &>(node);
            this->_max = int_node._max;
         }
         
         inline typename IntervalType::ValueType &max() { return this->_max; }
         inline const typename IntervalType::ValueType &max() const { return this->_max; }
         typename IntervalType::ValueType new_max() const {
            auto left = std::dynamic_pointer_cast<const IntervalNode>(this->left());
            auto right = std::dynamic_pointer_cast<const IntervalNode>(this->right());

            auto left_max = (left != nullptr) ? left->max() : this->value().high;
            auto right_max = (right != nullptr) ? right->max() : this->value().high;

            return std::max(this->value().high, std::max(left_max, right_max));
         }
      };

   protected:
      void update_max(typename AVLTree::SharedNode node) {
         typename AVLTree::SharedNode update = node;

         while (update != nullptr)
         {
            auto int_node = std::static_pointer_cast<IntervalNode>(update);
            int_node->_max = int_node->new_max();

            auto parent = std::static_pointer_cast<IntervalNode>(int_node->parent());

            if (parent != nullptr && parent->max() != int_node->max())
               update = parent;
            else
               update = nullptr;
         }
      }

      virtual void rotate_left(typename AVLTree::SharedNode node) {
         AVLTree::rotate_left(node);
         this->update_max(node);
      }

      virtual void rotate_right(typename AVLTree::SharedNode node) {
         AVLTree::rotate_right(node);
         this->update_max(node);
      }

      virtual typename AVLTree::SharedNode allocate_node(const typename AVLTree::ValueType &value) {
         auto node = std::make_shared<IntervalNode>(value);

         return node;
      }

      virtual typename AVLTree::SharedNode copy_node(typename AVLTree::ConstSharedNode node) {
         auto upcast_node = std::static_pointer_cast<const IntervalNode>(node);
         auto new_node = std::make_shared<IntervalNode>(*upcast_node);

         return new_node;
      }

      virtual void update_node(typename AVLTree::SharedNode node) {
         this->update_max(node);
         return AVLTree::update_node(node);
      }

   public:
      IntervalTree() : AVLTree() {}
      IntervalTree(std::vector<IntervalType> &nodes) : AVLTree(nodes) {}
      IntervalTree(const IntervalTree &other) : AVLTree(other) {}

      std::shared_ptr<IntervalNode> insert(const IntervalType &interval) {
         return std::static_pointer_cast<IntervalNode>(AVLTree::insert(interval));
      }

      std::shared_ptr<IntervalNode> insert_overlap(const IntervalType &interval)
      {
         auto overlaps = this->overlapping_interval(interval);
         auto final_interval = interval;

         if (overlaps.size() != 0)
         {
            for (auto overlap : overlaps)
            {
               final_interval = final_interval.join(overlap);
               this->remove(overlap);
            }
         }

         return this->insert(final_interval);
      }

      IntervalTree deoverlap() {
         auto values = this->to_vec();
         IntervalTree new_tree;

         for (auto value : values)
            new_tree.insert_overlap(value);

         return new_tree;
      }
         
      SetType containing_point(const typename IntervalType::ValueType &point) const {
         auto result = SetType();
         if (this->root() == nullptr) { return result; }

         std::vector<typename AVLTree::ConstSharedNode> traversal = { this->root() };

         while (traversal.size() > 0)
         {
            auto node = traversal.front();
            traversal.erase(traversal.begin());
            if (node == nullptr) { continue; }

            auto int_node = std::static_pointer_cast<const IntervalNode>(node);

            if (int_node->value().contains(point))
               result.insert(int_node->value());
            
            if constexpr (IntervalType::Inclusive)
            {
               if (point <= int_node->max())
                  traversal.push_back(int_node->left());
            }
            else {
               if (point < int_node->max())
                  traversal.push_back(int_node->left());
            }

            if (point >= int_node->value().low)
               traversal.push_back(int_node->right());
         }

         return result;
      }

      SetType containing_interval(const IntervalType &interval) const {
         auto result = SetType();
         if (this->root() == nullptr) { return result; }

         std::vector<typename AVLTree::ConstSharedNode> traversal = { this->root() };

         while (traversal.size() > 0)
         {
            auto node = traversal.front();
            traversal.erase(traversal.begin());
            if (node == nullptr) { continue; }

            auto int_node = std::static_pointer_cast<const IntervalNode>(node);

            if (int_node->value().contains(interval))
               result.insert(int_node->value());

            if constexpr (IntervalType::Inclusive)
            {
               if (interval.low <= int_node->max())
                  traversal.push_back(int_node->left());
            }
            else
            {
               if (interval.low < int_node->max())
                  traversal.push_back(int_node->left());
            }
            
            if (interval.high >= int_node->value().low)
               traversal.push_back(int_node->right());
         }

         return result;
      }

      SetType overlapping_interval(const IntervalType &interval) const {
         auto result = SetType();
         if (this->root() == nullptr) { return result; }

         std::vector<typename AVLTree::ConstSharedNode> traversal = { this->root() };
         auto int_root = std::static_pointer_cast<const IntervalNode>(this->root());
         auto int_leftmost = int_root;

         while (int_leftmost->left() != nullptr)
            int_leftmost = std::static_pointer_cast<const IntervalNode>(int_leftmost->left());

         // check if this interval overlaps all possible nodes
         if (interval.low <= int_leftmost->value().low && interval.high >= int_root->max())
         {
            auto vec = this->to_vec();
            return SetType(vec.begin(), vec.end());
         }

         while (traversal.size() > 0)
         {
            auto node = traversal.front();
            traversal.erase(traversal.begin());
            if (node == nullptr) { continue; }

            auto int_node = std::static_pointer_cast<const IntervalNode>(node);

            if (int_node->value().overlaps(interval))
               result.insert(int_node->value());
            
            if constexpr (IntervalType::Inclusive)
            {
               if (interval.low <= int_node->max())
                  traversal.push_back(int_node->left());
            }
            else
            {
               if (interval.low < int_node->max())
                  traversal.push_back(int_node->left());
            }
            
            if (interval.high >= int_node->value().low)
               traversal.push_back(int_node->right());
         }

         return result;
      }

      SetType contained_by_interval(const IntervalType &interval) const {
         auto result = SetType();
         if (this->root() == nullptr) { return result; }

         std::vector<typename AVLTree::ConstSharedNode> traversal = { this->root() };
         auto int_root = std::static_pointer_cast<const IntervalNode>(this->root());
         auto int_leftmost = int_root;

         while (int_leftmost->left() != nullptr)
            int_leftmost = std::static_pointer_cast<const IntervalNode>(int_leftmost->left());

         // check if this interval contains all possible nodes
         if (interval.low <= int_leftmost->value().low && interval.high >= int_root->max())
         {
            auto vec = this->to_vec();
            return SetType(vec.begin(), vec.end());
         }

         while (traversal.size() > 0)
         {
            auto node = traversal.front();
            traversal.erase(traversal.begin());
            if (node == nullptr) { continue; }

            auto int_node = std::static_pointer_cast<const IntervalNode>(node);

            if (int_node->value().contained_by(interval))
               result.insert(int_node->value());
            
            if constexpr (IntervalType::Inclusive)
            {
               if (interval.low <= int_node->max())
                  traversal.push_back(int_node->left());
            }
            else
            {
               if (interval.low < int_node->max())
                  traversal.push_back(int_node->left());
            }
            
            if (interval.high >= int_node->value().low)
               traversal.push_back(int_node->right());
         }

         return result;
      }
   };
}

#endif
