#define DOGE_2 1

#include "mpc.h"

#if defined(DOGE_1)
int main(int argc, char** argv) {

  /* Build a parser 'Adjective' to recognize descriptions */
  mpc_parser_t* Adjective = mpc_or(4, 
    mpc_sym("wow"), mpc_sym("many"),
    mpc_sym("so"),  mpc_sym("such")
  );

  /* Build a parser 'Noun' to recognize things */
  mpc_parser_t* Noun = mpc_or(5,
    mpc_sym("lisp"), mpc_sym("language"),
    mpc_sym("book"), mpc_sym("build"), 
    mpc_sym("c")
  );
  
  mpc_parser_t* Phrase = mpc_and(2, mpcf_strfold, 
    Adjective, Noun, free);
  
  mpc_parser_t* Doge = mpc_many(mpcf_strfold, Phrase);

  /* Do some parsing here... */
  
  mpc_delete(Doge);
  puts("DOGE_1");
  
  return 0;
  
}
#elif defined(DOGE_2)
int main(int argc, char** argv) {

  mpc_parser_t* Adjective = mpc_new("adjective");
  mpc_parser_t* Noun      = mpc_new("noun");
  mpc_parser_t* Phrase    = mpc_new("phrase");
  mpc_parser_t* Doge      = mpc_new("doge");

  mpca_lang(MPCA_LANG_DEFAULT,
    "                                           \
      adjective : \"wow\" | \"many\"            \
                |  \"so\" | \"such\";           \
      noun      : \"lisp\" | \"language\"       \
                | \"book\" | \"build\" | \"c\"; \
      phrase    : <adjective> <noun>;           \
      doge      : <phrase>*;                    \
    ",
    Adjective, Noun, Phrase, Doge);

  /* Do some parsing here... */

  mpc_cleanup(4, Adjective, Noun, Phrase, Doge);
  puts("DOGE_2");
  
  return 0;
  
}
#endif
