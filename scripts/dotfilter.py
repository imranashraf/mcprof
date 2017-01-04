#!/usr/bin/python

__author__ = 'iashraf'

import sys
import os.path
import networkx as nx
import subprocess
from math import log

try:
    print ("MCProf Graph filtering script")
    import pygraphviz as pg
    from networkx.drawing.nx_agraph import write_dot
    print("Using pygraphviz package")
except ImportError:
    try:
        import pydotplus
        from networkx.drawing.nx_pydot import write_dot
        print("using pydotplus package")
    except ImportError:
        print("Both pygraphviz and pydotplus were not found ")
        raise

def readDot(fin):
    dGraph = pg.AGraph(fin)
    # nxGraph = nx.Graph(dGraph)
    nxGraph = nx.DiGraph(dGraph)
    return nxGraph

def calcTotalCount(nxGraph):
    totalCount = 0
    for n in nxGraph.nodes():
        if( nxGraph.node[n]['type'] == 'function'):
            count = int( nxGraph.node[n]['count'] )
            totalCount += count
    return totalCount

def calcMaxWeight(nxGraph):
    maxComm = 0.0
    for (u,v,d) in nxGraph.edges(data='weight'):
        if d >= maxComm:
            maxComm = d;
    return maxComm

def calcTotalWeight(nxGraph):
    totalweight = 0.0
    for (u,v,d) in nxGraph.edges(data='weight'):
        totalweight += float(d);
    return totalweight

orignalTotalCount = 0.0
orignalTotalWeight = 0.0
def readGraph(fname):
    print('Reading input graph ...')
    nxGraph = nx.DiGraph()
    fin = open(fname)
    for line in fin:
        if not line.isspace() and not line.startswith('#'):
            # print(line)
            fields = line.split(';')
            if fields[0] == "f":
                # print("Adding function node")
                nxGraph.add_node(fields[1], type="function", name=fields[2], count=fields[3], calls=fields[4])
            elif fields[0] == "o":
                # print("Adding object node")
                nxGraph.add_node(fields[1], type="object", name=fields[2], size=fields[3])
            elif fields[0] == "e":
                # print("Adding edge")
                nxGraph.add_edge(fields[1], fields[2], weight=float(fields[3]) )
            else:
                print("Incorrect input file")
    fin.close()
    global orignalTotalCount
    global orignalTotalWeight
    orignalTotalCount = calcTotalCount(nxGraph)
    orignalTotalWeight = calcTotalWeight(nxGraph)
    return nxGraph

def writeDot(nxGraph, foutname):
    print('Writing graph in Dot file ... ')
    fout = open(foutname, 'w+')
    # write_dot(nxGraph,fout) # to write nx graph

    fout.write("digraph {\n")
    fout.write("graph [];\n")
    fout.write("edge [fontsize=20, arrowhead=vee, arrowsize=0.8, penwidth=3];\n")

    fstyle = "fontcolor=black, style=filled, fontsize=22";
    ostyle = "fontcolor=black, shape=box, fontsize=22"

    for n in nxGraph.nodes():
        if( nxGraph.node[n]['type'] == 'function'):
            name = nxGraph.node[n]['name']
            count = nxGraph.node[n]['count']
            countpercent = float(count) / orignalTotalCount * 100
            countstr = "%0.1f" % countpercent
            calls = nxGraph.node[n]['calls']
            nstr = "\"{0}\" [ label = \" {1} \\n {2} , {3} \" {4} ];\n".format( n, name, countstr, calls, fstyle )
            fout.write(nstr)

    for n in nxGraph.nodes():
        if( nxGraph.node[n]['type'] == 'object'):
            name = nxGraph.node[n]['name']
            size = nxGraph.node[n]['size']
            nstr = "\"{0}\" [ label = \" {1} \\n {2} \" {3} ];\n".format( n, name, size, ostyle )
            fout.write(nstr)

    maxcomm = calcMaxWeight(nxGraph)
    for (u,v,d) in nxGraph.edges(data='weight'):
        weight = log( float(d) ) / log( float(maxcomm) )
        color = int( 1023*weight )
        cstr3 =  format( max(0, color-768) , '02x')
        cstr2 =  format( min(255, 512-int(abs(color-512)) ), '02x' )
        cstr1 =  format( max(0, min(255,512-color)), '02x' )
        cstr = cstr3 + cstr2 + cstr1
        estr = " \"{0}\" -> \"{1}\" [ label = \" {2} \" color = \" #{3} \" ];\n".format( u, v, d, cstr )
        fout.write(estr)

    fout.write("}\n")
    fout.close()

def generatePdf(foutname):
    print('Generating Pdf ...')
    # cmd = "dot -Tpdf %s -O" %fout
    cmd = "/data/repositories/mcprof/scripts/dot2pdf.sh %s" %foutname
    ret = subprocess.check_output( cmd, shell=True)
    print("Written %s output dot file." % str(foutname))

def filterNodes(nxGraph, threshpercent):
    print('Filtering nodes ... ')
    thresh = threshpercent * orignalTotalCount / 100
    print('Removing nodes with instruction count less than %s' % thresh)
    for n in nxGraph.nodes():
        if( nxGraph.node[n]['type'] == 'function' and float( nxGraph.node[n]['count']) < thresh):
            # print("filtering node")
            nxGraph.remove_node(n)

def filterEdges(nxGraph, threshpercent):
    print('Filtering edges ...')
    thresh = threshpercent/100 * orignalTotalWeight
    print('Removing edges with communication less than %s' % thresh)
    for (u,v,d) in nxGraph.edges(data='weight'):
        if float(d) < thresh:
            # print("filtering edge")
            nxGraph.remove_edge(u,v)

xdotPIDs = {}
def showDot(dotout):
    print('Showing Dot in Xdot ...')
    global xdotPIDs
    try:
        status = "OPEN"
        pid = xdotPIDs[dotout]
    except KeyError:
        status = "CLOSE"

    if status == "CLOSE" or not os.path.exists( '/proc/{}'.format(pid) ):
        cmd = "xdot %s &" %dotout
        # ret = subprocess.check_output( cmd, shell=True)
        proc = subprocess.Popen(["nohup", "xdot", dotout ])
        xdotPIDs[dotout] = proc.pid
        print('Opened {} dot file in xdot with pid {}'.format( str(dotout), str(proc.pid) ) )
    else:
        print("%s dot file already open in xdot." % str(dotout))

if __name__ == "__main__":
    total = len(sys.argv)
    cmdargs = str(sys.argv)
    nthresh = 0.0
    ethresh = 0.0
    try:
        # Pharsing args one by one
        scriptName = sys.argv[0]
        scriptpath =  os.path.realpath(__file__)
        fin = sys.argv[1]
        nthresh = float( sys.argv[2] )
        ethresh = float( sys.argv[3] )
    except IndexError:
        print("Input file name not specified as input")
        print('  Usage: {} <mcprof graph file.dat> <%age node thresh> <%age edge thresh>'.format(scriptpath) )
        print('  For example: {} testgraphs/test07.dat 0.01 0.0001'.format(scriptpath) )
        sys.exit(0)

    print('Processing graph in {0} file with node threshold {1} and edge threshold {2}'.format(str(fin), nthresh, ethresh) )

    g = readGraph(fin)
    filterNodes(g, nthresh)
    filterEdges(g, ethresh)
    dotout = "output.dot"
    writeDot(g,dotout)
    generatePdf(dotout)
    # showDot(dotout)
