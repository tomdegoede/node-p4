{
  'variables': {
    'includeDir':'./tmp/p4api/include/p4',
    'libDir':'../tmp/p4api/lib/',
    'srcDir':'./src/cpp/'
  },
  'targets': [
    {
      'target_name': 'p4nodeapi',
      'sources': [ '<(srcDir)p4nodeapi.cc', '<(srcDir)basicuser.cc' ],
      'include_dirs': ['<(includeDir)'],
      'libraries': ['<(libDir)libclient.lib','<(libDir)librpc.lib','<(libDir)libsupp.lib', '<(libDir)ssleay32MT.lib', '<(libDir)libeay32MT.lib', 'Ws2_32.lib'],
    }
  ]
}
