#include <stdio.h>
#include "mootToken.h"
#include "mootTokenXmlDoc.h"

using namespace moot;

void churntest(int argc, char **argv) {
  TokenXmlDoc doc;
  TokenXmlDoc tmp;
  bool dolog = true;
  bool dolocal = true;
  bool doformat = true;

  TokenXmlDoc &doc2 = dolocal ? doc : tmp;
  doc2.xml_format  = doformat;

  if (argc > 1)
    doc.loadFilename(argv[1]);
  else
    doc.loadFilename("-");

  int typ;
  char buf[512];
  mootSentence sent;
  xmlNodePtr cmt = NULL;
  if (dolog) cmt = doc2.addComment((doc2.root()
				     ? doc2.root()
				     : doc2.addNewNode(NULL, "xmlchurn.nonlocal.root")),
				    "xmlchurn log\n");

  while ((typ = doc.get_sentence()) != TokTypeEOF) {
    mootSentence &sent = doc.sentence();

    if (dolog) doc2.addContent(cmt, "SENTENCE:\n");
    for (mootSentence::const_iterator si = sent.begin();
	 dolog && si != sent.end();
	 si++)
      {
	sprintf(buf,
		"\t+ TOKEN: type=`%s' ; toktext=`%s'\t ; besttag=`%s'\n",
		mootTokenTypeNames[si->toktype()],
		si->text().c_str(),
		si->besttag().c_str());
	doc2.addContent(cmt,buf);

	for (mootToken::AnalysisSet::const_iterator ai = si->analyses().begin();
	     ai != si->analyses().end();
	     ai++)
	  {
	    sprintf(buf,
		    "\t  - ANALYSIS: cost=%g\t ; tag=`%s'\t ; details=`%s'\t\n",
		    ai->cost,
		    ai->tag.c_str(),
		    ai->details.c_str());
	    doc2.addContent(cmt,buf);
	  }
      }

    //-- put sentence (local)
    doc2.put_sentence(sent);
  }
  if (dolog) doc2.addContent(cmt, "EOF");

  //-- save doc (local)
  doc2.saveFilename("-");

  //-- save doc (nonlocal)
  //doc2.saveFilename("-");
}

int main (int argc, char **argv) {

  //for (int i = 0; i < 10; i++)
  churntest(argc, argv);

  return 0;
}
