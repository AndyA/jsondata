provider jsondata {
  probe retain(void *v);
  probe release(void *v);
  probe rehash(void *v);
  probe string_new(size_t size);
};

#pragma D attributes Evolving/Evolving/Common provider jsondata provider
#pragma D attributes Evolving/Evolving/Common provider jsondata module
#pragma D attributes Evolving/Evolving/Common provider jsondata function
#pragma D attributes Evolving/Evolving/Common provider jsondata name
#pragma D attributes Evolving/Evolving/Common provider jsondata args
