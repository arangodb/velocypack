#define VELOCYPACK_GLOBAL_EXCEPTION_TRY                              \
  try {

#define VELOCYPACK_GLOBAL_EXCEPTION_CATCH                            \
  } catch (std::exception const& ex) {                               \
    std::cerr << "caught exception: " << ex.what() << std::endl;     \
    return EXIT_FAILURE;                                             \
  } catch (...) {                                                    \
    std::cerr << "caught unknown exception" << std::endl;            \
    return EXIT_FAILURE;                                             \
  }                                                                  

