#ifndef __AST_H
#define __AST_H


#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <sstream>
#include <string>


namespace rvcc {

enum class ExprType:int{
  NODE_ADD = 0,         // +
  NODE_SUB,             // -
  NODE_MUL,             // *
  NODE_DIV,             // /
  NODE_NUM,             // number
  NODE_NEG,             // '-'          
  NODE_EQ,              // ==
  NODE_NE,              // !=
  NODE_LT,              // <
  NODE_LE,              // <=
  NODE_ID,              // identify
  NODE_ASSIGN,          // =
  NODE_STMT,
  NODE_RETURN,
  NODE_COMPOUND,
  NODE_IF,         
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
    explicit Expr(ExprType type);
    Expr();
    virtual ~Expr() {}
    virtual Expr* getNext();     // 方便调试
    virtual Expr* getLeft();     // 方便调试
    virtual Expr* getRight();
    virtual Expr* getStmts();    // 方便调试
    virtual Expr* getCond();
    virtual Expr* getThen();
    virtual Expr* getEls();
    virtual int computer() = 0;
    virtual void codegen() = 0;
    virtual int& value() = 0;
    virtual void visualize(std::ostringstream& oss, int& ident_num) = 0;
    int& id();
    ExprType& type();
    const char* getTypeName() const;
    void ident(std::ostringstream& oss, int& ident_num);

    enum class WalkOrderType{
      PRE_ORDER = 0,
      IN_ORDER = 1,
      POST_ORDER = 2
    };
    using Func = std::function<void(Expr*)>;
    template<WalkOrderType walk_order>
    void static walkImpl(Func func, Expr*curr) {
      if (!curr) {
        return;
      }
      if constexpr (walk_order == WalkOrderType::PRE_ORDER) {
        func(curr);
      }
      walkImpl<walk_order>(func, curr->getLeft());
      if constexpr(walk_order == WalkOrderType::IN_ORDER) {
        func(curr);
      }
      walkImpl<walk_order>(func, curr->getRight());
      if constexpr(walk_order == WalkOrderType::POST_ORDER) {
        func(curr);
      }
    }
  private:
    int id_;
    ExprType type_;
    static std::atomic_int g_id;
    static const char* type_names[static_cast<int>(ExprType::NODE_COUNT)];

};
/*
NextExpr 作为 stmt的基类， 提供next方法，和return flag 方法
stmt的种类包括 单语句 复合语句 if-else for call 等等
*/
class NextExpr : public Expr {
  public:
    NextExpr(ExprType type, Expr* next=nullptr);
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
    BinaryExpr(ExprType type, Expr* left=nullptr,
               Expr* right=nullptr);
    virtual Expr* getLeft() override;
    virtual Expr* getRight() override;
    virtual int computer() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Expr*& left();
    Expr*& right();
  private:
    int value_;
    Expr* left_;
    Expr* right_;
};

class UnaryExpr: public Expr {
  public:
    UnaryExpr();
    UnaryExpr(ExprType type, Expr* left=nullptr);
    virtual Expr* getLeft() override;
    virtual int computer() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Expr*& left();
  private:
    int value_;
    Expr* left_;
};

class NumExpr: public Expr {
  public:
    NumExpr(int value=0);
    virtual int computer() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
  private:
    int value_;
};

class IdentityExpr: public Expr {
  public:
    IdentityExpr(Var* var=nullptr);
    ~IdentityExpr();
    virtual int computer() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Var*& var();
  private:
    Var* var_;
};

class StmtExpr: public NextExpr {
  public:
    StmtExpr(Expr* next=nullptr, Expr* left=nullptr);
    ~StmtExpr();
    virtual Expr* getLeft() override;
    virtual int computer() override;
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
    CompoundStmtExpr(NextExpr* stmts=nullptr);
    ~CompoundStmtExpr();
    virtual Expr* getStmts() override;
    virtual int computer() override;
    virtual void codegen() override;
    virtual int& value() override; // stmt value is id
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    NextExpr*& stmts();
  private:
    NextExpr* stmts_;
    int value_;
};

class IfExpr: public NextExpr {
  public:
    IfExpr(Expr* cond=nullptr, Expr* then=nullptr, Expr* els=nullptr);
    ~IfExpr();
    virtual Expr* getCond() override;
    virtual Expr* getThen() override;
    virtual Expr* getEls() override;
    virtual int computer() override;
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


class Function {
  public:
    Function();
    Function(Expr* body, std::map<std::size_t, Var*>&& var_maps);
    ~Function();
    void getVarOffsets();
    std::map<std::size_t, Var*>& var_maps();
    void visualize(std::ostringstream& oss, int& ident_num);
    int computer();
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
    
      int computer();
      void codegen();
      int visualization(std::string filename);
    private:
      Function* root_;
  };
}
#endif