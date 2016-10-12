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
      'libraries': ['<(libDir)libclient.a','<(libDir)librpc.a','<(libDir)libsupp.a'],
    }
  ]
}
