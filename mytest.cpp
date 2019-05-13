#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include <system_error>

#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"

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

// RecursiveASTVisitor is is the big-kahuna visitor that traverses
// everything in the AST.

class MyRecursiveASTVisitor : public RecursiveASTVisitor<MyRecursiveASTVisitor>{
public:
    MyRecursiveASTVisitor(Rewriter &R) : Rewrite(R) { }
    void InstrumentStmt(Stmt *s);
    bool VisitStmt(Stmt *s);
    bool VisitFunctionDecl(FunctionDecl *f);
    Expr *VisitBinaryOperator(BinaryOperator *op);

    Rewriter &Rewrite;
}

