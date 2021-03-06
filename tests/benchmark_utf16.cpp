#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <iostream>
#include <cassert>
#include <emscripten.h>

double test(const unsigned short *str) {
  double res = EM_ASM_DOUBLE({
    var t0 = _emscripten_get_now();
    var str = Module.UTF16ToString($0);
    var t1 = _emscripten_get_now();
    Module.print('t: ' + (t1 - t0) + ', len(result): ' + str.length + ', result: ' + str.slice(0, 100));
    return (t1-t0);
  }, str);
  return res;
}

unsigned short *utf16_corpus = 0;
long utf16_corpus_length = 0;

unsigned short *randomString(int len) {
  if (!utf16_corpus) {
//    FILE *handle = fopen("ascii_corpus.txt", "rb");
    FILE *handle = fopen("utf16_corpus.txt", "rb");
    fseek(handle, 0, SEEK_END);
    utf16_corpus_length = ftell(handle)/2;
    assert(utf16_corpus_length > 0);
    utf16_corpus = new unsigned short[utf16_corpus_length+1];
    fseek(handle, 0, SEEK_SET);
    fread(utf16_corpus, 2, utf16_corpus_length, handle);
    fclose(handle);
    utf16_corpus[utf16_corpus_length] = 0;
  }
  int startIdx = rand() % (utf16_corpus_length - len);
  while((utf16_corpus[startIdx] & 0xFF00) == 0xDC00) {
    ++startIdx;
    if (startIdx + len > utf16_corpus_length) len = utf16_corpus_length - startIdx;
  }
  assert(len > 0);
  unsigned short *s = new unsigned short[len+1];
  memcpy(s, utf16_corpus + startIdx, len*2);
  s[len] = 0;
  while(((unsigned short)s[len-1] & 0xFF00) == 0xD800) { s[--len] = 0; }
  assert(len >= 0);
  return s;
}

int main() {
  srand(time(NULL));
  double t = 0;
  double t2 = emscripten_get_now();
  for(int i = 0; i < 10; ++i) {
    // FF Nightly: Already on small strings of 64 bytes in length, TextDecoder trumps in performance.
    unsigned short *str = randomString(100);
    t += test(str);
    free(str);
  }
  double t3 = emscripten_get_now();
  printf("OK. Time: %f (%f).\n", t, t3-t2);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
