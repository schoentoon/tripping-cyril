#ifndef _TC_STRING_H
#define _TC_STRING_H

#include <string>

using namespace std;

namespace trippingcyril {

class String : public string {
public:
  explicit String(bool b) : string(b ? "true" : "false") {};
  explicit String(char c);
  explicit String(unsigned char c);
  explicit String(short i);
  explicit String(unsigned short i);
  explicit String(int i);
  explicit String(unsigned int i);
  explicit String(long i);
  explicit String(unsigned long i);
  explicit String(long long i);
  explicit String(unsigned long long i);
  explicit String(double i, unsigned int precision = 2);
  explicit String(float i, unsigned int precision = 2);
  String(const std::stringstream& stream);
  String() : string() {};
  String(const char* c) : string(c) {};
  String(const char* c, size_t l) : string(c, l) {};
  String(const string& s) : string(s) {};
  virtual ~String() {};

  bool ToBool() const;
  short ToShort(unsigned int base = 10) const;
  unsigned short ToUShort(unsigned int base = 10) const;
  int ToInt(unsigned int base = 10) const;
  unsigned int ToUInt(unsigned int base = 10) const;
  long ToLong(unsigned int base = 10) const;
  unsigned long ToULong(unsigned int base = 10) const;
  long long ToLongLong(unsigned int base = 10) const;
  unsigned long long ToULongLong(unsigned int base = 10) const;
  double ToDouble() const;
  float ToFloat() const;
};

};

#endif //_TC_STRING_H