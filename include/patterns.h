#ifndef _patterns_h_
#define _patterns_h_
/**
 * Some macros helping to implement patterns
 */

/**
 * Macro that
 * - creates a private Constructor (the {} brackets) are needed there).
 * - makea sure the copy constructor and assignment operator are inaccessible(especially from outside), 
 *   otherwise, we may accidentally get copies of the singleton appearing.
 * 
 * Scott Meyers mentions in his Effective Modern C++ book, that deleted functions should generally be public
 * as it results in better error messages due to the compilers behavior to check accessibility before deleted status
 */
#if (__cplusplus >= 201103L)
  #define MAKE_SINGLETON(ClassName) \
    public: \
      static ClassName & getInstance() { \
        static ClassName instance; \
        return instance; \
      };\
      ClassName(ClassName const&) = delete; \
      void operator=(ClassName const&)  = delete; \
    private: \
        ClassName() {};
#else
  #define MAKE_SINGLETON(ClassName) \
    public: \
      static ClassName & getInstance() { \
        static ClassName instance; \
        return instance; \
      };\
    private: \
        ClassName() {}; \
        ClassName(ClassName const&); \
        void operator=(ClassName);
#endif

#endif