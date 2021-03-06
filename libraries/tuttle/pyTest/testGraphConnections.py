# scons: Checkerboard LensDistort Merge

from pyTuttle import tuttle
import os

def setUp():
	tuttle.core().preload(False)

def testGraphConnections():
	graph = tuttle.Graph()

	print "graph:", graph

	checkerboard = graph.createNode( "tuttle.checkerboard" ).asImageEffectNode()
	lensdistort = graph.createNode( "tuttle.lensdistort" ).asImageEffectNode()
	
	clipOut = checkerboard.getClip("Output")
	clipIn = lensdistort.getClip("Source")

	graph.connect( clipOut, clipIn )
	print "graph:", graph

	graph.unconnect( clipOut, clipIn )
	print "graph:", graph


def testGraphUnconnect():
	"""
	Multiple connections

	"""
	graph = tuttle.Graph()

	print "graph:", graph

	checkerboard = graph.createNode( "tuttle.checkerboard" ).asImageEffectNode()

	merge = graph.createNode( "tuttle.merge" ).asImageEffectNode()
	mergeClipA = merge.getClip("A")
	mergeClipB = merge.getClip("B")
	
	lensdistort = graph.createNode( "tuttle.lensdistort" ).asImageEffectNode()

	assert graph.getNbOutputConnections(checkerboard) == 0
	graph.connect( checkerboard.getClip("Output"), mergeClipA )
	assert graph.getNbOutputConnections(checkerboard) == 1
	graph.unconnect( checkerboard.getClip("Output"), mergeClipA )
	print "graph:", graph
	assert graph.getNbOutputConnections(checkerboard) == 0


def testGraphMultipleConnections():
	"""
	Multiple connections

	"""
	graph = tuttle.Graph()

	print "graph:", graph

	checkerboard = graph.createNode( "tuttle.checkerboard" ).asImageEffectNode()

	merge = graph.createNode( "tuttle.merge" ).asImageEffectNode()
	mergeClipA = merge.getClip("A")
	mergeClipB = merge.getClip("B")
	
	assert graph.getNbOutputConnections(checkerboard) == 0
	graph.connect( checkerboard.getClip("Output"), mergeClipA )
	assert graph.getNbOutputConnections(checkerboard) == 1
	graph.connect( checkerboard.getClip("Output"), mergeClipA )	
	assert graph.getNbOutputConnections(checkerboard) == 1
	print "graph:", graph
	
	graph.connect( checkerboard.getClip("Output"), mergeClipB )
	
	print "graph:", graph
	#TODO: Should be able to have 2 connections...
	#assert graph.getNbOutputConnections(checkerboard) == 2

