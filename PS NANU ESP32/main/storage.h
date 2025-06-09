#pragma once
#include <Preferences.h>
#include <vector>
#include <Arduino.h>  // pentru String

class Storage {
public:
  Storage(const char* ns, int capacity)
    : _ns(ns), _cap(capacity), _count(0), _idx(0) {}

  void begin() {
    prefs.begin(_ns, false);
    _count = prefs.getInt("count", 0);
    _idx   = prefs.getInt("idx", 0);
  }

  void add(const String& v) {
    prefs.putString(key(_idx).c_str(), v.c_str());
    _idx = (_idx + 1) % _cap;
    if (_count < _cap) _count++;
    prefs.putInt("idx", _idx);
    prefs.putInt("count", _count);
  }

  std::vector<String> getAll() {
    std::vector<String> out;
    for (int i = 0; i < _count; ++i) {
      int pos = (_idx + _cap - _count + i) % _cap;
      String s = prefs.getString(key(pos).c_str(), "");
      out.push_back(s);
    }
    return out;
  }

  void removeAt(int i) {
    auto arr = getAll();
    if (i < 0 || i >= (int)arr.size()) return;
    prefs.clear();
    begin();
    for (int j = 0; j < (int)arr.size(); ++j) {
      if (j != i) add(arr[j]);
    }
  }

private:
  String key(int i) const { return String("it") + i; }
  const char* _ns;
  int _cap, _count, _idx;
  Preferences prefs;
};
