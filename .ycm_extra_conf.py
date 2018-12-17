def FlagsForFile(*args):
  print(args)
  return { "flags" : ["-O0"],
      "do_cache" : False }
