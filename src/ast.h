#ifndef __AST_H
#define __AST_H


#include "lexer.h"
#include "token.h"
#include "type.h"
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <sstream>
#include <string>


namespace rvcc {

enum class ExprKind:int{
  //叶子节点
  NODE_NUM = 0,         // number
  NODE_ID,              // identify
  // unary op
  NODE_NEG,             // 取反操作
  NODE_ADDR,            // 取地址 &
  NODE_DEREF,           // 解引用 *
  NODE_RETURN,
  // binary op
  NODE_ADD,              // +
  NODE_SUB,             // -
  NODE_MUL,             // *
  NODE_DIV,             // /
  NODE_EQ,              // ==
  NODE_NE,              // !=
  NODE_LT,              // <
  NODE_LE,              // <=
  NODE_ASSIGN,          // =
  // 语句
  NODE_STMT,
  NODE_COMPOUND,
  NODE_IF,
  NODE_FOR,
  NODE_WHILE,      
  NODE_ILLEGAL,         // illegal
  NODE_COUNT
};

class Var{
  public:
    Var();
    Var(char* name, int len, int value = 0);
    int& len();
    const char* getName();
    int& value();
    int& offset();
  private:
    const char* name_; // name 共享 输入buffer 制作， 不需要释放
    int len_;
    int value_;
    int offset_; // codegen 再栈中的相对栈帧的偏移
};

class Expr {
  public:
    explicit Expr(ExprKind kind);
    Expr();
    virtual ~Expr() {}
    virtual Expr* getNext();     // 方便调试
    virtual Expr* getLeft();     // 方便调试
    virtual Expr* getRight();
    virtual Expr* getStmts();    // 方便调试
    virtual Expr* getCond();
    virtual Expr* getThen();
    virtual Expr* getEls();
    virtual Expr* getInit();
    virtual Expr* getInc();
    virtual Type* getType();
    virtual void codegen() = 0;
    virtual int& value() = 0;
    virtual void visualize(std::ostringstream& oss, int& ident_num) = 0;
    int& id();
    ExprKind& kind();
    const char* kindName() const;
  private:
    int id_;
    ExprKind kind_;
    static std::atomic_int g_id;
    static const char* kind_names[static_cast<int>(ExprKind::NODE_COUNT)];

};
/*
NextExpr 作为 stmt的基类， 提供next方法，和return flag 方法
stmt的种类包括 单语句 复合语句 if-else for call 等等
*/
class NextExpr : public Expr {
  public:
    NextExpr(ExprKind kind, Expr* next=nullptr);
    Expr * getNext() override;
    Expr*& next();
    bool getReturnFlag();
    bool& returnFlag();
  private:
    Expr* next_;
    bool return_flag_;
};

class BinaryExpr: public Expr {
  public:
    BinaryExpr(ExprKind kind, Expr* left=nullptr,
               Expr* right=nullptr);
    virtual Expr* getLeft() override;
    virtual Expr* getRight() override;
    virtual Type* getType() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Type*& type();
    Expr*& left();
    Expr*& right();
  private:
    int value_;
    Expr* left_;
    Expr* right_;
    Type* type_;
};

class UnaryExpr: public Expr {
  public:
    UnaryExpr();
    UnaryExpr(ExprKind kind, Expr* left=nullptr);
    virtual Expr* getLeft() override;
    virtual Type* getType() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Type*& type();
    Expr*& left();
  private:
    int value_;
    Expr* left_;
    Type* type_;
};

class NumExpr: public Expr {
  public:
    NumExpr(int value=0);
    virtual Type* getType() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Type*& type();
  private:
    int value_;
    Type* type_;
};

class IdentityExpr: public Expr {
  public:
    IdentityExpr(Var* var=nullptr);
    ~IdentityExpr();
    virtual Type* getType() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Type*& type();
    Var*& var();
  private:
    Var* var_;
    Type* type_;
};

class StmtExpr: public NextExpr {
  public:
    StmtExpr(Expr* left=nullptr);
    ~StmtExpr();
    virtual Expr* getLeft() override;
    virtual void codegen() override;
    virtual int& value() override; // stmt value is id
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Expr*& left();
  private:
    Expr* left_;
    int value_;
};

class CompoundStmtExpr: public NextExpr {
  public:
    CompoundStmtExpr(Expr* stmts=nullptr);
    ~CompoundStmtExpr();
    virtual Expr* getStmts() override;
    virtual void codegen() override;
    virtual int& value() override; // stmt value is id
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Expr*& stmts();
  private:
    Expr* stmts_;
    int value_;
};

class IfExpr: public NextExpr {
  public:
    IfExpr(Expr* cond=nullptr, Expr* then=nullptr, Expr* els=nullptr);
    ~IfExpr();
    virtual Expr* getCond() override;
    virtual Expr* getThen() override;
    virtual Expr* getEls() override;
    virtual void codegen() override;
    virtual int& value() override; // stmt value is id
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Expr*& cond();
    Expr*& then();
    Expr*& els();
  private:
    Expr* cond_;
    Expr* then_;
    Expr* els_;
    int value_;
};

class ForExpr: public NextExpr {
  public:
    ForExpr(Expr* init=nullptr, Expr* cond=nullptr, Expr* inc=nullptr, Expr* stmts=nullptr);
    ~ForExpr();
    virtual Expr* getStmts() override;
    virtual Expr* getInit() override;
    virtual Expr* getCond() override;
    virtual Expr* getInc() override;
    virtual void codegen() override;
    virtual int& value() override; // stmt value is id
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Expr*& init();
    Expr*& cond();
    Expr*& inc();
    Expr*& stmts();
  private:
    Expr* init_;
    Expr* cond_;
    Expr* inc_;
    Expr* stmts_;
    int value_;
};

class WhileExpr: public NextExpr {
  public:
    WhileExpr(Expr* cond=nullptr, Expr* stmts=nullptr);
    ~WhileExpr();
    virtual Expr* getStmts() override;
    virtual Expr* getCond() override;
    virtual void codegen() override;
    virtual int& value() override; // stmt value is id
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Expr*& cond();
    Expr*& stmts();
  private:
    Expr* cond_;
    Expr* stmts_;
    int value_;
};

class Function {
  public:
    Function();
    Function(Expr* body, std::map<std::size_t, Var*>&& var_maps);
    ~Function();
    std::map<std::size_t, Var*>& var_maps();
    void visualize(std::ostringstream& oss, int& ident_num);
    void codegen();
    Expr*& body(); 
  private:
    void freeNode(Expr* curr);
    Expr* body_;
    std::map<std::size_t, Var*> var_maps_;
};

class Ast{
  public:
    Ast();
    Ast(Function* root);
    ~Ast();
    Function*& root();
    void codegen();
    int visualization(std::string filename);
  private:
    Function* root_;
};

}
#endif