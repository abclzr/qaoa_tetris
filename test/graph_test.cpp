#include "../src/Graph.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;

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
   Graph generate_line2(bool directed = false) {
      Graph g(2, directed);
      g.add_edge(0, 1);
      return g;
   }

   Graph generate_line3(bool directed = false) {
      Graph g(3, directed);
      g.add_edge(0, 1);
      g.add_edge(1, 2);
      return g;
   }

   Graph generate_triangle(bool directed = false) {
      Graph g(3, directed);
      g.add_edge(0, 1);
      g.add_edge(1, 2);
      g.add_edge(2, 0);
      return g;
   }

   Graph generate_square(bool directed = false) {
      Graph g(3, directed);
      g.add_edge(0, 1);
      g.add_edge(1, 2);
      g.add_edge(2, 3);
      g.add_edge(3, 0);
      return g;
   }

   Graph generate_clique(int n, bool directed = false) {
      Graph g(n, directed);
      for (int i = 0; i < n - 1; i++) {
         for (int j = i + 1; j < n; j++) {
            g.add_edge(i, j);
         }
      }
      return g;
   }


   /**
    * @brief This is the query graph in Figure 1.
    * 
    * @return Graph::Graph_bst 
    */
   Graph generate_fig1q() {
      Graph g(4, false);
      g.add_edge(0, 1);
      g.add_edge(0, 2);
      g.add_edge(1, 2);
      g.add_edge(1, 3);
      g.add_edge(2, 3);

      return g;
   }

   /**
    * @brief This is the graph in Figure 5.
    * 
    * @return Graph::Graph_bst 
    */
   Graph generate_fig5(bool directed = false) {
      Graph g(9, directed);
      g.add_edge(0, 1);
      g.add_edge(0, 2);
      g.add_edge(0, 3);
      g.add_edge(1, 4);
      g.add_edge(1, 5);
      g.add_edge(2, 6);
      g.add_edge(3, 7);
      g.add_edge(5, 6);
      g.add_edge(6, 8);

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

TEST_F(GraphTest, LoadFromFileTest) {
   vector<string> test_graph_names = {"line2", "line3", "triangle", "square", "clique4"};
   vector<Graph> correct_graphs = {
      generate_line2(true), 
      generate_line3(true),
      generate_triangle(false),
      generate_square(false),
      generate_clique(4, true)
      };
   vector<bool> graph_directed = {
      true, true, false, false, true
   };

   for (int i = 0; i < test_graph_names.size(); i++) {
      Graph g;
      // XXX: modify the current testing/working directory in cmakefile
      string test_graph_path = "../../test/graph/" + test_graph_names[i] + ".txt";
      ifstream graph_file(test_graph_path);
      if (!graph_file.is_open() || !g.load_from_file(graph_file, graph_directed[i])) {
         cerr << "Cannot load graph file " << test_graph_path << ".\n";
         FAIL();
      } else {
         // TODO: We also need to check if edges are matched. Further, if nodes have labels, we need to check labels.
         // TODO: We need a function here to check if two bst graphs are the same. (both directed and undirected)
         EXPECT_EQ(g.num_nodes(), correct_graphs[i].num_nodes());
         EXPECT_EQ(g.num_edges(), correct_graphs[i].num_edges());

         graph_file.close();
      }

   }
}

TEST_F(GraphTest, DirectedGraphDegreeTest) {
   Graph g = generate_fig5(true);
   vector<int> in_degrees = {0, 1, 1, 1, 1, 1, 2, 1, 1};
   vector<int> out_degrees = {3, 2, 1, 1, 0, 1, 1, 0, 0};
   for (int i = 0; i < g.num_nodes(); i++) {
      EXPECT_EQ(g.in_degree(i), in_degrees[i]);
      EXPECT_EQ(g.out_degree(i), out_degrees[i]);
   }
}


TEST_F(GraphTest, UndirectedGraphDegreeTest) {
   Graph g = generate_fig5(false);
   vector<int> degrees = {3, 3, 2, 2, 1, 2, 3, 1, 1};
   for (int i = 0; i < g.num_nodes(); i++) {
      EXPECT_EQ(g.degree(i), degrees[i]);
   }
}


TEST_F(GraphTest, DirectedGraphTopoIterTest) {
   Graph g = generate_fig5(true);
   Graph g_dag;
   vector<vector<int>> edges = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.generate_dag(0).get_edges(), edges);
   edges = {{1, 4}, {1, 5}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.generate_dag(1).get_edges(), edges);
}


TEST_F(GraphTest, UndirectedGraphTopoIterTest) {
   Graph g = generate_fig5(false);
   Graph g_dag;
   vector<vector<int>> edges = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.generate_dag(0).get_edges(), testing::UnorderedElementsAreArray(edges));
   edges = {{1, 0}, {1, 4}, {1, 5}, {0, 2}, {0, 3}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.generate_dag(1).get_edges(), testing::UnorderedElementsAreArray(edges));
}

TEST_F(GraphTest, CandidateSetTest) {
   Graph queryG, dataG;
   queryG = generate_fig5(false);
   vector<set<int>> correct_cs = {
      {7, 8, 9}, // degree 3
      {7, 8, 9}, // degree 3
      {1, 2, 4, 6, 7, 8, 9}, // degree 2
      {1, 2, 4, 6, 7, 8, 9}, // degree 2
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, // degree 1
      {1, 2, 4, 6, 7, 8, 9}, // degree 2
      {7, 8, 9}, // degree 3
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, // degree 1
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9} // degree 1
   };

   string data_graph_path = "../../test/graph/10_1_0.txt";
   ifstream data_graph_file(data_graph_path);
   if (!data_graph_file.is_open() || !dataG.load_from_file(data_graph_file)) {
      cerr << "Cannot load graph file ../../test/graph/10_1_0.txt.\n";
      FAIL();
   }

   for (int i = 0; i < queryG.num_nodes(); i++) {
      EXPECT_THAT(queryG.get_candidate_set(i, dataG), testing::UnorderedElementsAreArray(correct_cs[i]));
   }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}