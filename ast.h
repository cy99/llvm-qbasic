/*
    defination of QBASIC Abstruct Syntax Tree
    Copyright (C) 2012  microcai <microcai@fedoraproject.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef __AST_H__
#define __AST_H__

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include <llvm/Value.h>
#include <llvm/Module.h>

enum CompOperator{
	Equl = 1, // == , =
	NotEqul , // <> , >< , != as in basic
	Less , // <
	Greater , // >
};

enum MathOperator{
	Mul = 1 , // *
	Div , // /
	Add , // +
	Minus , // -
	Mod , // % , MOD
	Power , // ^	
};

enum ExprType{
	VOID=0,	// nul type , used as return type of SUB XXX
			// only used by FunctionAST to define SUB in BASIC
	BOOLEAR,	// as boolear , TRUE,FALSE
	SHORT,	// as short
	Intger,	// as Intger
	Long,	// as long
	Double,	// as Double
	STRUCT,	// qbasic supports structure,
	STRING,	// STRING is an internal struct type.
			// STRING is implemented as structure by calling some member function
			// automanticall by compiler
	ARRAY	// 数组，only used by VariableExprAST & DimAST
};

enum Linkage{
	STATIC = 1,	//静态函数，无导出
	EXTERN,		//导出函数
	IMPORTC,		//导入C函数，这样就可以使用 C 函数调用了，算是我提供的一个扩展吧
};

enum ReferenceType{
	BYVALUE,	//传值
	BYREF,	//引用，实质就是指针了. 函数的默认参数是传引用
};
// allow us to use shared ptr to manage the memory
class AST // :public boost::enable_shared_from_this<AST>
{
public:
	AST();
	boost::shared_ptr<AST> next; //下一条语句
	virtual llvm::Value *Codegen() = 0;
	virtual ~AST();
	static llvm::Module * module;
private:
	AST( const AST &  );
	AST & operator =( const AST &  );
};

class DimAST: public AST
{

};

class VariableDimAST : public DimAST
{
	ExprType type;
	ReferenceType	reftype; //引用类型
	//ExprType type; // the type of the expresion
	std::string varname; //定义的变量名字
	
};
typedef boost::shared_ptr<VariableDimAST> VariableDimASTPtr;

//定义结构体变量
class VariableStructDimAST : VariableDimAST
{
	//VariableSimpleDimAST	
	std::list<VariableDimASTPtr> members;
};

//定义数组
class VariableArrayDimAST : VariableDimAST
{
	VariableDimASTPtr	itemtype;
	int start,end; //起始位置
};

// 表达式
class ExprAST: public AST //
{
	ExprType type; // the type of the expresion

};

typedef boost::shared_ptr<ExprAST>	ExprASTPtr;

// 常量
class ConstExprAST:public ExprAST
{
public:
	std::string constval;
	ConstExprAST(const std::string * val);
	virtual	llvm::Value *Codegen();
};

class VariableRefExprAST:public ExprAST
{
	std::string	var; //指向引用的变量名字.
 	VariableDimASTPtr	vardim; //变量的定义位置. 如果为 0 则是随地定义. // not used by parser but generator
};

typedef 	boost::shared_ptr<VariableRefExprAST> VariableExprASTPtr;


// 结构体变量，就是各种变量类型的集合
class StructVariableExprAST: public VariableRefExprAST
{
	std::list<VariableExprASTPtr>	members; //各个成员
};

typedef boost::shared_ptr<StructVariableExprAST> StructVariableExprASTPtr;

//结构体成员的引用
class StructVariableRefExprAST: public VariableRefExprAST
{
	StructVariableExprASTPtr	structvar; //引用的结构体
	std::string			nameormember; //引用的成员
};

//数组变量的使用
class ArrayVariableRefExprAST:public VariableRefExprAST
{
	VariableExprASTPtr items; //数组变量的话，数组的成员的类型

	ExprASTPtr		index; //下表操作还是一个表达式
};
typedef boost::shared_ptr<VariableRefExprAST> VariableRefExprASTPtr;

//比较表达式 比较两个表达式的值
class CompExprAST:public ExprAST // bool as result
{
	ExprASTPtr  RHS,LHS;
	enum CompOperator op;	
};

// 数值计算表达式
class CalcExprAST:public ExprAST
{
	ExprASTPtr  RHS,LHS;
	enum MathOperator op;
};

// 前向函数声明
class FunctionDeclarAST: public DimAST
{
	Linkage		linkage; //链接类型。static? extern ?
	ExprType		type; //返回值
	std::string	name; //函数名字
	std::list<VariableDimASTPtr> args_type; //checked by CallExpr
};

//函数体
class FunctionDimAST: public FunctionDeclarAST
{
	std::list<VariableDimAST> args; //定义的参数

	AST * body; //函数体

//	std::list<DimAST>	dims;//定义的本地变量

//	StatementsAST		body;//函数体
};

typedef std::list<ExprASTPtr>	FunctionParameterListAST;
// CALL Sub Functions , 函数调用也是表达式之一，返回值是表达式嘛
class CallExprAST:public ExprAST
{
	//参数，参数是一个表达式列表
	FunctionParameterListAST	args;
};

typedef boost::shared_ptr<CalcExprAST> CalcExprASTPtr;

// 语句
class StatementAST: public AST
{
public:
	std::string	LABEL;	// label , if there is. then we can use goto
						// must be uniq among function bodys
	virtual llvm::Value* Codegen();
};

typedef boost::shared_ptr<StatementAST>	StatementASTPtr;

//左值和右值, 把右值赋给左值
class LetStatementAST: public StatementAST
{
public:
	LetStatementAST(VariableRefExprASTPtr lval , ExprASTPtr rval);

	VariableRefExprASTPtr lval;//注意，左值只能是变量表达式
	ExprASTPtr rval; // 右值可以是任意的表达式。注意，需要可以相互转化的。
	virtual	llvm::Value *Codegen();
};


// IF XX THEN xx ELSE xx ENDIF
class IFExprAST:public StatementAST
{
	ExprASTPtr ifexpresion;
	StatementAST THEN;
	StatementAST ELSE;
};

// loop XXX until
class LoopExprAST: public StatementAST
{
	
	
};


//函数调用语句
class CallStatmentAST: public StatementAST
{
public:
//	CallStatmentAST( CalcExprASTPtr );
	CalcExprASTPtr tocall;
};

////////////////////////////////////////////////////////////////////////////////
//内建函数语句. PRINT , 为 PRINT 生成特殊的函数调用:)
////////////////////////////////////////////////////////////////////////////////
class PrintAST: public StatementAST
{
public:
	PrintAST(FunctionParameterListAST);
    virtual llvm::Value* Codegen();

	FunctionParameterListAST printlist;
};

#endif // __AST_H__

