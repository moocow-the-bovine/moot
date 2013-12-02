#!/usr/bin/python
# -*- coding: utf-8 -*-

import unicodedata,re,string,sys,argparse,urllib2

script_data = {}
any_char = {}
any_char["Any"] = []

def gen_scripts ( handle ):
	idx = []
	names = []

	for ln in handle:
		p = re.findall(r'([0-9A-F]+)(?:\.\.([0-9A-F]+))?\W+(\w+)\s*#\s*(\w+)', ln)
		if p:
			a, b, name, cat = p[0]
			if name not in names:
				names.append(name)
			idx.append((int(a, 16), int(b or a, 16), names.index(name)))
	idx.sort()
	return {"names":names,"idx":idx}

def script(chr):
	global script_data
	l = 0
	r = len(script_data['idx']) - 1
	c = ord(chr)
	while r >= l:
		m = (l + r) >> 1
		if c < script_data['idx'][m][0]:
			r = m - 1
		elif c > script_data['idx'][m][1]:
			l = m + 1
		else:
			return script_data['names'][script_data['idx'][m][2]]
	return 'Unknown'

def convert_hex ( x ):
	return "\\x%0.2x" % ord(x)

def list2string ( data ):
	last = "\000"
	result = ""
	for x in data:
		if ord ( x ) != ord ( last ) + 1:
			if last == "\000":
				result += convert_hex ( x )
			else:
				if result[-1] == "-":
					result += convert_hex ( last )
				result += convert_hex ( x )
		else:
			if result[-1] != "-":
				result += "-"
		last = x

	if result[-1] == "-":
		result += convert_hex ( last )
	return result

def compact_range ( data, nbyte ):
	if nbyte == 1:
		return "[" + list2string ( data ) + "]"
	else:
		result = ""
		if nbyte == 2:
			for i in data:
				result += "([%s][%s])|" % (convert_hex ( i ), list2string ( data[i] ))
		elif nbyte == 3:
			for i in data:
				for j in data[i]:
					result += "([%s][%s][%s])|" % (convert_hex ( i ), convert_hex ( j ), list2string ( data[i][j] ))
					
			
		return result.rstrip ( "|" )
	

def compact_insert ( mapping, seq ):
	if len ( seq ) == 1:
		mapping[1].append ( seq )
	elif len ( seq ) == 2:
		if not mapping[2].has_key ( seq[0] ):
			mapping[2][seq[0]] = []
		mapping[2][seq[0]].append ( seq[1] )
	elif len ( seq ) == 3:
		if not mapping[3].has_key ( seq[0] ):
			mapping[3][seq[0]] = {}
		if not mapping[3][seq[0]].has_key ( seq[1] ):
			mapping[3][seq[0]][seq[1]] = []
		mapping[3][seq[0]][seq[1]].append ( seq[2] )

def print_re2c ( data, dest ):

	for dat in data:

		DATA = {}
		DATA[1] = []
		DATA[2] = {}
		DATA[3] = {}
		DATA[4] = {}

		for c in data[dat]:
			x = c.encode ( "utf-8" )
			compact_insert ( DATA, x )

		dat_out = "%s = " % dat
		for i in range ( 1, 5 ):
			if len ( DATA[i] ):
				cur_range = compact_range ( DATA[i], i )
				if cur_range:
					dest.write ( "%s%i = %s;\n" % (dat, i, cur_range) )
					dat_out += dat + str ( i ) + "|"

		dest.write ( "%s;\n" % dat_out.rstrip ( "|" ) )


def main ( argv ):
	#
	# global variables
	#
	global script_data

	#
	# command line argument handling
	#
	arg_parser = argparse.ArgumentParser ( 'Enhances re2c scanner defintions with a set of named definitions corresponding to unicode categories and scripts.' )
	
	# input file
	arg_parser.add_argument ( 'input', metavar='INPUT', type=argparse.FileType ( 'r' ), nargs='?', help='An input source (default: stdin)', default=sys.stdin )

	# output file
	arg_parser.add_argument ( '-o', '--output', type=argparse.FileType ( 'w' ), nargs='?', help='An output destination (default: stdout)', default=sys.stdout )

	# external scripts
	arg_parser.add_argument ( '-s', '--scripts', type=argparse.FileType ( 'r' ), nargs='?', help='File containing unicode script information (default: http://www.unicode.org/Public/UNIDATA/Scripts.txt).',dest='script_data' )

	args = arg_parser.parse_args()

	put = 1
	for line in args.input:
		if line.strip () == "%%UCL%%":

			#
			# parse scripts: python::unicodedata does not provide script information
			#
			if args.script_data:
				handle = args.script_data
			else:
				handle = urllib2.urlopen('http://www.unicode.org/Public/UNIDATA/Scripts.txt')
			script_data = gen_scripts ( handle )

			cats = {}
			scripts = {}
			intersects = {}

			i = 2
			while (i < 4294967296): #65536
				c = unichr ( i )
				cat = unicodedata.category ( c )
				scr = script ( c )
				if not cats.has_key ( cat ):
					cats[cat] = []
				if not scripts.has_key ( scr ):
					scripts[scr] = []
				cats[cat].append ( c )
				scripts[scr].append ( c )
				any_char["Any"].append ( c )
				i += 1
			
			# manually generate important intersects
			intersects["Latin_Ll"] = list ( set ( cats["Ll"] ) & set ( scripts["Latin"] ) )
			intersects["Latin_Ll"].sort ()
			intersects["Latin_Lu"] = list ( set ( cats["Lu"] ) & set ( scripts["Latin"] ) )
			intersects["Latin_Lu"].sort ()
				
			print_re2c ( cats, args.output )
			print_re2c ( scripts, args.output )
			print_re2c ( intersects, args.output )
			print_re2c ( any_char, args.output )
		else:
			args.output.write ( line )

if __name__ == "__main__":
    main ( sys.argv[1:] )
