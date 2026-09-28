#pragma once
// ============================================================
//  SharedStore.h  (generic template)
//
//  Thin wrapper around Preferences (NVS) so two unrelated sketches
//  can read/write the same keys, even though they're completely
//  different compiled binaries running in different OTA partitions.
//  NVS lives in its own partition, untouched by either app slot.
//
//  Agree on key names + types between both sketches up front --
//  there's no compiler to catch a mismatch (e.g. sketch A writing
//  a float under "temp" while sketch B reads it as an int).
//
//  IMPORTANT: keep this file IDENTICAL in both sketch folders.
// ============================================================

#include <Preferences.h>

#define SHARED_STORE_NAMESPACE "shared"   // change per-project if you like

// ---- Raw bytes (e.g. binary blobs, structs, IR codes) ----
inline bool sharedPutBytes(const char *key, const void *data, size_t len) {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, false)) return false;
  size_t written = prefs.putBytes(key, data, len);
  prefs.end();
  return written == len;
}

// outBuf must be at least expectedLen bytes. Returns false if the key
// is missing or stored under a different length.
inline bool sharedGetBytes(const char *key, void *outBuf, size_t expectedLen) {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, true)) return false;
  size_t len = prefs.getBytesLength(key);
  bool ok = false;
  if (len == expectedLen) {
    ok = (prefs.getBytes(key, outBuf, expectedLen) == expectedLen);
  }
  prefs.end();
  return ok;
}

// ---- Convenience typed helpers ----
inline bool sharedPutInt(const char *key, int32_t value) {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, false)) return false;
  size_t n = prefs.putInt(key, value);
  prefs.end();
  return n > 0;
}

// Returns defaultValue if the key doesn't exist yet.
inline int32_t sharedGetInt(const char *key, int32_t defaultValue = 0) {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, true)) return defaultValue;
  int32_t v = prefs.getInt(key, defaultValue);
  prefs.end();
  return v;
}

inline bool sharedPutFloat(const char *key, float value) {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, false)) return false;
  size_t n = prefs.putFloat(key, value);
  prefs.end();
  return n > 0;
}

inline float sharedGetFloat(const char *key, float defaultValue = 0.0f) {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, true)) return defaultValue;
  float v = prefs.getFloat(key, defaultValue);
  prefs.end();
  return v;
}

inline bool sharedPutString(const char *key, const String &value) {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, false)) return false;
  size_t n = prefs.putString(key, value);
  prefs.end();
  return n > 0;
}

inline String sharedGetString(const char *key, const String &defaultValue = "") {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, true)) return defaultValue;
  String v = prefs.getString(key, defaultValue);
  prefs.end();
  return v;
}

// True if `key` exists at all (any type).
inline bool sharedHasKey(const char *key) {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, true)) return false;
  bool exists = prefs.isKey(key);
  prefs.end();
  return exists;
}

// Wipes every key in the shared namespace. Handy for a clean re-test.
inline void sharedClearAll() {
  Preferences prefs;
  if (!prefs.begin(SHARED_STORE_NAMESPACE, false)) return;
  prefs.clear();
  prefs.end();
}
