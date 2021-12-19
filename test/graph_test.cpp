#include "../src/Graph.hpp"
#include <boost/graph/adjacency_list.hpp>

#include "gtest/gtest.h"

using namespace boost;

// The fixture for testing class Foo.
class GraphTest : public ::testing::Test {
   protected:
   // You can remove any or all of the following functions if their bodies would
   // be empty.

   GraphTest() {
      // You can do set-up work for each test here.
   }

   ~GraphTest() override {
      // You can do clean-up work that doesn't throw exceptions here.
   }

   // If the constructor and destructor are not enough for setting up
   // and cleaning up each test, you can define the following methods:
   Graph::Graph_bst generate_line2() {
      Graph::Graph_bst g(2);
      add_edge(0, 1, g);
      return g;
   }

   Graph::Graph_bst generate_line3() {
      Graph::Graph_bst g(3);
      add_edge(0, 1, g);
      add_edge(1, 2, g);
      return g;
   }

   Graph::Graph_bst generate_triangle() {
      Graph::Graph_bst g(3);
      add_edge(0, 1, g);
      add_edge(1, 2, g);
      add_edge(2, 0, g);
      return g;
   }

   Graph::Graph_bst generate_square() {
      Graph::Graph_bst g(3);
      add_edge(0, 1, g);
      add_edge(1, 2, g);
      add_edge(2, 3, g);
      add_edge(3, 0, g);
      return g;
   }

   Graph::Graph_bst generate_clique(int n) {
      Graph::Graph_bst g(n);
      for (int i = 0; i < n - 1; i++) {
         for (int j = i + 1; j < n; j++) {
            add_edge(i, j, g);
         }
      }
      return g;
   }

   Graph::Graph_bst generate_graph1() {
      Graph::Graph_bst g(6);
      add_edge(0, 1, g);
      add_edge(0, 2, g);
      add_edge(1, 2, g);
      add_edge(1, 3, g);
      add_edge(2, 3, g);
      add_edge(0, 4, g);
      add_edge(4, 5, g);

      return g;
   }

   Graph::DGraph_bst generate_graph1_dag() {
      Graph::DGraph_bst g(6);
      add_edge(0, 1, g);
      add_edge(0, 2, g);
      add_edge(0, 4, g);
      add_edge(1, 2, g);
      add_edge(1, 3, g);
      add_edge(2, 3, g);
      add_edge(4, 5, g);

      return g;
   }

   Graph::Graph_bst generate_graph2() {
      Graph::Graph_bst g(9);
      add_edge(1, 2, g);
      add_edge(1, 3, g);
      add_edge(1, 4, g);
      add_edge(2, 5, g);
      add_edge(2, 6, g);
      add_edge(3, 7, g);
      add_edge(4, 8, g);
      add_edge(6, 7, g);
      add_edge(7, 9, g);

      return g;
   }

   Graph::DGraph_bst generate_graph2_dag() {
      Graph::DGraph_bst g(9);
      add_edge(1, 2, g);
      add_edge(1, 3, g);
      add_edge(1, 4, g);
      add_edge(2, 5, g);
      add_edge(2, 6, g);
      add_edge(3, 7, g);
      add_edge(4, 8, g);
      add_edge(6, 7, g);
      add_edge(7, 9, g);

      return g;
   }

   void SetUp() override {
      // Code here will be called immediately after the constructor (right
      // before each test).
   }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
  }
      // Class members declared here can be used by all tests in the test suite
      // for Foo.
   };

TEST_F(GraphTest, LoadFromFileSmall) {
   vector<string> test_graph_names = {"line2", "line3", "triangle", "square", "clique4"};
   vector<Graph::Graph_bst> correct_graphs = {
      generate_line2(), 
      generate_line3(),
      generate_triangle(),
      generate_square(),
      generate_clique(4)
      };

   for (int i = 0; i < test_graph_names.size(); i++) {
      Graph g;
      string test_graph_path = "../test/graph/" + test_graph_names[i] + ".txt";
      ifstream graph_file(test_graph_path);
      if (!graph_file.is_open() || !g.load_from_file(graph_file)) {
         cerr << "graph file " << test_graph_path << " does not exist.\n";
         FAIL();
      } else {
         // TODO: We also need to check if edges are matched. Further, if nodes have labels, we need to check labels.
         // TODO: We need a function here to check if two bst graphs are the same. (both directed and undirected)
         EXPECT_EQ(num_vertices(g.graph_), num_vertices(correct_graphs[i]));
         EXPECT_EQ(num_edges(g.graph_), num_edges(correct_graphs[i]));

         graph_file.close();
      }

   }
}

TEST_F(GraphTest, GenerateDAG) {
   vector<pair<Graph::Graph_bst, int>> graphs = {
      make_pair(generate_graph1(), 0),
      make_pair(generate_graph2(), 1)
      };


   for (int i = 0; i < graphs.size(); i++) {
      Graph g(graphs[i].first);
      int rootIndex = graphs[i].second;
      Graph::DGraph_bst dag = g.generate_dag(rootIndex);

      // Check rootIndex is not in any adj list.
      graph_traits <Graph::DGraph_bst>::vertex_iterator nodeI, nodeEnd;
      graph_traits <Graph::DGraph_bst>::adjacency_iterator nbrI, nbrEnd;
      for (boost::tie(nodeI, nodeEnd) = vertices(dag); nodeI != nodeEnd; ++nodeI)
         for (boost::tie(nbrI, nbrEnd) = adjacent_vertices(*nodeI, dag); nbrI != nbrEnd; ++nbrI)
            EXPECT_NE(*nbrI, rootIndex);

      EXPECT_EQ(num_vertices(dag), num_vertices(g.graph_));
      EXPECT_EQ(num_edges(dag), num_edges(g.graph_));
   }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}