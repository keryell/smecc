/* Post process the smecc output to massage the accelerated code into the
   lower IR runtime API

   Ronan Keryell @ SILKAN
   for Artemis SMECY European project
 */

/* For EXIT_SUCCESS */
#include <cstdlib>

#include <iterator>
#include <iostream>
#include <fstream>
/* Not yet implemented in gcc 4.7.2
   #include <regex>
   So rely on TR1 from Boost instead
*/
#include <boost/regex.hpp>
/*
#include <sstream>
#include <string>
*/

/** Read a file content into a string
 */
std::string slurp_file(const char file_name[]) {
  // Open the given file for reading as a stream
  std::ifstream f { file_name }; // , std::ifstream::binary
  // Do not skip white spaces with the right manipulator
  f >> std::noskipws;
  // Create a range of iterators to read the file
  std::istream_iterator<char> in { f };
  std::istream_iterator<char> eof;
  // Copy all the characters from the file at the end of content
  std::string content;
  copy(in, eof, std::back_inserter(content));
  return content;
}


/** Transform a string with ","-separated items into a string of
    ";"-separated items, each one on a different line
*/
std::string from_comma_list_to_semicolumn_list(const std::string &s) {
  static const boost::regex r { "," };
  std::string result;
  for (boost::sregex_token_iterator item { s.cbegin(), s.cend(), r, -1 }, end;
       item != end;
       ++item)
    result += "  " + *item + ";\n";
  return result;
}


/** Process a whole file toward another file with ".smecc_pp" extension
 */
void process(const char file_name[]) {
  // std::clog << file_name << ':' << std::endl
  //          << slurp_file(file_name) << std::endl;
  const std::string file_content = slurp_file(file_name);
  std::string transformed_content;
  /* Capture all the first lines of the declaration or the definitition of
     generated accelerated functions:
     - subexpression 1 matches the begin up to the '(';
     - subexpression 2 matches the local name of the accelerated function;
     - subexpression 3 matches the parameter list;
     - subexpression 4 matches the ';' at the end in case of a declaration;
     - subexpression 5 matches the '{' at the begining of the function
       body in case of a definition.
  */
  static const boost::regex r { R"(^(void smecy_func_([^\(]*)\()(.*?)\)(;?)\n(\{?))" };
  std::string::size_type previous_match_end = 0;
  for (boost::sregex_iterator m { file_content.cbegin(), file_content.cend(), r },
         m_end;
       m != m_end;
       ++m) {
    /*
    std::clog << "At position " << m->position() << ": "
              << m->str() << std::endl
              << (*m)[0] << std::endl << (*m)[1] << std::endl
              << (*m)[2] << std::endl << (*m)[3] << std::endl
              << (*m)[4] << std::endl;
    std::clog << "previous_match_end: " << previous_match_end << std::endl;
    */
    /* Output the litteral input since the previous match end up to before
       the current match */
    transformed_content += file_content.substr(previous_match_end,
                                               m->position() - previous_match_end);
    // For next loop, think to copy the input from the end of this match
    previous_match_end = m->position() + m->length();
    /* If there is a ';' after the (parameter list), this is only a
       function declaration. */
    if ((*m)[4] == ";") {
      /* So replace it by a declaration with an empty body only to have
         the old code using this function to compile. It will not be run
         on the accelerator side anyway */
      transformed_content += "\n/* Dummy function to have the old remaining "
        "code still compiling.\n"
        "    The smecy_accel_" + (*m)[2] + "() will be called instead.\n */\n"
        + (*m)[1] + (*m)[3] + ") {\n}\n";
    }
    else {
      /* This is a declaration, transform the parameter list into
         automatic variables so that the MCAPI runtime have stack
         allocation to store what it receives.

         Change also the function name to smecy_accel_... to avoid
         conflicts with the old code.

         First the declaration header:
      */
      transformed_content += "/* Code to be executed on the accelerator */\n"
        "void smecy_accel_" + (*m)[2] + "(SMECY_accel_func_args) {\n"
        "  /* On-stack storage to send/receive address parameters\n"
        "     to/from the host: */\n";
      /* Then the new automatic variables: */
      transformed_content += from_comma_list_to_semicolumn_list((*m)[3]);
    }
  }
  // std::clog << "previous_match_end: " << previous_match_end << std::endl;

  // Copy the remaining input, if any
  transformed_content += file_content.substr(previous_match_end);
  // std::clog << "Exit:" << std::endl << transformed_content << std::endl;
  std::ofstream output { std::string { file_name } + ".smecc_pp" };
  output << transformed_content;
}


/** Process a list of files given as arguments to the command
 */
int main(const int argc, const char * const argv[]) {

  for (auto arg = &argv[1]; arg != &argv[argc]; ++arg)
    process(*arg);

  return EXIT_SUCCESS;
}
