#!/usr/bin/python

__author__ = 'iashraf'

import sys
import os.path
import networkx as nx
import subprocess
from math import log

rootDir = os.path.dirname(os.path.realpath(__file__))


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


# Following two utility functions are used to convert size in bytes to human
# readable size and reverse. These are also available in hurry.filesize

## {{{ http://code.activestate.com/recipes/578019/ (r15)
"""
Bytes-to-human / human-to-bytes converter.
Based on: http://goo.gl/kTQMs
Working with Python 2.x and 3.x.

Author: Giampaolo Rodola' <g.rodola [AT] gmail [DOT] com>
License: MIT
"""
SYMBOLS = {
    'customary'     : ('B', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'),
    'customary_ext' : ('byte', 'kilo', 'mega', 'giga', 'tera', 'peta', 'exa',
                       'zetta', 'iotta'),
    'iec'           : ('Bi', 'Ki', 'Mi', 'Gi', 'Ti', 'Pi', 'Ei', 'Zi', 'Yi'),
    'iec_ext'       : ('byte', 'kibi', 'mebi', 'gibi', 'tebi', 'pebi', 'exbi',
                       'zebi', 'yobi'),
}

def bytes2human(n, format='%(value).1f %(symbol)s', symbols='customary'):
    """
    Convert n bytes into a human readable string based on format.
    symbols can be either "customary", "customary_ext", "iec" or "iec_ext",
    see: http://goo.gl/kTQMs

      >>> bytes2human(0)
      '0.0 B'
      >>> bytes2human(0.9)
      '0.0 B'
      >>> bytes2human(1)
      '1.0 B'
      >>> bytes2human(1.9)
      '1.0 B'
      >>> bytes2human(1024)
      '1.0 K'
      >>> bytes2human(1048576)
      '1.0 M'
      >>> bytes2human(1099511627776127398123789121)
      '909.5 Y'

      >>> bytes2human(9856, symbols="customary")
      '9.6 K'
      >>> bytes2human(9856, symbols="customary_ext")
      '9.6 kilo'
      >>> bytes2human(9856, symbols="iec")
      '9.6 Ki'
      >>> bytes2human(9856, symbols="iec_ext")
      '9.6 kibi'

      >>> bytes2human(10000, "%(value).1f %(symbol)s/sec")
      '9.8 K/sec'

      >>> # precision can be adjusted by playing with %f operator
      >>> bytes2human(10000, format="%(value).5f %(symbol)s")
      '9.76562 K'
    """
    n = int(n)
    if n < 0:
        raise ValueError("n < 0")
    symbols = SYMBOLS[symbols]
    prefix = {}
    for i, s in enumerate(symbols[1:]):
        prefix[s] = 1 << (i+1)*10
    for symbol in reversed(symbols[1:]):
        if n >= prefix[symbol]:
            value = float(n) / prefix[symbol]
            return format % locals()
    return format % dict(symbol=symbols[0], value=n)

def human2bytes(s):
    """
    Attempts to guess the string format based on default symbols
    set and return the corresponding bytes as an integer.
    When unable to recognize the format ValueError is raised.

      >>> human2bytes('0 B')
      0
      >>> human2bytes('1 K')
      1024
      >>> human2bytes('1 M')
      1048576
      >>> human2bytes('1 Gi')
      1073741824
      >>> human2bytes('1 tera')
      1099511627776

      >>> human2bytes('0.5kilo')
      512
      >>> human2bytes('0.1  byte')
      0
      >>> human2bytes('1 k')  # k is an alias for K
      1024
      >>> human2bytes('12 foo')
      Traceback (most recent call last):
          ...
      ValueError: can't interpret '12 foo'
    """
    init = s
    num = ""
    while s and s[0:1].isdigit() or s[0:1] == '.':
        num += s[0]
        s = s[1:]
    num = float(num)
    letter = s.strip()
    for name, sset in SYMBOLS.items():
        if letter in sset:
            break
    else:
        if letter == 'k':
            # treat 'k' as an alias for 'K' as per: http://goo.gl/kTQMs
            sset = SYMBOLS['customary']
            letter = letter.upper()
        else:
            raise ValueError("can't interpret %r" % init)
    prefix = {sset[0]:1}
    for i, s in enumerate(sset[1:]):
        prefix[s] = 1 << (i+1)*10
    return int(num * prefix[letter])



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
            countstr = '%0.1f' % countpercent
            calls = nxGraph.node[n]['calls']
            callsStrip = calls.strip()
            nstr = '\"{0}\" [ label = \" {1} \\n {2}%, {3}x \" {4}];'.format( n, name, countstr, callsStrip, fstyle )
            fout.write(nstr)

    for n in nxGraph.nodes():
        if( nxGraph.node[n]['type'] == 'object'):
            name = nxGraph.node[n]['name']
            size = nxGraph.node[n]['size']
            sizeHumanStr = bytes2human(size)
            nstr = "\"{0}\" [ label = \" {1} \\n {2} \" {3} ];\n".format( n, name, sizeHumanStr, ostyle )
            fout.write(nstr)

    maxcomm = calcMaxWeight(nxGraph)
    for (u,v,d) in nxGraph.edges(data='weight'):
        weight = log( float(d) ) / log( float(maxcomm) )
        color = int( 1023*weight )
        cstr3 =  format( max(0, color-768) , '02x')
        cstr2 =  format( min(255, 512-int(abs(color-512)) ), '02x' )
        cstr1 =  format( max(0, min(255,512-color)), '02x' )
        cstr = cstr3 + cstr2 + cstr1
        weightHumanStr = bytes2human(d)
        estr = " \"{0}\" -> \"{1}\" [ label = \" {2} \" color = \" #{3} \" ];\n".format( u, v, weightHumanStr, cstr )
        fout.write(estr)

    fout.write("}\n")
    fout.close()

def generatePdf(foutname):
    print('Generating Pdf ...')
    # cmd = "dot -Tpdf %s -O" %fout
    d2pScript = os.path.join(rootDir, "dot2pdf.sh" )
    cmd = '{0} {1}'.format(d2pScript, foutname)
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
    dotout = "communication_filtered_" + str(nthresh) +"_"+ str(ethresh) + ".dot"
    writeDot(g,dotout)
    generatePdf(dotout)
    # showDot(dotout)
