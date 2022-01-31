#include "Graph.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using namespace qaoagraph;

// The fixture for testing class Foo.
class GraphTest : public ::testing::Test {
   protected:
   // You can remove any or all of the following functions if their bodies would
   // be empty.
   string resource_dir = TEST_RESOURCE_DIR;
   

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
      string testGraphPath = resource_dir + testGraphNames[i] + ".txt";
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
   unordered_set<int> nodes = {0};
   g.remove_nodes(nodes);
   vector<vector<int>> edges = {{1, 4}, {1, 5}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   EXPECT_THAT(g.get_edges(), testing::UnorderedElementsAreArray(edges));
   vector<int> inDegrees = {0, 0, 0, 0, 1, 1, 2, 1, 1};
   vector<int> outDegrees = {0, 2, 1, 1, 0, 1, 1, 0, 0};
   for (int node = 0; node < g.num_nodes(); node++) {
      EXPECT_EQ(g.in_degree(node), inDegrees[node]);
      EXPECT_EQ(g.out_degree(node), outDegrees[node]);
   }

   g = generate_fig5(true);
   nodes = {1, 2, 3};
   g.remove_nodes(nodes);
   edges = {{5, 6}, {6, 8}};
   EXPECT_THAT(g.get_edges(), testing::UnorderedElementsAreArray(edges));
}

// FIXME: Check no cycle.
// We need to check 1) # of edges 2) each edge matches original edge 3) no cycle
TEST_F(GraphTest, UndirectedGraphDAGTest) {
   Graph g = generate_fig5(false);
   Graph g_dag = g.generate_dag(0);
   EXPECT_EQ(g_dag.num_edges(), g.num_edges());
   for (auto edge : g_dag.get_edges()) {
      EXPECT_EQ(g.has_edge(edge[0], edge[1]), true);
   }
   g_dag = g.generate_dag(1);
   EXPECT_THAT(g_dag.num_edges(), g.num_edges());
   for (auto edge : g_dag.get_edges()) {
      EXPECT_EQ(g.has_edge(edge[0], edge[1]), true);
   }
   // vector<vector<int>> edges = {{0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 5}, {2, 6}, {3, 7}, {6, 5}, {6, 8}};
   // EXPECT_THAT(g.generate_dag(0).get_edges(), testing::UnorderedElementsAreArray(edges));
   // edges = {{1, 0}, {1, 4}, {1, 5}, {0, 2}, {0, 3}, {2, 6}, {3, 7}, {5, 6}, {6, 8}};
   // EXPECT_THAT(g.generate_dag(1).get_edges(), testing::UnorderedElementsAreArray(edges));
}


TEST_F(GraphTest, ReverseGraphTest) {
   Graph g_dag = generate_fig5(false).generate_dag(0);
   Graph reverse_g_dag = g_dag.generate_reversed_graph();
   vector<vector<int>> edges = {{1, 0}, {2, 0}, {3, 0}, {4, 1}, {5, 1}, {6, 2}, {7, 3}, {5, 6}, {8, 6}};
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

   string dataGraphPath = resource_dir + "10_1_0.txt";
   ifstream dataGraphFile(dataGraphPath);
   if (!dataGraphFile.is_open() || !dataG.load_from_file(dataGraphFile)) {
      cerr << "Cannot load graph file 10_1_0.txt.\n";
      FAIL();
   }
   dataGraphFile.close();
   for (int i = 0; i < queryG.num_nodes(); i++) {
      EXPECT_THAT(queryG.get_candidate_set(i, dataG), testing::UnorderedElementsAreArray(correctCS[i]));
   }
}


TEST_F(GraphTest, TopoIterTest) {
   Graph g = generate_fig5(false).generate_dag(0);
   // FIXME Add more tests...
   vector<int> topoOrder = {0, 3, 7, 2, 6, 8, 1, 5, 4};
   EXPECT_EQ(g.get_topo_order(), topoOrder);
}


TEST_F(GraphTest, ReversedTopoIterTest) {
   Graph g = generate_fig5(false).generate_dag(0);
   // FIXME Add more tests...
   vector<int> revTopoOrder = {4, 5, 1, 8, 6, 2, 7, 3, 0};
   EXPECT_EQ(g.get_reversed_topo_order(), revTopoOrder);
}


TEST_F(GraphTest, SCCTest) {
   Graph g(true);
   g.add_edge(0, 1);
   g.add_edge(1, 0);
   g.add_edge(0, 2);
   g.add_edge(1, 3);
   g.add_edge(2, 3);
   g.add_edge(3, 4);
   g.add_edge(4, 2);
   g.add_edge(3, 5);

   auto sccs = g.strongly_connected_components();
   vector<unordered_set<int>> correct_sccs = {{0, 1}, {2, 3, 4}, {5}};
   EXPECT_THAT(sccs, testing::UnorderedElementsAreArray(correct_sccs));

   Graph testG;
   string testGraphPath = resource_dir + "small4.txt";
   ifstream graphFile(testGraphPath);
   if (!graphFile.is_open() || !testG.load_from_file(graphFile, true)) {
      cerr << "Cannot load graph file " << testGraphPath << ".\n";
      FAIL();
   }
   graphFile.close();
   sccs = testG.strongly_connected_components();
   correct_sccs = {{0, 1, 2, 3}};
   EXPECT_THAT(sccs, testing::UnorderedElementsAreArray(correct_sccs));
}


TEST_F(GraphTest, SubgraphTest) {
   Graph testG;
   string testGraphPath = resource_dir + "small4.txt";
   ifstream graphFile(testGraphPath);
   if (!graphFile.is_open() || !testG.load_from_file(graphFile, true)) {
      cerr << "Cannot load graph file " << testGraphPath << ".\n";
      FAIL();
   }
   graphFile.close();
   Graph subgraph;
   subgraph = testG.generate_subgraph({0});
   EXPECT_EQ(subgraph.num_nodes(), 4);
   EXPECT_EQ(subgraph.num_edges(), 0);
   subgraph = testG.generate_subgraph({1, 2});
   EXPECT_EQ(subgraph.num_nodes(), 4);
   EXPECT_EQ(subgraph.num_edges(), 1);
   subgraph = testG.generate_subgraph({0, 3});
   EXPECT_EQ(subgraph.num_nodes(), 4);
   EXPECT_EQ(subgraph.num_edges(), 2);
}

TEST_F(GraphTest, SimpleCycleTest) {
   Graph testG1;
   string testGraphPath = resource_dir + "small4.txt";
   ifstream graphFile(testGraphPath);
   if (!graphFile.is_open() || !testG1.load_from_file(graphFile, true)) {
      cerr << "Cannot load graph file " << testGraphPath << ".\n";
      FAIL();
   }
   graphFile.close();
   auto cycles = testG1.simple_cycles();
   vector<vector<int>> correct_cycles = {{0, 3}, {0, 1, 2, 3}};
   EXPECT_THAT(cycles, testing::UnorderedElementsAreArray(correct_cycles));  

   Graph testG2(3, true);
   testG2.add_edge(0, 1);
   testG2.add_edge(1, 0);
   testG2.add_edge(1, 2);
   testG2.add_edge(2, 1);
   testG2.add_edge(2, 0);
   cycles = testG2.simple_cycles();
   correct_cycles = {{0, 1}, {1, 2}, {0, 1, 2}};
   EXPECT_THAT(cycles, testing::UnorderedElementsAreArray(correct_cycles));  

   Graph testG3(6, true);
   testG3.add_edge(0, 1);
   testG3.add_edge(1, 0);
   testG3.add_edge(0, 2);
   testG3.add_edge(1, 3);
   testG3.add_edge(2, 3);
   testG3.add_edge(3, 4);
   testG3.add_edge(4, 2);
   testG3.add_edge(3, 5);
   cycles = testG3.simple_cycles();
   correct_cycles = {{0, 1}, {2, 3, 4}};
   EXPECT_THAT(cycles, testing::UnorderedElementsAreArray(correct_cycles));  
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}