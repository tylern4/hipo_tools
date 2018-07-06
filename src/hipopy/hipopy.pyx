# distutils: language = c++
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "hipo/node.h" namespace "hipo":
    cdef cppclass node[T]:
      node() except +
      node(int, int) except +
      T getValue(int)
      void   reset()
      void   show()
      int    getLength()
      char  *getAddress()
      int    getBytesLength()
      void   setLength(int)
      void   setAddress(char *)

cdef extern from "hipo/reader.h" namespace "hipo":
    cdef cppclass reader:
      reader() except +
      reader(bool) except +
      vector[string] getDictionary()
      void open(char*)
      int getRecordCount()
      bool next()
      bool  isOpen()
      void  showInfo()
      #node *getBranch[T](int, int)
      node *getBranch[T](char*,char*)


def str_to_char(str name):
  cdef bytes name_bytes = name.encode()
  cdef char* c_name = name_bytes
  return c_name


cdef class int_node:
  cdef node[int]*c_node
  def __cinit__(self):
    self.c_node = new node[int]()

  cdef setup(self, node[int]* node):
    self.c_node = node

  def getValue(self, x):
    return self.c_node.getValue(x)

  def getLength(self):
    return self.c_node.getLength()


cdef class float_node:
  cdef node[float]*c_node
  def __cinit__(self):
    self.c_node = new node[float]()

  cdef setup(self, node[float]* node):
    self.c_node = node

  def getValue(self, x):
    return self.c_node.getValue(x)

  def getLength(self):
    return self.c_node.getLength()

cdef class hipo_reader:
  cdef reader*c_reader
  def __cinit__(self):
    self.c_reader = new reader(True)

  def __cinit__(self, str filename, bool randomAccess = True):
    self.c_reader = new reader(randomAccess)
    self.open(filename)

  def open(self, str filename):
    cdef bytes filename_bytes = filename.encode()
    cdef char* c_filename = filename_bytes
    self.c_reader.open(c_filename)

  def isOpen(self):
    return self.c_reader.isOpen()

  def showInfo(self):
    self.c_reader.showInfo()

  def getRecordCount(self):
    return self.c_reader.getRecordCount()

  def next(self):
    return self.c_reader.next()

  def getDictionary(self):
    return self.c_reader.getDictionary()

  def getIntNode(self, str group, str item):
    cdef node[int]*c_node
    c_group = str_to_char(group)
    c_item = str_to_char(item)
    c_node = self.c_reader.getBranch[int](c_group,c_item)
    py_node = int_node()
    py_node.setup(c_node)
    return py_node

  def getFloatNode(self, str group, str item):
    cdef node[float]*c_node
    c_group = str_to_char(group)
    c_item = str_to_char(item)
    c_node = self.c_reader.getBranch[float](c_group,c_item)
    py_node = float_node()
    py_node.setup(c_node)
    return py_node
