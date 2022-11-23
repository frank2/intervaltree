#include <framework.hpp>
#include <intervaltree.hpp>

#include <cstdint>
#include <cstddef>

using namespace intervaltree;

int
test_interval
()
{
   INIT();
   
   using ClosedInterval = Interval<std::size_t, false>;
   using OpenInterval = Interval<std::size_t, true>;

   ClosedInterval closed(20, 40);
   OpenInterval open(20, 40);

   ASSERT(closed.contains(10) == false);
   ASSERT(closed.contains(20) == true);
   ASSERT(closed.contains(30) == true);
   ASSERT(closed.contains(40) == false);
   ASSERT(closed.contains(50) == false);
   ASSERT(open.contains(10) == false);
   ASSERT(open.contains(20) == true);
   ASSERT(open.contains(30) == true);
   ASSERT(open.contains(40) == true);
   ASSERT(open.contains(50) == false);

   ASSERT(closed.contains(10,20) == false);
   ASSERT(closed.contains(20,30) == true);
   ASSERT(closed.contains(30,40) == true);
   ASSERT(closed.contains(40,50) == false);
   ASSERT(open.contains(10,20) == false);
   ASSERT(open.contains(20,30) == true);
   ASSERT(open.contains(30,40) == true);
   ASSERT(open.contains(40,50) == false);

   ASSERT(closed.overlaps(10,20) == false);
   ASSERT(closed.overlaps(20,30) == true);
   ASSERT(closed.overlaps(30,40) == true);
   ASSERT(closed.overlaps(40,50) == false);
   ASSERT(open.overlaps(10,20) == true);
   ASSERT(open.overlaps(20,30) == true);
   ASSERT(open.overlaps(30,40) == true);
   ASSERT(open.overlaps(40,50) == true);

   ASSERT(closed.contained_by(10,30) == false);
   ASSERT(closed.contained_by(10,40) == true);
   ASSERT(closed.contained_by(10,50) == true);
   ASSERT(closed.contained_by(20,30) == false);
   ASSERT(closed.contained_by(20,40) == true);
   ASSERT(closed.contained_by(20,50) == true);
   ASSERT(closed.contained_by(30,50) == false);
   ASSERT(open.contained_by(10,30) == false);
   ASSERT(open.contained_by(10,40) == true);
   ASSERT(open.contained_by(10,50) == true);
   ASSERT(open.contained_by(20,30) == false);
   ASSERT(open.contained_by(20,40) == true);
   ASSERT(open.contained_by(20,50) == true);
   ASSERT(open.contained_by(30,50) == false);

   ASSERT(closed.join(10,30) == ClosedInterval(10,40));
   ASSERT(closed.join(30,50) == ClosedInterval(20,50));
   ASSERT(closed.join(10,50) == ClosedInterval(10,50));

   ASSERT(closed.size() == 20);
   ASSERT(open.size() == 21);
   
   COMPLETE();
}

int
test_intervaltree
()
{
   INIT();

   using IntervalType = Interval<std::size_t>;
   using TreeType = IntervalTree<IntervalType>;
   
   std::vector<IntervalType> wiki_nodes = {
      IntervalType(20,36),
      IntervalType(29,99),
      IntervalType(3,41),
      IntervalType(0,1),
      IntervalType(10,15)
   };
   TreeType wiki_tree(wiki_nodes);

   ASSERT(wiki_tree.containing_point(35) == TreeType::SetType({IntervalType(20,36), IntervalType(29,99), IntervalType(3,41)}));
   ASSERT(wiki_tree.containing_interval(IntervalType(11,14)) == TreeType::SetType({IntervalType(3,41), IntervalType(10,15)}));
   ASSERT(wiki_tree.overlapping_interval(IntervalType(0,25)) == TreeType::SetType({IntervalType(20,36), IntervalType(3,41), IntervalType(0,1), IntervalType(10,15)}));
   ASSERT(wiki_tree.contained_by_interval(IntervalType(0,41)) == TreeType::SetType({IntervalType(0,1), IntervalType(3,41), IntervalType(10,15), IntervalType(20,36)}));

   TreeType fuzz_tree(std::vector<IntervalType>({
            IntervalType(8,12),
            IntervalType(8,11),
            IntervalType(9,12),
            IntervalType(0,2),
            IntervalType(0,4),
            IntervalType(2,6),
            IntervalType(4,8),
            IntervalType(6,8),
            IntervalType(10,14),
            IntervalType(12,14),
            IntervalType(14,22),
            IntervalType(4,24)
         }));

   ASSERT(fuzz_tree.containing_interval(IntervalType(9,11)) == TreeType::SetType({IntervalType(8,12),IntervalType(8,11),IntervalType(9,12),IntervalType(4,24)}));

   auto deoverlapped = fuzz_tree.deoverlap();
   ASSERT(deoverlapped.to_vec() == std::vector<IntervalType>({IntervalType(0,24)}));
   
   COMPLETE();
}

int
main
(int argc, char *argv[])
{
   INIT();

   LOG_INFO("Testing Interval.");
   PROCESS_RESULT(test_interval);

   LOG_INFO("Testing IntervalTree.");
   PROCESS_RESULT(test_intervaltree);
      
   COMPLETE();
}
