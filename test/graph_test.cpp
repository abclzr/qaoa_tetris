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
   vector<string> testGraphNames = {"line2", "line3", "triangle", "square", "clique4"};
   vector<Graph> correctGraphs = {
      generate_line2(true), 
      generate_line3(true),
      generate_triangle(false),
      generate_square(false),
      generate_clique(4, true)
      };
   vector<bool> graphDirected = {
      true, true, false, false, true
   };

   for (int i = 0; i < testGraphNames.size(); i++) {
      Graph g;
      // XXX: modify the current testing/working directory in cmakefile
      string testGraphPath = "../../test/graph/" + testGraphNames[i] + ".txt";
      ifstream graphFile(testGraphPath);
      if (!graphFile.is_open() || !g.load_from_file(graphFile, graphDirected[i])) {
         cerr << "Cannot load graph file " << testGraphPath << ".\n";
         FAIL();
      } else {
         // TODO: We also need to check if edges are matched. Further, if nodes have labels, we need to check labels.
         // TODO: We need a function here to check if two bst graphs are the same. (both directed and undirected)
         EXPECT_EQ(g.num_nodes(), correctGraphs[i].num_nodes());
         EXPECT_EQ(g.num_edges(), correctGraphs[i].num_edges());

         graphFile.close();
      }

   }
}

TEST_F(GraphTest, DirectedGraphDegreeTest) {
   Graph g = generate_fig5(true);
   vector<int> inDegrees = {0, 1, 1, 1, 1, 1, 2, 1, 1};
   vector<int> outDegrees = {3, 2, 1, 1, 0, 1, 1, 0, 0};
   for (int i = 0; i < g.num_nodes(); i++) {
      EXPECT_EQ(g.in_degree(i), inDegrees[i]);
      EXPECT_EQ(g.out_degree(i), outDegrees[i]);
   }
}


TEST_F(GraphTest, UndirectedGraphDegreeTest) {
   Graph g = generate_fig5(false);
   vector<int> degrees = {3, 3, 2, 2, 1, 2, 3, 1, 1};
   for (int i = 0; i < g.num_nodes(); i++) {
      EXPECT_EQ(g.degree(i), degrees[i]);
   }
}


TEST_F(GraphTest, RemoveNodesTest) {
   Graph g = generate_fig5(true);
   set<int> nodes = {0};
   g.remove_nodes(nodes);
   vector<vector<int>> edges = {{1, 4}, {1, 5}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.get_edges(), testing::UnorderedElementsAreArray(edges));

   g = generate_fig5(true);
   nodes = {1, 2, 3};
   g.remove_nodes(nodes);
   edges = {{5, 6}, {6, 8}};
   EXPECT_THAT(g.get_edges(), testing::UnorderedElementsAreArray(edges));
}


TEST_F(GraphTest, DirectedGraphDAGTest) {
   Graph g = generate_fig5(true);
   Graph g_dag;
   vector<vector<int>> edges = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.generate_dag(0).get_edges(), edges);
   edges = {{1, 4}, {1, 5}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.generate_dag(1).get_edges(), edges);
}


TEST_F(GraphTest, UndirectedGraphDAGTest) {
   Graph g = generate_fig5(false);
   Graph g_dag;
   vector<vector<int>> edges = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.generate_dag(0).get_edges(), testing::UnorderedElementsAreArray(edges));
   edges = {{1, 0}, {1, 4}, {1, 5}, {0, 2}, {0, 3}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.generate_dag(1).get_edges(), testing::UnorderedElementsAreArray(edges));
}


TEST_F(GraphTest, ReverseGraphTest) {
   Graph g_dag = generate_fig5(false).generate_dag(0);
   Graph reverse_g_dag = g_dag.generate_reversed_graph();
   vector<vector<int>> edges = {{1, 0}, {2, 0}, {3, 0}, {4, 1}, {5, 1}, {6, 2}, {7, 3}, {6, 5}, {8, 6}};
   EXPECT_THAT(reverse_g_dag.get_edges(), testing::UnorderedElementsAreArray(edges));
}


TEST_F(GraphTest, CandidateSetTest) {
   Graph queryG, dataG;
   queryG = generate_fig5(false);
   vector<set<int>> correctCS = {
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

   string dataGraphPath = "../../test/graph/10_1_0.txt";
   ifstream dataGraphFile(dataGraphPath);
   if (!dataGraphFile.is_open() || !dataG.load_from_file(dataGraphFile)) {
      cerr << "Cannot load graph file ../../test/graph/10_1_0.txt.\n";
      FAIL();
   }

   for (int i = 0; i < queryG.num_nodes(); i++) {
      EXPECT_THAT(queryG.get_candidate_set(i, dataG), testing::UnorderedElementsAreArray(correctCS[i]));
   }
}


TEST_F(GraphTest, TopoIterTest) {
   Graph g = generate_fig5(false).generate_dag(0);
   // FIXME Add more tests...
   vector<int> topoOrder = {0, 3, 7, 2, 1, 5, 6, 8, 4};
   EXPECT_EQ(g.get_topo_order(), topoOrder);
}


TEST_F(GraphTest, ReversedTopoIterTest) {
   Graph g = generate_fig5(false).generate_dag(0);
   // FIXME Add more tests...
   vector<int> revTopoOrder = {4, 8, 6, 5, 1, 2, 7, 3, 0};
   EXPECT_EQ(g.get_reversed_topo_order(), revTopoOrder);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}