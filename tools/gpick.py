#!/usr/bin/env python

import os
import time
import SCons
import re
import string
import sys
import glob
import subprocess

from lemon import *
from flex import *

from SCons.Script import *
from SCons.Util import *
from SCons.Script.SConscript import SConsEnvironment

import SCons.Script.SConscript
import SCons.SConf
import SCons.Conftest

def MatchFiles (files, path, repath, dir_exclude_pattern,  file_exclude_pattern):

	for filename in os.listdir(path):
		fullname = os.path.join (path, filename)
		repath_file =  os.path.join (repath, filename);
		if os.path.isdir (fullname):
			if not dir_exclude_pattern.search(repath_file):
				MatchFiles (files, fullname, repath_file, dir_exclude_pattern, file_exclude_pattern)
		else:
			if not file_exclude_pattern.search(filename):
				files.append (fullname)

def CheckPKG(context, name):
	context.Message( 'Checking for %s... ' % name )
	ret = context.TryAction('pkg-config --exists "%s"' % name)[0]
	context.Result( ret )
	return ret

class GpickLibrary(NodeList):
	include_dirs = []

class GpickEnvironment(SConsEnvironment):
	
	extern_libs = {}
	
	def AddCustomBuilders(self):
		addLemonBuilder(self)
		addFlexBuilder(self)
		
	def DefineLibrary(self, library_name, library):
		self.extern_libs[library_name] = library
		
	def UseLibrary(self, library_name):
		lib = self.extern_libs[library_name]
		
		for i in lib:
			lib_include_path = os.path.split(i.path)[0]
			self.PrependUnique(LIBS = [library_name], LIBPATH = ['#' + lib_include_path])
			
		self.PrependUnique(CPPPATH = lib.include_dirs)
		
		return lib

	def ConfirmLibs(self, conf, libs):
		
		conf.AddTests({ 'CheckPKG' : CheckPKG })
		
		for evar, args in libs.iteritems():
			found = False
			for name, version in args['checks'].iteritems():
				if conf.CheckPKG(name + ' ' + version):
					self[evar]=name
					found = True;
					break
			if not found:
				if 'required' in args:
					if not args['required']==False:
						self.Exit(1)
				else:
					self.Exit(1)

	def InstallPerm(self, dir, source, perm):
		obj = self.Install(dir, source)
		for i in obj:
			self.AddPostAction(i, Chmod(i, perm))
		return dir

	InstallProgram = lambda self, dir, source: GpickEnvironment.InstallPerm(self, dir, source, 0755)
	InstallData = lambda self, dir, source: GpickEnvironment.InstallPerm(self, dir, source, 0644)

	def GetSourceFiles(self, dir_exclude_pattern, file_exclude_pattern):
		dir_exclude_prog = re.compile(dir_exclude_pattern)
		file_exclude_prog = re.compile(file_exclude_pattern)
		files = []
		MatchFiles(files, self.GetLaunchDir(), os.sep, dir_exclude_prog, file_exclude_prog)
		return files

	def GetVersionInfo(self):
		try:
			svn_revision = subprocess.Popen(['svnversion', '-n',  self.GetLaunchDir()], shell=False, stdout=subprocess.PIPE).communicate()[0]
			svn_revision = str(svn_revision)
			if svn_revision=="exported":
				svn_revision=""
			svn_revision=svn_revision.replace(':','.')
			svn_revision=svn_revision.rstrip('PSM')
			revision=svn_revision;
		except OSError, e:
			revision = ''

		self.Replace(GPICK_BUILD_REVISION = revision,
			GPICK_BUILD_DATE =  time.strftime ("%Y-%m-%d"),
			GPICK_BUILD_TIME =  time.strftime ("%H:%M:%S"));	

def RegexEscape(str):
	return str.replace('\\', '\\\\')

def WriteNsisVersion(target, source, env):
	for t in target:
		for s in source:
			file = open(str(t),"w")
			file.writelines('!define VERSION "' + str(env['GPICK_BUILD_VERSION']) + '"')
			file.close()
	return 0

def Glob(path):
	files = []
	for f in glob.glob(os.path.join(path, '*')):
		if os.path.isdir(str(f)):
			files.extend(Glob(str(f)));
		else:
			files.append(str(f));
	return files


