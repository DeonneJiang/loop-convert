/***   CIrewriter.cpp   ******************************************************
 * This code is licensed under the New BSD license.
 * See LICENSE.txt for details.
 *
 * This tutorial was written by Robert Ankeney.
 * Send comments to rrankene@gmail.com.
 * 
 * This tutorial is an example of using the Clang Rewriter class coupled
 * with the RecursiveASTVisitor class to parse and modify C code.
 *
 * Expressions of the form:
 *     (expr1 && expr2)
 * are rewritten as:
 *     L_AND(expr1, expr2)
 * and expressions of the form:

 *     (expr1 || expr2)
 * are rewritten as:a
 *     L_OR(expr1, expr2)
 *
 * Functions are located and a comment is placed before and after the function.
 *
 * Statements of the type:
 *   if (expr)
 *      xxx;
 *   else
 *      yyy;
 *
 * are converted to:
 *   if (expr)
 *   {
 *      xxx;
 *   }
 *   else
 *   {
 *      yyy;
 *   }
 *
 * And similarly for while and for statements.
 *
 * Interesting information is printed on stderr.
 *
 * Usage:
 * CIrewriter <options> <file>.c
 * where <options> allow for parameters to be passed to the preprocessor
 * such as -DFOO to define FOO.
 *
 * Generated as output <file>_out.c
 *
 * Note: This tutorial uses the CompilerInstance object which has as one of
 * its purposes to create commonly used Clang types.
 *****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include <system_error>
#include <fstream>
#include <iostream>

#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Analysis/MemoryBuiltins.h"


#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;

#define _funcsum 70
#define _funcnamelen 50
#define _dangerfuncsum 20
#define _vartypesum 20

int           stmtsum=0;
std::ofstream out("/root/result.txt",std::ios::app);	
std::ofstream func_blocks("/root/func_blocks.txt",std::ios::app);
int           ThinPathSum=0;
//address disinfect
int           pos = -1;
int           stack = -1;
int           fs = 0;
int           blockflag=0;                                    //define blocks[64000] with extern or not
char          checkleak[1024];
char          vartypearray[_vartypesum][30]={};               // var typt len 20
int           varsumarray[_vartypesum]={0,0,0,0,0,0,0,0,0,0}; // var type sum 10
int           varstrategy=0;
//static analysis
SourceLocation  FuncEnd;
SourceLocation  FuncEND1;
bool            func_call[_funcsum][_funcsum]={0};            // func sum 20,[x][x] = 1,shows vul
char            func_name[_funcsum][_funcnamelen]={0};
int             func_declare[_funcsum]={0};
int             func_declare_sum=0;
int             func_now=0;
int             func_main=0;
int             danger_func_path[2*_funcsum][_funcsum]={0};
int             func_buf[_funcsum]={0};


class MyRecursiveASTVisitor
    : public RecursiveASTVisitor<MyRecursiveASTVisitor>
{

 public:
  MyRecursiveASTVisitor(Rewriter &R) : Rewrite(R) { }
  bool VisitDecl(Decl* d);
  void InstrumentStmt(Stmt *s,int flag);
  bool GetFuncCallGraph(Stmt *s);
  void AddrDisinfect(Stmt *s);
  void VisitThinPath(Stmt *s,int flag);
  bool VisitStmt(Stmt *s);
  bool VisitFunctionDecl(FunctionDecl *f);
  Expr *VisitBinaryOperator(BinaryOperator *op);
  SizeOffsetType 	visitGlobalVariable (GlobalVariable &GV);


  Rewriter &Rewrite;
};


// Decl instrument
bool MyRecursiveASTVisitor::VisitDecl(Decl* d){    
    ASTContext& ctx = d->getASTContext();
    SourceManager& sm = ctx.getSourceManager();
    

    const RawComment* rc = d->getASTContext().getRawCommentForDeclNoCache(d);
    if (rc)
    {
        llvm::errs() << "comment found\n";
        //Found comment!
        SourceRange range = rc->getSourceRange();

        PresumedLoc startPos = sm.getPresumedLoc(range.getBegin());
        PresumedLoc endPos = sm.getPresumedLoc(range.getEnd());

        std::string raw = rc->getRawText(sm);
        std::string brief = rc->getBriefText(ctx);

        llvm::errs() << "raw:" << raw << "\n";
        llvm::errs() << "brief:" << brief << "\n";

        //SourceLocation ST = ((CompoundStmt *)s)->getLBracLoc().getLocWithOffset(1);
        //Rewrite.InsertText(range.getEnd(), "/*-----------*/", true, true);
        if(brief=="write covercity"){ 				//modify here to accomplish icom
             char temp2[1000]={0};
             sprintf(temp2,"\n  FILE *fp;\
                   \n  if((fp=fopen(\"abc\",\"wt+\")) == NULL){\
                   \n  printf(\"error\");\
                   \n  return -1;\
                   \n  }\
                   \n  fwrite(blocks,sizeof(unsigned char),64000,fp);\
                   \n  fclose(fp);\
                   \n");

             Rewrite.InsertText(range.getEnd(), temp2, true, true);
        }
        
        // ... Do something with positions or comments
    }

    return true;
   /* */
}

SizeOffsetType 	MyRecursiveASTVisitor::visitGlobalVariable (GlobalVariable &GV){
  llvm::error<< "sssssss";
  return 0;
}


// Override Binary Operator expressions
Expr *MyRecursiveASTVisitor::VisitBinaryOperator(BinaryOperator *E){
  // Determine type of binary operator
  if (E->isLogicalOp())
  {
    // Insert function call at start of first expression.
    // Note getBeginLoc() should work as well as getExprLoc()
    Rewrite.InsertText(E->getLHS()->getExprLoc(),
             E->getOpcode() == BO_LAnd ? "L_AND(" : "L_OR(", true);

    // Replace operator ("||" or "&&") with ","
    Rewrite.ReplaceText(E->getOperatorLoc(), E->getOpcodeStr().size(), ",");

    // Insert closing paren at end of right-hand expression
    Rewrite.InsertTextAfterToken(E->getRHS()->getEndLoc(), ")");
  }
  else
  // Note isComparisonOp() is like isRelationalOp() but includes == and !=
  if (E->isRelationalOp())
  {
    llvm::errs() << "Relational Op " << E->getOpcodeStr() << "\n";
  }
  else
  // Handles == and != comparisons
  if (E->isEqualityOp())
  {
    llvm::errs() << "Equality Op " << E->getOpcodeStr() << "\n";
  }

  return E;
}





// AddrDisinfect - add after var
void MyRecursiveASTVisitor::AddrDisinfect(Stmt *s){
	char temp[100];
    //sprintf(temp,"\tint stack%d = %d;\n",stack,fs);
    SourceLocation ST = s->getBeginLoc();
    SourceManager& sr = Rewrite.getSourceMgr();
    int offset = Lexer::MeasureTokenLength(ST,
                                           Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) + 1;
    SourceLocation END = s->getEndLoc();
    int offset1 = Lexer::MeasureTokenLength(END,
                                           Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) + 1;
    SourceLocation END1 = END.getLocWithOffset(offset1);
    int STgre = ST.getRawEncoding();
    const char *endCharPtr2 = sr.getCharacterData(ST);
    int stroffset = 0;
    int i=0,j=0;
   
    char getstrData[100];//all the line
    char getvarName[100];//var name

    while(endCharPtr2[stroffset]!=';'){
    	getstrData[stroffset] = endCharPtr2[stroffset];
    	stroffset++;
    	if (stroffset>80) {
    		llvm::errs() << "too long senetnces"<<"\n";
    		break;
    	}
    }
    getstrData[stroffset]='\0';
    varstrategy = 0;
    char *pp;
    pp = strchr(getstrData,'[');
    if (pp!=NULL) varstrategy = 1;
    pp = strstr(getstrData,"long long");
    if (pp!=NULL) varstrategy = 1;
    
    while(getstrData[stroffset--]!=' ');
    if (getstrData[stroffset]=='=') {
    	stroffset--;
        while(getstrData[stroffset--]!=' ');
        stroffset--;
        while(getstrData[stroffset--]!=' ');
    }
    
    stroffset+=2;
    i=0;
    //llvm::errs()<<"AAAA";
    while(getstrData[i+stroffset]!=';'&&getstrData[i+stroffset]!='='){
    	getvarName[i] = getstrData[i+stroffset];
    	i++;
    	if (i>80) {
    		llvm::errs() << "too long senetnces"<<"\n";
    		break;
    	}
    }
    //llvm::errs()<<"BBBB";
    getvarName[i] = '\0';
    getstrData[--stroffset]='\0';

    i=0;
    while (varsumarray[i]!=0){
    	if (strcmp(vartypearray[i],getstrData)==0){
    		varsumarray[i]+=1;
    		break;
    	}
    	i++;
    	if (i>=_vartypesum) {
    		llvm::errs() << "too much var type"<<"\n";
    		break;
    	}
    }
    if (varsumarray[i]==0){
    	strcpy(vartypearray[i],getstrData);
    	varsumarray[i]=1;
    }

    if (varstrategy==1){

    	sprintf(temp,"\tchar v%d%d[8]={'\\x12','\\x34','\\x56','\\x78','\\x13','\\x24','\\x79','\\x00'};\n",i,varsumarray[i]);
    	Rewrite.InsertText(ST, temp, true, true);
    }
    else if(varsumarray[i]==1){
    	sprintf(temp,"\t%s v%d0=0x23;\n\t%s v%d%d=0x45;\n\tv%d0=&v%d%d - &%s;\n\tif (v%d0>0) ;\n",getstrData,i,getstrData,i,varsumarray[i],i,i,varsumarray[i],getvarName,i);
        
        
        Rewrite.InsertText(END1, temp, true, true);

    }
    else{
    	sprintf(temp,"\t%s v%d%d=0x45;\n\t%s v%d0=&v%d%d - &%s;\n\tif (v%d0>0) ;\n",getstrData,i,varsumarray[i],getstrData,i,i,varsumarray[i],getvarName,i);
        
        
        Rewrite.InsertText(END1, temp, true, true);
    }
    int arry;
    arry = i;
    
    
    //end 
    
    int offset2 = Lexer::MeasureTokenLength(FuncEnd,
                                           Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) - 100;
    SourceLocation FuncEnd1 = FuncEnd.getLocWithOffset(offset2);
    int Func1gre = FuncEnd1.getRawEncoding();
    int Funcgre = FuncEnd.getRawEncoding();
    const char *endCharPtr3 = sr.getCharacterData(FuncEnd1);
    char getReturn[100];
    i = 0;
    while (endCharPtr3[i]!='\0' && i<100){
    	getReturn[i] = endCharPtr3[i];
    	i++;
    }
    getReturn[i] = '\0';
    pp = strstr(getReturn,"return");
    j=&pp[0]-&getReturn[0];
    int offset3 = Lexer::MeasureTokenLength(FuncEnd,
                                           Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) - 100 + j;
    SourceLocation FuncEnd2 = FuncEnd.getLocWithOffset(offset3);

    if (pp==NULL)  
    Rewrite.InsertText(FuncEND1, checkleak, true, true);
    /*if (pp!=NULL){

        sprintf(temp,"\t333%d %d %d :%s %c%c%c\n",j,Func1gre,Funcgre,getReturn,endCharPtr3[0],endCharPtr3[1],endCharPtr3[4]);
        Rewrite.InsertText(FuncEnd1,temp,true,true);
    }*/
	if (varstrategy==1){
	    sprintf(temp,"\tif(strcmp(v%d%d,\"\\x12\\x34\\x56\\x78\\x13\\x24\\x79\")!=0) print2(\"v%d%d stack overflow\\n\");\n",arry,varsumarray[arry],arry,varsumarray[arry]);
	    strcat(checkleak,temp);
        //Rewrite.InsertText(FuncEnd2,temp,true,true);
    }
    else if (varstrategy==0 && varsumarray[arry]==1){
    	/*sprintf(temp,"\tif(v%d0!=0x23) print2(\"v%d0 stack overflow\\n\");\n\tif(v%d1!=0x45) print2(\"v%d1 stack overflow\\n\");\n",arry,arry,arry,arry);*/
    	sprintf(temp,"\tif(v%d1!=0x45) print2(\"v%d1 stack overflow\\n\");\n",arry,arry);
    	strcat(checkleak,temp);
        //Rewrite.InsertText(FuncEnd2,temp,true,true);
    }
    else{
    	sprintf(temp,"\tif(v%d%d!=0x45) print2(\"v%d%d stack overflow\\n\");\n",arry,varsumarray[arry],arry,varsumarray[arry]);
    	strcat(checkleak,temp);
        //Rewrite.InsertText(FuncEnd2,temp,true,true);
    }
  
}

// GetFuncCallGraph - All Func Call 
bool MyRecursiveASTVisitor::GetFuncCallGraph(Stmt *s){
	char temp[100];
    //sprintf(temp,"\tint stack%d = %d;\n",stack,fs);
    SourceLocation ST = s->getBeginLoc();
    SourceManager& sr = Rewrite.getSourceMgr();
    int offset = Lexer::MeasureTokenLength(ST,
                                           Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) + 1;
    int STgre = ST.getRawEncoding();
    const char *endCharPtr2 = sr.getCharacterData(ST);
    int stroffset = 0;
    int i=0,j=0;

    char getstrData[200];//all the line
    char getvarName[100];//var name

    while(endCharPtr2[stroffset]!=';'){
    	getstrData[stroffset] = endCharPtr2[stroffset];
    	stroffset++;
    	if (stroffset>80) {
    		llvm::errs() << "too long sentences"<<"\n";
    		break;
    	}
    }
    
    char *strhave;
    getstrData[stroffset]='\0';
    strhave = strchr(getstrData,'(');
    if (strhave!=NULL){
    	strhave[0] = '\0';
    }
    llvm::errs() << "Found CallExpr:"<< getstrData<<"\n";
    /*for (i=0;i<_dangerfuncsum;i++){
    	if(dangerFunc[i][0]=='\0') break;
    	if (strcmp(getstrData,dangerFunc[i])==0){
    		func_call[func_now][func_now]=1;
    		llvm::errs() << "vulnerable function"<< "\n";
    		return true;
    		break;
    	}
    }*/
    if (out.is_open()) out<<" ,to "<<getstrData;  
    else  llvm::errs() << "out.close"<< "\n";
    		
    for (i=0;i<_funcsum;i++){
    	if(func_name[i][0]=='\0') break; 
    	if (strcmp(getstrData,func_name[i])==0 && i != func_now){
    		func_call[func_now][i]=1;
    		llvm::errs() << "a function call: "<<func_name[func_now]<<" to "<<func_name[i]<< "\n";
    		
    		break;
    	}
    }
    
    if(i<_funcsum && func_name[i][0]=='\0') {
    	strcpy(func_name[i],getstrData);
    	if (i != func_now) func_call[func_now][i]=1;
    	llvm::errs() << "a function call: "<<func_name[func_now]<<" to "<<func_name[i]<< "\n";
    	i++;
    }
    else if (i>=_funcsum){
    	llvm::errs() << "too much functions"<< "\n";
    }
}

// Stmt Instrument
void MyRecursiveASTVisitor::InstrumentStmt(Stmt *s, int flag)
{
  char temp[256]={0};
  std::ifstream infile("loopconvert.txt");
  infile>>pos;
  pos++;
  infile.close();
  char char_pos[15]={0}; 
  sprintf(char_pos,"%d",pos%100000);
  SourceLocation STT = s->getBeginLoc();

  llvm::errs()<<STT.getRawEncoding()<<"\n";
  if(STT.getRawEncoding()==0)
  {
    llvm::errs()<<"LocStart == 0, ret\n";
    return;
  }

  if(STT.isMacroID())
  {
  	llvm::errs() << "isMacroID exists, but what's this\n";
  }

  
  // Only perform if statement is not compound
  if (flag==2){
    SourceLocation ST = s->getBeginLoc();
    SourceLocation ENDD = s->getEndLoc();
    SourceManager& sr = Rewrite.getSourceMgr();
    int offset = Lexer::MeasureTokenLength(ST,
                                           Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) + 1;
    int STgre = ST.getRawEncoding();
    const char *endCharPtr2 = sr.getCharacterData(ST);
    int stroffset = 0;
    int i=0,j=0;
   
    char getstrData[100];//all the line
    char getvarName[100];//var name

    while(endCharPtr2[stroffset]!=':'){
    	stroffset++;
    	if (stroffset>80) {
    		llvm::errs() << "too long switchcase;default"<<"\n";
    		break;
    	}
    }
    stroffset++;
    SourceLocation ST1 = ST.getLocWithOffset(stroffset);
    sprintf(temp,"\nblocks[%d] = '1';//%d %d!\n",pos%100000,ST,ENDD);
    Rewrite.InsertText(ST1, temp, true, true);
  }
  else if(flag==1)
  {
    // sprintf(temp,"\n  int seq_out_byte = %s/8;\
                \n  int seq_in_byte =1<<(%s%8);\
                \n  blocks[seq_out_byte]=blocks[seq_out_byte]|seq_in_byte;\n",char_pos,char_pos);
    
    SourceLocation ST = s->getBeginLoc();
    SourceLocation ENDD = s->getEndLoc();

    sprintf(temp,"\nblocks[%d] = '1';//%d %d!\n",pos%100000,ST,ENDD);
    llvm::errs() << "Found SwitchStmt!!! \n";
    // Insert opening brace.  Note the second true parameter to InsertText()
    // says to indent.  Sadly, it will indent to the line after the if, giving:
    // if (expr)
    //   {
    //   stmt;
    //   }
    
    Rewrite.InsertText(ST, temp, true, true); 
  }
  else if (!isa<CompoundStmt>(s))
  {
    // sprintf(temp,"{\n  int seq_out_byte = %s/8;\
                \n  int seq_in_byte =1<<(%s%8);\
                \n  blocks[seq_out_byte]=blocks[seq_out_byte]|seq_in_byte;\n",char_pos,char_pos);
    SourceLocation ST = s->getBeginLoc();
    SourceLocation ENDD = s->getEndLoc();
    sprintf(temp,"{\nblocks[%d] = '1';//%d %d@\n",pos%100000,ST,ENDD);
    llvm::errs() << "Found not CompoundStmt!!! \n";
    

    // Insert opening brace.  Note the second true parameter to InsertText()
    // says to indent.  Sadly, it will indent to the line after the if, giving:
    // if (expr)
    //   {
    //   stmt;
    //   }
    
    Rewrite.InsertText(ST, temp, true, true);

    // Note Stmt::getEndLoc() returns the source location prior to the
    // token at the end of the line.  For instance, for:
    // var = 123;
    //      ^---- getEndLoc() points here.

    SourceLocation END = s->getEndLoc();
    SourceManager& sr = Rewrite.getSourceMgr();

    // const char *endCharPtr = sr.getCharacterData(END);
    // 	Lexer::MeasureTokenLength();
    // MeasureTokenLength gets us past the last token, and adding 1 gets
    // us past the ';'.
    int offset = Lexer::MeasureTokenLength(END,
                                           Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) + 1;

    SourceLocation END1 = END.getLocWithOffset(offset);
    if(END1.getRawEncoding()-ST.getRawEncoding()>1000)
    {
	    const char *endCharPtr2 = sr.getCharacterData(ST);
    	size_t offsentSemicolon = 0;
    	while(endCharPtr2[offsentSemicolon++]!=';');
		END1 = ST.getLocWithOffset(offsentSemicolon);
	}

    llvm::errs()<<"CompoundStmt LocEnd "<<END1.getRawEncoding()<<"\n";

    Rewrite.InsertText(END1, "\n}", true, true);
  }
  else{
    // sprintf(temp,"\n  int seq_out_byte = %s/8;\n  int seq_in_byte =1<<(%s%8);\n  blocks[seq_out_byte]=blocks[eq_out_byte]|seq_in_bye;\n",char_pos,char_pos);
    SourceLocation ENDD = s->getEndLoc();
    SourceLocation ST = ((CompoundStmt *)s)->getLBracLoc().getLocWithOffset(1);
    sprintf(temp,"\nblocks[%d] = '1';//%d %d#\n",pos%100000,ST,ENDD);
    llvm::errs() << "Found CompoundStmt \n";
    
    Rewrite.InsertText(ST, temp, true, true);
    /*
    SourceLocation END = s->getEndLoc();
    int offset = Lexer::MeasureTokenLength(END,
        z                                   Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) + 1;

    SourceLocation END1 = END.getLocWithOffset(offset);
    Rewrite.InsertText(END1, "\n}", true, true);
    */
  }

  // Also note getEndLoc() on a CompoundStmt points ahead of the '}'.
  // Use getEndLoc().getLocWithOffset(1) to point past it.
  //end of function ,blockpos write back
  std::ofstream outfile("loopconvert.txt");
  outfile<<pos;
  outfile.close();
}

// Override Statements which includes expressions and more
bool MyRecursiveASTVisitor::VisitStmt(Stmt *s){
  int flag = 0;
  llvm::errs() << "Stmt Name :: " << s->getStmtClassName()<<"\n";
  stmtsum++;
  //llvm::errs() << "0000\n";
  if(isa<DeclStmt>(s))
  {
    stack++;  
    llvm::errs() << "DeclStmt Found------------- \n";
    AddrDisinfect(s);
  }
    
  if (isa<IfStmt>(s))
  {
    llvm::errs() << "Found if\n";
    if (isa<CallExpr>(s)) llvm::errs() << "Found if&call\n";
    // Cast s to IfStmt to access the then and else clauses
    IfStmt *If = cast<IfStmt>(s);
    Stmt *TH = If->getThen();

    // Add braces if needed to then clause
    //InstrumentStmt(TH,flag);// if has else if ,broken
    

    Stmt *EL = If->getElse();
    if (EL)
    {
      llvm::errs() << "Found else\n"; 
      

      if(!isa<IfStmt>(EL))
      {
        llvm::errs() << "found if in else\n";
        InstrumentStmt(TH,flag);
        InstrumentStmt(EL,flag);
	    //VisitThinPath(TH,3);
        //VisitThinPath(EL,4);
      }
      else
      {
          InstrumentStmt(TH,flag);
          //VisitThinPath(TH,2);
      }
      // Add braces if needed to else clause
    }
    else{
    	//VisitThinPath(TH,2);
    	InstrumentStmt(TH,flag);
    }
  }
  else
  if (isa<WhileStmt>(s))
  {
    llvm::errs() << "Found while\n";
    WhileStmt *While = cast<WhileStmt>(s);
    Stmt *BODY = While->getBody();
    InstrumentStmt(BODY,flag);
    //VisitThinPath(BODY,5);
  }
  else
  if (isa<ForStmt>(s))
  {
    llvm::errs() << "Found for\n";

    ForStmt *For = cast<ForStmt>(s);
    Stmt *BODY = For->getBody();
    InstrumentStmt(BODY,flag);
    //VisitThinPath(BODY,5);

  }/*
  else if(isa<SwitchCase>(s))
  {
      llvm::errs() << "Found Switch Case\n";
  }*/
  else if(isa<CaseStmt>(s))
  {
      llvm::errs() << "Found CaseStmt\n";
      InstrumentStmt(s,2);
  }
  else if(isa<DefaultStmt>(s))
  {
      llvm::errs() << "Found DefaultStmt\n";
      InstrumentStmt(s,2);

  }/*
  else if(isa<SwitchStmt>(s))
  {
    flag = 1;
      llvm::errs() << "Found Switch\n";
      SwitchStmt *Switch = cast<SwitchStmt>(s);
      SwitchCase *sc = Switch->getSwitchCaseList();
      while(sc)
      {
        Stmt *BODY = sc->getSubStmt();
        SourceLocation STT = sc->getBeginLoc();

        llvm::errs() <<"switch pos"<< STT.getRawEncoding() << "\n";

        InstrumentStmt(BODY,flag);
        //VisitThinPath(BODY,1);
        sc = sc->getNextSwitchCase();
      }
  }*/
  else if (isa<CallExpr>(s))
  {
  	llvm::errs() << "Found Call\n";
  	GetFuncCallGraph(s);
    

  }
  else if (isa<ReturnStmt>(s)){
  	llvm::errs() << "Found Return\n";
  	SourceLocation ST = s->getBeginLoc();
    SourceManager& sr = Rewrite.getSourceMgr();
    int offset = Lexer::MeasureTokenLength(ST,
                                           Rewrite.getSourceMgr(),
                                           Rewrite.getLangOpts()) + 1;
    char ss[100]="";
    sprintf(ss,"\n\tprint2(\"%s end,\");\n\t",func_name[func_now]);
    Rewrite.InsertText(ST,checkleak,true,true);
    Rewrite.InsertText(ST,ss,true,true);

  }
  //llvm::errs() << "Found Null\n";
  return true; // returning false aborts the traversal
}

bool MyRecursiveASTVisitor::VisitFunctionDecl(FunctionDecl *f)
{
  if (f->hasBody())
  {
  	llvm::errs() << "Found function " << (f->getNameInfo()).getName().getAsString()<<"\n";
  	llvm::errs() <<"stmtsum: "<<stmtsum<<'\n';
  	if (out.is_open()) out<<" \nA Func "<<(f->getNameInfo()).getName().getAsString(); 
  	if (func_blocks.is_open()) {
  		if (pos>=0) func_blocks<<pos%100000<<"\n";
  		func_blocks<<(f->getNameInfo()).getName().getAsString()<<" ";
  	}
    SourceRange sr = f->getSourceRange();
    Stmt *s = f->getBody();
    int i=0;
    //String s;
    char Funcname[_funcnamelen];
    strcpy(checkleak,"");
    for(i=0;i<sizeof((f->getNameInfo()).getName().getAsString());i++){
    	Funcname[i] = (f->getNameInfo()).getName().getAsString()[i];
    	if (i>=_funcnamelen-2) {
    		llvm::errs() << "too long Funcname"<<"\n";
    		break;
    	}
	}
	Funcname[i]='\0';
	strcpy(func_name[0],"NO_USE_FUNC_NAME");
	for (i=0;i<_funcsum;i++){
    	if(func_name[i][0]=='\0') break; 
    	if (strcmp(func_name[i],Funcname)==0) break;
    	
	}

	if (i>=_funcsum){
    	llvm::errs() << "too much functions"<<"\n";
    }
    else{
	    strcpy(func_name[i],Funcname);
	    func_now = i;
	    func_declare[func_declare_sum++]=func_now;
    }
    for (i=0;i<_funcsum;i++){
    	if(func_name[i][0]=='\0') break; 
    	llvm::errs() << func_name[i] <<func_call[i][0]
    	<<func_call[i][1]<<func_call[i][2]<<func_call[i][3]<<func_call[i][4]<<func_call[i][5]<<"\n";
    	
	}


    
    
    //llvm::errs() << "Exprloc"<<s->getExprLoc()<<"\n";
    //FF
    FuncEnd = sr.getEnd();

    // Make a stab at determining return type
    // Getting actual return type is trickier
    QualType q = f->getReturnType();
    const Type *typ = q.getTypePtr();

    std::string ret;
    if (typ->isVoidType())
       ret = "void";
    else
    if (typ->isIntegerType())
       ret = "integer";
    else
    if (typ->isCharType())
       ret = "char";
    else
       ret = "Other";

    

    if (f->isMain()){
      llvm::errs() << "Found main()\n";
      if (out.is_open()) out<<" Main";   
      func_main =func_now;
      // Get name of function
      DeclarationNameInfo dni = f->getNameInfo();
      DeclarationName dn = dni.getName();
      std::string fname = dn.getAsString();
    }
      // Point to start of function declaration
      SourceLocation ST = sr.getBegin();
      
      /*
      // Add 
      char fc[256];
      sprintf(fc, "#include <stdio.h>\n unsigned char blocks[64000]={0};\n");
      Rewrite.InsertText(ST, fc, true, true);
	  */
      // Add 
      SourceLocation INIT = s->getBeginLoc().getLocWithOffset(1);
      char temp[256]={0};
      sprintf(temp,"\n\tprint2(\"%s start,\");\n",Funcname);
      Rewrite.InsertText(INIT, temp, true, true);

      // Add 
      SourceLocation END = s->getEndLoc();
      FuncEND1 = END;
      char temp2[1000]={0};
      sprintf(temp2,"\n\tprint2(\"%s end,\");\n\t%s\n",Funcname,checkleak);
      Rewrite.InsertText(END, temp2, true, true);
       
    
  }

  return true; // returning false aborts the traversal
}





// Unchanged from the cirewriter ----- begin

class MyASTConsumer : public ASTConsumer
{
 public:

  MyASTConsumer(Rewriter &Rewrite) : rv(Rewrite) { }
  virtual bool HandleTopLevelDecl(DeclGroupRef d);

  MyRecursiveASTVisitor rv;
};

bool MyASTConsumer::HandleTopLevelDecl(DeclGroupRef d)
{
  typedef DeclGroupRef::iterator iter;

  for (iter b = d.begin(), e = d.end(); b != e; ++b)
  {
    rv.TraverseDecl(*b);
  }

  return true; // keep going
}

// Unchanged from the cirewriter ----- end

int main(int argc, char **argv)
{
  struct stat sb;

  if (argc < 2)
  {
     llvm::errs() << "Here is the Usage: CIrewriter <options> <filename>\n";
     return 1;
  }

  fs = rand();
  // Get filename
  std::string fileName(argv[argc - 1]);

  // Make sure it exists
  if (stat(fileName.c_str(), &sb) == -1)
  {
    perror(fileName.c_str());
    exit(EXIT_FAILURE);
  }

  CompilerInstance compiler;
  DiagnosticOptions diagnosticOptions;
  compiler.createDiagnostics();
  //compiler.createDiagnostics(argc, argv);

  // Create an invocation that passes any flags to preprocessor
  auto Invocation = std::make_shared<CompilerInvocation>();
  Invocation->getFrontendOpts().Inputs.push_back(FrontendInputFile("test.cpp",
                                                                   clang::InputKind::CXX));
  Invocation->getFrontendOpts().ProgramAction = frontend::ParseSyntaxOnly;
  compiler.setInvocation(std::move(Invocation));

  // Set default target triple
    std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
  pto->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *pti = TargetInfo::CreateTargetInfo(compiler.getDiagnostics(), pto);
  compiler.setTarget(pti);

  compiler.createFileManager();
  compiler.createSourceManager(compiler.getFileManager());

  HeaderSearchOptions &headerSearchOptions = compiler.getHeaderSearchOpts();

  // Allow C++ code to get rewritten
  LangOptions langOpts;
  langOpts.GNUMode = 1; 
  langOpts.CXXExceptions = 1; 
  langOpts.RTTI = 1; 
  langOpts.Bool = 1; 
  langOpts.CPlusPlus = 1; 
  /*
  Invocation->setLangDefaults(langOpts,
                              clang::IK_CXX,
                              clang::LangStandard::lang_cxx0x);
  */
  compiler.createPreprocessor(clang::TU_Prefix);
  //---------------compiler.getPreprocessorOpts().UsePredefines = false;
 
  compiler.createASTContext();

  // Initialize rewriter
  Rewriter Rewrite;
  Rewrite.setSourceMgr(compiler.getSourceManager(), compiler.getLangOpts());

  const FileEntry *pFile = compiler.getFileManager().getFile(fileName);
    compiler.getSourceManager().setMainFileID( compiler.getSourceManager().createFileID( pFile, clang::SourceLocation(), clang::SrcMgr::C_User));
  compiler.getDiagnosticClient().BeginSourceFile(compiler.getLangOpts(),
                                                &compiler.getPreprocessor());

  MyASTConsumer astConsumer(Rewrite);

  // Convert <file>.c to <file_out>.c
  std::string outName (fileName);
  size_t ext = outName.rfind(".");
  if (ext == std::string::npos)
     ext = outName.length();
  outName.insert(ext, "_out");


  llvm::errs() << "Output to: " << outName << "\n";
  std::error_code OutErrorInfo;
  std::error_code ok;
  llvm::raw_fd_ostream outFile(llvm::StringRef(outName), OutErrorInfo, llvm::sys::fs::F_None);





  //TODO

  if (OutErrorInfo == ok)
  {
    // Parse the AST
    ParseAST(compiler.getPreprocessor(), &astConsumer, compiler.getASTContext());
    compiler.getDiagnosticClient().EndSourceFile();

    // Output some #ifdefs
    outFile << "#define L_AND(a, b) a && b\n";
    outFile << "#define L_OR(a, b) a || b\n";
    outFile << "#ifndef STDIO_H\n";
    outFile << "#define STDIO_H\n";
    outFile << "#endif\n";

    char fc[256];
    std::ifstream infile("/root/loopconvert.txt");
    infile>>blockflag;
    infile.close();
    if (blockflag>=100000){
    	outFile << "\nextern unsigned char blocks[1000];\n";
    	infile.close();
    }
    else {
    	outFile << "\nunsigned char blocks[1000]={0};\n";
    	blockflag+=100000;
    	std::ofstream outfile("/root/loopconvert.txt");
        outfile<<blockflag;
        outfile.close();
    }
	  

    // Now output rewritten source code
    const RewriteBuffer *RewriteBuf =
      Rewrite.getRewriteBufferFor(compiler.getSourceManager().getMainFileID());
    outFile << std::string(RewriteBuf->begin(), RewriteBuf->end());
  }
  else
  {
    llvm::errs() << "Cannot open " << outName << " for writing\n";
  }

  outFile.close();

  //GetThinPath(0x1,715);
  //ResetFuncName();
  //CreateDangerFuncPathByDeep(func_main);
  //ShowDangerFuncPath();
  if (out.is_open()) out<< " \n";
  out.close();
  func_blocks<<pos%100000<<"\n";
  func_blocks.close();
  return 0;
}

