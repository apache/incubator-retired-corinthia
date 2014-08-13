//
//  WordLists.h
//  DocFormats
//
//  Created by Peter Kelly on 29/12/12.
//  Copyright (c) 2012-2014 UX Productivity Pty Ltd. All rights reserved.
//

#ifndef DocFormats_WordLists_h
#define DocFormats_WordLists_h

struct WordConverter;

double listDesiredIndent(struct WordConverter *conv, const char *numId, const char *ilvl);

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                            WordLists                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

void WordPostProcessHTMLLists(struct WordConverter *conv);
void WordPreProcessHTMLLists(struct WordConverter *conv);
void WordFixLists(struct WordConverter *conv);

#endif
