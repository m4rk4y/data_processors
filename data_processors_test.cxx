// Needs some algorithmic explanation.
//
// Problem: traverse a DAG.
//
// Start at START and proceed to END.
//
// Each node can have any or all of 4 outward edges,
// labelled "0", "1", "2", "3".
//
// Traversal proceeds by talking to a server: the client asks "0", "1", "2" or
// "3" and the server responds with the name of the node at the far end of
// that edge, or some kind of error response beginning with "ERROR : ". Once
// you reach the "END" node, your only option is to say "RESET" which takes
// you back to the "START" node.
//
// The solution needs to explore the entire DAG and then produce a
// GraphViz-style description.
//
// This solution has no proper client/server code. (The original problem
// called for some TCP/IP code to talk to a real server, and that's quite
// fiddly and time-consuming, not to mention algorithmically uninteresting for
// the purpose of this exercise. I would also have to build and run a server
// too.) Instead there is a hard-coded representation of an example DAG,
// invoked by buildServerTree(), and a fake server, invoked by getResponse().
// Obviously I should invent a DAG syntax (or in fact use GraphViz?) and some
// code to read it.
//
// Post-process the output with e.g.
// dot -Tjpg out.txt > out.jpg

#include <time.h>

#include <iostream>
#include <map>
#include <set>
#include <string>

using namespace std;

class Node;

class Edge
{
    public:
        string m_label;
        Node * m_to;
};

class Node
{
    public:
        Node ( const char * name )
          : m_name ( name ), m_completelyExplored ( false )
        {
            for ( int inx = 0; inx < 4; ++inx )
            {
                m_edges[inx] = 0;
            }
        }
        bool isCompletelyExplored()
        {
            clog << "Checking " << m_name << " for isCompletelyExplored" << endl;
            if ( m_completelyExplored ) // already determined
            {
                    clog << "    already determined as true" << endl;
                    return true;
            }
            for ( int inx = 0; inx < 4; ++inx )
            {
                Edge * edge = m_edges[inx];
                if ( edge == 0 )
                {
                    clog << "    edge " << inx << " == 0, hence false for " << m_name << endl;
                    return false;
                }
                Node * to = edge->m_to;
                // to == 0 means we tried this edge but it led nowhere.
                if ( to != 0 && ! to->isCompletelyExplored() )
                {
                    clog << "    to " << inx << " (" << to->m_name << ") != 0 and not isCompletelyExplored, hence false for " << m_name << endl;
                    return false;
                }
            }
            m_completelyExplored = true;    // for later optimisation
            clog << "    " << m_name << " is-completely-explored" << endl;
            return true;
        }
        string m_name;
        Edge* m_edges[4];
        bool m_completelyExplored;
};

static Node startNode ( "START" );
static Node endNode ( "END" );
static char* nodeRequests[] = { "0", "1", "2", "3" };
static map<string,Node*> visitedNodes;

static Node startServerNode ( "START" );
static Node * currentServerNode = &startServerNode;
static void buildServerTree()
{
    // Construct:
    //             /-3- g -2-\ /--3-- f -1-\
    //  START -0- a -0- c -2- e -----2----- END
    //        \2- b ----0----/             /
    //            \_______1_______________/

    Node * a = new Node ( "a" );
    Node * b = new Node ( "b" );
    Node * c = new Node ( "c" );
    Node * d = new Node ( "d" );
    Node * e = new Node ( "e" );
    Node * f = new Node ( "f" );
    Node * g = new Node ( "g" );
    Node * h = new Node ( "h" );
    clog << "startServerNode is " << &startServerNode << endl;
    clog << "Start node is " << &startNode << endl;
    clog << "End node is " << &endNode << endl;
    clog << "Created server node " << a << " with name " << a->m_name << endl;
    clog << "Created server node " << b << " with name " << b->m_name << endl;
    clog << "Created server node " << c << " with name " << c->m_name << endl;
    clog << "Created server node " << d << " with name " << d->m_name << endl;
    clog << "Created server node " << e << " with name " << e->m_name << endl;
    clog << "Created server node " << f << " with name " << f->m_name << endl;
    clog << "Created server node " << g << " with name " << g->m_name << endl;
    clog << "Created server node " << h << " with name " << h->m_name << endl;

    Edge * s_to_a = new Edge;
    s_to_a->m_to = a;
    startServerNode.m_edges[0] = s_to_a;

    Edge * s_to_b = new Edge;
    s_to_b->m_to = b;
    startServerNode.m_edges[2] = s_to_b;

    Edge * a_to_c = new Edge;
    a_to_c->m_to = c;
    a->m_edges[0] = a_to_c;

    Edge * c_to_e = new Edge;
    c_to_e->m_to = e;
    c->m_edges[2] = c_to_e;

    Edge * b_to_e = new Edge;
    b_to_e->m_to = e;
    b->m_edges[0] = b_to_e;

    Edge * e_to_end = new Edge;
    e_to_end->m_to = &endNode;
    e->m_edges[2] = e_to_end;

    Edge * b_to_end = new Edge;
    b_to_end->m_to = &endNode;
    b->m_edges[1] = b_to_end;

    Edge * e_to_f = new Edge;
    e_to_f->m_to = f;
    e->m_edges[3] = e_to_f;

    Edge * f_to_end = new Edge;
    f_to_end->m_to = &endNode;
    f->m_edges[1] = f_to_end;

    Edge * a_to_g = new Edge;
    a_to_g->m_to = g;
    a->m_edges[3] = a_to_g;

    Edge * g_to_e = new Edge;
    g_to_e->m_to = e;
    g->m_edges[2] = g_to_e;
}

static string getResponse ( const string & request )
{
    clog << "getResponse: " << request << endl;
    if ( request == "RESET" )
    {
        currentServerNode = &startServerNode;
        clog << "    : currentServerNode node is now " << currentServerNode << " " << currentServerNode->m_name << endl;
        return currentServerNode->m_name;
    }
    else if ( currentServerNode == 0 )
    {
        return "ERROR : no current node";
    }
    if ( request == "0" || request == "1" || request == "2" || request == "3" )
    {
        clog << "    : currentServerNode node is " << currentServerNode << " " << currentServerNode->m_name << endl;
        clog << "    : look for edge " << atoi(request.c_str()) << endl;
        Edge * edge = currentServerNode->m_edges[atoi(request.c_str())];
        if ( edge == 0 )
        {
            return "ERROR : no edge here";
        }
        currentServerNode = edge->m_to;
        clog << "    : currentServerNode node is now " << currentServerNode;
        if ( currentServerNode != 0 )
        {
            clog << " " << currentServerNode->m_name;
        }
        clog << endl;
        if ( currentServerNode == 0 )
        {
            return "ERROR : no node here";
        }
        return currentServerNode->m_name;
    }
    return "ERROR : invalid request";
}

// Returns false for fully-explored, otherwise true (i.e. "keep exploring").
static bool explore ( Node * node )
{
    clog << "Explore: " << node->m_name << endl;
    if ( node->isCompletelyExplored() )
    {
        clog << node->m_name << " is completely explored" << endl;
        return false;
    }
    for ( int inx = 0; inx < 4; ++inx )
    {
        clog << node->m_name << " inx: " << inx << endl;
        if ( node->m_edges[inx] == 0 )
        {
            Edge * edge = new Edge;
            node->m_edges[inx] = edge;
            edge->m_label = nodeRequests[inx];
            string toName = getResponse ( nodeRequests[inx] );
            clog << "response was " << toName << endl;
            if ( toName == "END" )
            {
                clog << "Set " << node->m_name << " edge " << inx << " to point to END" << endl;
                edge->m_to = &endNode;
                clog << node->m_name << " found END" << endl;
                return true;
            }
            else if ( toName.substr(0,8) == "ERROR : " )
            {
                // Presume missing edge. Add one so we know we tried, but leave it pointing nowhere.
                clog << "Set " << node->m_name << " edge " << inx << " to point nowhere" << endl;
                edge->m_to = 0;
                continue;
            }
            else
            {
                map<string,Node*>::iterator iter = visitedNodes.find ( toName );
                Node * toNode;
                if ( iter != visitedNodes.end() )
                {
                    toNode = iter->second;
                    clog << "Found visited node " << toNode << " with name " << toNode->m_name << endl;
                }
                else
                {
                    toNode = new Node ( toName.c_str() );
                    visitedNodes.insert ( pair<string,Node*> ( toNode->m_name, toNode ) );
                    clog << "Created node " << toNode << " with name " << toNode->m_name << endl;
                }
                clog << "Set " << node->m_name << " edge " << inx << " to point to " << toNode->m_name << endl;
                edge->m_to = toNode;
                if ( ! toNode->isCompletelyExplored() )
                {
                    bool subResult = explore ( toNode );
                    if ( subResult )
                    {
                        clog << "subResult on " << toNode->m_name << " was true" << endl;
                        clog << "hence " << node->m_name << " returning true" << endl;
                        return true;
                    }
                }
                else    // force a reset
                {
                    clog << "Forcing a reset" << endl;
                    return true;
                }
            }
        }
        else
        {
            Edge * edge = node->m_edges[inx];
            if ( edge->m_to != &endNode && edge->m_to != 0 &&
                 ! edge->m_to->isCompletelyExplored()
               )
            {
                // We know where we're going but we still have to make the request
                // in order to move the server on.
                string toName = getResponse ( nodeRequests[inx] );
                clog << "response was " << toName << endl;
                Node * toNode = node->m_edges[inx]->m_to;
                if ( toNode != 0 && toNode != &endNode )
                {
                    bool subResult = explore ( toNode );
                    if ( subResult )
                    {
                        clog << "subResult on " << toNode->m_name << " was true" << endl;
                        clog << "hence " << node->m_name << " returning true" << endl;
                        return true;
                    }
                }
            }
        }
    }
    clog << node->m_name << " returning true" << endl;
    return true;
}

static set<Node*> drawnNodes;

static void draw ( FILE * file, Node * node )
{
    if ( drawnNodes.find ( node ) == drawnNodes.end() )
    {
        for ( int inx = 0; inx < 4; ++inx )
        {
            if ( node->m_edges[inx] != 0 && node->m_edges[inx]->m_to != 0 )
            {
                fprintf ( file, "%s -> %s [ label = \"%s\" ];\n",
                          node->m_name.c_str(),
                          node->m_edges[inx]->m_to->m_name.c_str(),
                          node->m_edges[inx]->m_label.c_str() );
                draw ( file, node->m_edges[inx]->m_to );
            }
        }
        drawnNodes.insert ( node );
    }
}

static bool drawGraph ( const char * fileName )
{
    FILE * file = fopen ( fileName, "w" );
    if ( file == 0 )
    {
        cerr << "Failed to open " << fileName << " for writing" << endl;
        return false;
    }
    fprintf ( file, "digraph G {\n" );
    draw ( file, &startNode );
    fprintf ( file, "}\n" );
    fclose ( file );
    return true;
}

extern int main ( int argc, char ** argv )
{
    if ( argc < 2 )
    {
        cerr << "Error: needs <filename> [ -v ]" << endl;
        return 1;
    }
    // If -v not seen then suppress logging.
    if ( argc < 3 || strcmp ( argv[2], "-v" ) != 0 )
    {
        clog.setstate ( ios_base::badbit );
    }

    // Build tree for testing.
    buildServerTree();

    // Avoid duplicating START and END nodes.
    visitedNodes.insert ( pair<string,Node*> ( startNode.m_name, &startNode ) );
    visitedNodes.insert ( pair<string,Node*> ( endNode.m_name, &endNode ) );

    // Set END node as completely explored.
    for ( int inx = 0; inx < 4; ++inx )
    {
        Edge * edge = new Edge;
        edge->m_label = nodeRequests[inx];  // not strictly needed
        edge->m_to = 0;
        endNode.m_edges[inx] = edge;
    }

    while ( explore ( &startNode ) )
    {
        string dummy = getResponse ( "RESET" );
        clog << "response was " << dummy << endl;
    }

    return drawGraph ( argv[1] ) ? 0 : 1;
}