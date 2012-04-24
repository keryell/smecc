/* Some global utility methods to SME-CC */

#include "public.hpp"

namespace smecy {

  /* Construct a string with some file info on an AST node
   */
  std::string debugInfo(SgNode* context) {
    std::stringstream ss("");
    if (context)
      ss << context->get_file_info()->get_filenameString()
	 << ":"
	 << context->get_file_info()->get_line()
	 << ": ";

    // RK: I guess this unfortunately imply a std::string copy here:
    return ss.str();
  }
}
