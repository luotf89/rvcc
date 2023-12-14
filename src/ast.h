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
    const char* name_;
    int len_;
    int value_;
    int offset_; // codegen 再栈中的相对栈帧的偏移
};

class Expr {
  public:
    explicit Expr(ExprType type);
    Expr();
    virtual ~Expr() {}
    virtual Expr* getNext() = 0;
    virtual Expr* getLeft() = 0;
    virtual Expr* getRight() = 0;
    virtual int computer() = 0;
    virtual void codegen() = 0;
    virtual int& value() = 0;
    virtual void visualize(std::ostringstream& oss, int& ident_num) = 0;
    int& id();
    ExprType& type();
    const char* getTypeName() const;
    void ident(std::ostringstream& oss, int& ident_num);
  private:
    int id_;
    ExprType type_;
    static std::atomic_int g_id;
    static const char* type_names[static_cast<int>(ExprType::NODE_COUNT)];

};

class BinaryExpr: public Expr {
  public:
    BinaryExpr();
    BinaryExpr(ExprType type, int value=0,
      Expr* left=nullptr, Expr* right=nullptr);
    virtual Expr* getNext() override;
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
    UnaryExpr(ExprType type, int value=0, Expr* left=nullptr);
    virtual Expr* getNext() override;
    virtual Expr* getLeft() override;
    virtual Expr* getRight() override;
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
    virtual Expr* getNext() override;
    virtual Expr* getLeft() override;
    virtual Expr* getRight() override;
    virtual int computer() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
  private:
    int value_;
};

class IdentityExpr: public Expr {
  public:
    IdentityExpr(Var* var=0);
    virtual Expr* getNext() override;
    virtual Expr* getLeft() override;
    virtual Expr* getRight() override;
    virtual int computer() override;
    virtual void codegen() override;
    virtual int& value() override;
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Var*& var();
  private:
    Var* var_;
};

class StmtExpr: public Expr {
  public:
    StmtExpr(Expr* next=nullptr, Expr* left=nullptr);
    virtual Expr* getNext() override;
    virtual Expr* getLeft() override;
    virtual Expr* getRight() override;
    virtual int computer() override;
    virtual void codegen() override;
    virtual int& value() override; // stmt value is id
    virtual void visualize(std::ostringstream& oss, int& ident_num) override;
    Expr*& next();
    Expr*& left();
  private:
    Expr* next_;
    Expr* left_;
};


class Function {
  public:
    enum class WalkOrderType{
      PRE_ORDER = 0,
      IN_ORDER = 1,
      POST_ORDER = 2
    };
    Function();
    Function(Expr* body, std::map<std::size_t, Var*>&& var_maps);
    ~Function();
    void getVarOffsets();
    std::map<std::size_t, Var*>& var_maps();
    void visualize(std::ostringstream& oss, int& ident_num);
    int computer();
    void codegen();
    Expr*& body();
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